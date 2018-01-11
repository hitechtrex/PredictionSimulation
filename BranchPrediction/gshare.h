/*
* CS5900-10 Computer Architecture
* Project: Branch Prediction
* Author: Steve Jia
* File: twolevgap.h
*
* Two Level Adaptive Branch Predictor
* GShare Branch Prediction Using Per-address
* Pattern History Tables Implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LINE_SIZE 512	//for reading the file
#define DEBUG_Gs 0		//debug flag

/* function: gs_predict
 * get the prediction value and update the pattern history table
 * compare the branch result to the prediction and returns
 * the prediction correctness */
int gs_predict(unsigned long global_hist_reg,
			unsigned long global_hist_len,
			int pat_table[],
			int addr,
			int taken)
{				
	//grab value of lowest x bits of BHR, x = global_hist_len
	//row index in the global_pat_table
	unsigned long pat_index = global_hist_reg << (64 - global_hist_len) >> (64 - global_hist_len);
	
	//get the prediction from the pattern history table, 1:taken, 0:not taken
	int prediction = pat_table[pat_index] > 1 ? 1 : 0;
	
	if(DEBUG_Gs) fprintf(stdout, "add: %d, pat: %ld, pred: %d, tak: %d\n", 
							addr, pat_index, prediction, taken);
	//update the pattern history table
	if(taken){
		//if result is taken, increase by 1, but maxed out value at 3
		pat_table[pat_index] = 
			(pat_table[pat_index]+1 < 3) ? (pat_table[pat_index]+1) : 3;
	}
	else{
		//if result is not taken, decrease by 1, but value does not go below zero
		pat_table[pat_index] = 
			(pat_table[pat_index]-1 > 0) ? (pat_table[pat_index]-1) : 0;
	}
	//return prediction correctness
	return (taken == prediction);
}//predict()


/* function gshare_predictor:
 * core function of the GShare predictor; the trace file is opened and
 * the address and branch result is read and processed line by line 
 * branch history register is updated in here after every prediction */
void gshare_predictor(char* input_file, 
					     int ppht_table_size, 
						 int global_hist_len){
	//global_hist_len is number of bits in the global branch history register
	unsigned long global_history = 0; //value of GBHR
	int pat_table_size = pow(2, global_hist_len); //number of rows in each PPHT
	int global_pat_table[ppht_table_size][pat_table_size]; //pphts
	
	//initialize all tables
	for(int i = 0; i < ppht_table_size; i++){
		for(int j = 0; j < pat_table_size; j++){
			global_pat_table[i][j] = 1; //weakly not taken
		}
	}
	
	int addrInt, taken, ppht_index;
	char* addrChar;
	char* text;
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
		
		//XOR the address value and the BHR value to get the index for the PPHTs
		ppht_index = (addrInt ^ global_history) & (ppht_table_size - 1);
		if(DEBUG_Gs) fprintf(stdout, "add: %s (%d), ppht: %d\n", addrChar, addrInt, ppht_index);
		
		//predict the result
		correct += gs_predict(global_history, 
						   global_hist_len,
						   global_pat_table[ppht_index],
						   addrInt,
						   taken);
						   
		//shift the GBHR value left by 1 bit and add the branch result to the LSB
		global_history = (global_history << 1) + taken;
		
		total++;
		
	}//while()
	fclose(file);
	
	//print result
	printf("\n------ GShare Prediction -------\n");
	printf("----- BHR Len: %d, PHT Len: %d -----\n", global_hist_len, ppht_table_size);
	printf("\tPercentage Correct: %f%%\n\tCorrect: %f\n\tTotal Branches: %f\n", 
			correct/total * 100, correct, total);
		
	// calculate hardware cost
	// for GShare, cost = (k + b*(2^k)*2)/8 bytes where k is GBHR length, b is PHT count
	// pat_table_size = pow(2, global_hist_len), since we already calculated it, we
	// will re-use it here
	double temp1 = pat_table_size;
	temp1 *= 2.0;
	temp1 *= ppht_table_size; // b*(2^k)*2
	double cost = temp1 + global_hist_len; // (k + b*(2^k)*2)
	cost /= 8.0; // convert to bytes
	cost /= 1000.0; //convert to Kbytes
	// print the cost
	fprintf(stdout, "\tHardware Cost %f Kbytes\n", cost);
		
}//gshare_predictor()