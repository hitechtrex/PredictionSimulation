/*
* CS5900-10 Computer Architecture
* Project: Branch Prediction
* Author: Steve Jia
* File: static.h
*
* Static Branch Predictor:
*    this predictor always predicts not taken
*    on a branch
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 512 //for reading the file

void static_prediction(char* input_file){
	//variable for correct predictions and total branches
	double correct = 0;
	double total = 0; 
	//start by opening the file and read it line by line
	FILE *file = fopen(input_file, "r");
	char line[LINE_SIZE];
	char* text;
	while(fgets(line, sizeof(line), file)){
		text = line;
		text[10] = '\0';
		//compare the branch result to the prediction
		// since we always predict not taken, so compare
		// file value with '-'
		if(strcmp(&text[9], "-") == 0){
			correct++;
		}
		total++;
	}
	fclose(file);
	
	//print result
	printf("\n----- Static Prediction -----\n");
	printf("\tPercentage correct: %f%%\n\tCorrect: %f\n\tTotal Branches: %f\n", 
		correct/total * 100, correct, total);
}//static_prediction()