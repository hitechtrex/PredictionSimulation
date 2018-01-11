/*
* CS5900-10 Computer Architecture
* Project: Branch Prediction
* Author: Steve Jia
* File: twolevel.h
*
* Two Level Adaptive Branch Predictor
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LINE_SIZE 512
#define DEBUG 0

typedef struct _hist_reg{
	int usage;
	int address;
	unsigned long pattern;
} hist_reg;


int find_index(hist_reg* hist_table, int table_size, int address){
	//search the local_history table, if found, return index
	//  else return index of first occurance of least used element
	int index = 0;
	int least = 2147483647; //initial usage, max int 2147483647
	for(int i = 0; i < table_size; i++){
		if(hist_table[i].address == address){
			return i;
		}
		
		if(hist_table[i].usage < least){
			least = hist_table[i].usage;
			index = i;
		}
	}//for
	return index;
}//find_index()


int predict(hist_reg* hist_table,
			unsigned long reg_length,
			int pat_hist[],
			int address,
			int index,
			int taken){

	if(DEBUG){
		fprintf(stdout, "adr: %d, pat: %lu, use: %d\n",
			 hist_table[index].address, 
			 hist_table[index].pattern, 
			 hist_table[index].usage);
	}

	// grab lowest x bits, x = reg_length
	unsigned long pat_index = hist_table[index].pattern << 
								(64 - reg_length) >> (64 - reg_length);

	//get the prediction from the pattern history table, 1:taken, 0:not taken
	int prediction = pat_hist[pat_index] > 1 ? 1 : 0;

	if(DEBUG)
		fprintf(stdout, "pat: %ld, pred: %d\n", pat_index, prediction);

	if(taken){
		pat_hist[pat_index] = (pat_hist[pat_index]+1 < 3) ? (pat_hist[pat_index]+1) : 3;
	}
	else{
		pat_hist[pat_index] = (pat_hist[pat_index]-1 > 0) ? (pat_hist[pat_index]-1) : 0;	
	}

	hist_table[index].pattern = (hist_table[index].pattern << 1) + taken;

	return (taken == prediction);
}//predict()


void two_level_predictor(char* input_file,
						 int hist_table_size,
						 int reg_len)
{
	//char* input_file = "gccSmall.trace";
	int index = 0, addrInt = 0, taken = 0;
	char* text;
	char* addrChar;

	hist_reg hist_table[hist_table_size];

	int pat_table_size = pow(2, reg_len);

	int pat_table[hist_table_size][pat_table_size];

	for(int i = 0; i < hist_table_size; i++){
		hist_table[i].usage = 0;
		hist_table[i].address = 0;
		hist_table[i].pattern = 0;
		for(int j = 0; j < pat_table_size; j++){
			pat_table[i][j] = 1; //init set to weakly taken
		}//inner for
	}//outer for


	FILE *file = fopen(input_file, "r");
	char line[LINE_SIZE];
	double correct = 0;
	double total = 0; 

	while(fgets(line, sizeof(line), file)){
		text = line;
		text[8] = '\0';
		addrChar = &text[0];
		addrInt = (int)strtol(addrChar, NULL, 16);
		text[10] = '\0';
		taken = (strcmp(&text[9], "+")==0) ? 1 : 0;

		index = find_index(hist_table, hist_table_size, addrInt);
		
		if(DEBUG)
			fprintf(stdout, "\nFound Index: %d\n", index);
			
		if(hist_table[index].address == addrInt){
			//entry exist in table
			hist_table[index].usage++;
		}
		else{
			//address does not exist
			if(DEBUG){
				fprintf(stdout, "Entry Replaced, Prev: %d, New: %d\n",
						 hist_table[index].address, addrInt);
			}
			hist_table[index].address = addrInt;
			if(hist_table[index].pattern != 0)
				hist_table[index].pattern = 0;
			if(hist_table[index].usage != 1)
				hist_table[index].usage = 1;
		}//if-else

		if(DEBUG){
			fprintf(stdout, "addr: %s, sig:%s, addrInt: %d, tak:%d\n", 
						addrChar, &text[9], addrInt, taken);
			fprintf(stdout, "index: %d, use: %d\n", index, hist_table[index].usage);
		}

		correct += predict(hist_table,
						   reg_len,
						   pat_table[index],
						   addrInt,
						   index,
						   taken);
		total++;
	}//while fgets
	fclose(file);
	
	//free(hist_table);
	//free(pat_table);

	printf("\n------ Two-Level Prediction -------\n");
	printf("----- BHT Len: %d, BHR Len: %d -----\n", hist_table_size, reg_len);
	printf("\tPercentage correct: %f%%\n\tCorrect: %f\n\tTotal Branches: %f\n", 
		correct/total * 100, correct, total);

}//two_level_predictor()