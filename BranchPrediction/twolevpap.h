/*
* CS5900-10 Computer Architecture
* Project: Branch Prediction
* Author: Steve Jia
* File: twolevpap.h
*
* Two Level Adaptive Branch Predictor
* Per-address Adaptive Branch Prediction Using Per-address
* Pattern History Tables (PAp) Implementation with least-
* used-algorithm for collision detection in the branch
* history table.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_INT 2147483647	// maxium signed integer number
#define LINE_SIZE 512	// for reading the file
#define DEBUG_PAp 0		// debug flag


// structure definition of a branch history register
typedef struct _hist_reg{
	int usage;				// number of times  used
	int address;			// address int value
	unsigned long pattern;	// register value
} hist_reg;


/* function: find_index
 * search the branch history table, if found, return index
 * else return index of first occurance of least used element */
int find_index(hist_reg* hist_table, int table_size, int address){
	int index = 0;
	int least = MAX_INT; // initial usage, max int 2147483647
	for(int i = 0; i < table_size; i++){
		if(hist_table[i].address == address){
			return i;
		}
		//find a track the usage to find the least used
		if(hist_table[i].usage < least){
			least = hist_table[i].usage;
			index = i;
		}
	}//for
	return index;
}//find_index()


/* function: pap_predict
 * update the branch history table and per-address pattern history
 * table, get the prediction value, compare to branch result and
 * returns prediction correctness */
int pap_predict(hist_reg* hist_table,
			unsigned long reg_length,
			int pat_hist[],
			int address,
			int index,
			int taken){

	if(DEBUG_PAp){
		fprintf(stdout, "adr: %d, pat: %lu, use: %d\n",
			 hist_table[index].address, 
			 hist_table[index].pattern, 
			 hist_table[index].usage);
	}

	//grab value of lowest x bits of BHR, x = reg_length
	unsigned long pat_index = hist_table[index].pattern << 
								(64 - reg_length) >> (64 - reg_length);

	//get the prediction from the pattern history table, 1:taken, 0:not taken
	int prediction = pat_hist[pat_index] > 1 ? 1 : 0;

	if(DEBUG_PAp)
		fprintf(stdout, "pat: %ld, pred: %d\n", pat_index, prediction);

	//update the pattern history table
	if(taken){
		//if result is taken, increase by 1, but maxed out value at 3
		pat_hist[pat_index] = (pat_hist[pat_index]+1 < 3) ? (pat_hist[pat_index]+1) : 3;
	}
	else{
		//if result is not taken, decrease by 1, but value does not go below zero
		pat_hist[pat_index] = (pat_hist[pat_index]-1 > 0) ? (pat_hist[pat_index]-1) : 0;	
	}

	//shift the BHR value left by 1 bit and add the branch result to the LSB
	hist_table[index].pattern = (hist_table[index].pattern << 1) + taken;
	//return prediction correctness
	return (taken == prediction);
}//predict()


/* function: twolev_pap_predictor
 * core function of the PAp predictor, the trace file is opened and
 * the address and branch result is read and processed line by line */
void twolev_pap_predictor(char* input_file,
						 int hist_table_size,
						 int reg_len)
{
	//char* input_file = "gccSmall.trace";
	int index = 0, addrInt = 0, taken = 0;
	char* text;
	char* addrChar;

	//declare the per-address branch history table
	hist_reg hist_table[hist_table_size];
	//calculate how many entries for each pattern history table
	int pat_table_size = pow(2, reg_len);
	//declare the entire pattern history table
	int pat_table[hist_table_size][pat_table_size];
	//initialize all tables
	for(int i = 0; i < hist_table_size; i++){
		hist_table[i].usage = 0;
		hist_table[i].address = 0;
		hist_table[i].pattern = 0;
		for(int j = 0; j < pat_table_size; j++){
			pat_table[i][j] = 1; //init set to weakly taken
		}//inner for
	}//outer for

	//open the trace file
	FILE *file = fopen(input_file, "r");
	char line[LINE_SIZE];
	double correct = 0;
	double total = 0; 

	while(fgets(line, sizeof(line), file)){
		text = line;
		text[8] = '\0';
		addrChar = &text[0];
		//get the address's int value
		addrInt = (int)strtol(addrChar, NULL, 16);
		text[10] = '\0';
		//check if branch result is taken or not taken
		taken = (strcmp(&text[9], "+")==0) ? 1 : 0;
		//find the index of address in the branch history table
		index = find_index(hist_table, hist_table_size, addrInt);
		
		if(DEBUG_PAp)
			fprintf(stdout, "\nFound Index: %d\n", index);
			
		if(hist_table[index].address == addrInt){
			//entry exist in table
			hist_table[index].usage++;
		}
		else{
			//address does not exist
			if(DEBUG_PAp){
				fprintf(stdout, "Entry Replaced, Prev: %d, New: %d\n",
						 hist_table[index].address, addrInt);
			}
			hist_table[index].address = addrInt;
			if(hist_table[index].pattern != 0)
				hist_table[index].pattern = 0;
			if(hist_table[index].usage != 1)
				hist_table[index].usage = 1;
		}//if-else

		if(DEBUG_PAp){
			fprintf(stdout, "addr: %s, sig:%s, addrInt: %d, tak:%d\n", 
						addrChar, &text[9], addrInt, taken);
			fprintf(stdout, "index: %d, use: %d\n", index, hist_table[index].usage);
		}
		
		//predict the result
		correct += pap_predict(hist_table,
						   reg_len,
						   pat_table[index],
						   addrInt,
						   index,
						   taken);
		total++;
	}//while fgets
	fclose(file);

	//print result
	printf("\n------ Two-Level Prediction (PAp) -------\n");
	printf("----- BHT Len: %d, BHR Len: %d -----\n", hist_table_size, reg_len);
	printf("\tPercentage Correct: %f%%\n\tCorrect: %f\n\tTotal Branches: %f\n", 
			correct/total * 100, correct, total);
		
	// calculate hardware cost
	// for PAp, cost = (b*k + b*(2^k)*2)/8 bytes where k is BHR length, b is PHT count
	double temp1 = hist_table_size * reg_len; // b*k
	// pat_table_size = pow(2, reg_len), since we already calculated it, we
	// will re-use it here
	double temp2 = pat_table_size;
	temp2 *= 2.0;
	temp2 *= hist_table_size; // b*(2^k)*2
	double cost = temp1 + temp2; // (k*b + b*(2^k)*2)
	cost /= 8.0; // convert to bytes
	cost /= 1000.0; //convert to Kbytes
	// print the cost
	fprintf(stdout, "\tHardware Cost %f Kbytes\n", cost);
	
}// two_level_predictor()