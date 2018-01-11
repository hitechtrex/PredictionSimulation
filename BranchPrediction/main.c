/*
* CS5900-10 Computer Architecture
* Project: Branch Prediction
* Author: Steve Jia
* File: main.c
* Description: this is the main file that starts
* 	the different types of branch predictors. In
* 	this program, a static predictor will be called
* 	first, and then two different two-level branch
* 	predictors will be called (PAp and GAp), and
* 	finally, a GShare predictor will be called.
* 	Each predictor will use different table size
* 	and history length to show performance diff-
* 	erences.
*/

#include <stdio.h>
#include "static.h"
#include "twolevpap.h"
#include "twolevgap.h"
#include "gshare.h"

/* function: main
 * main function that specifies the trace, and run the static,
 * two-level PAp, two-level GAp, and the GShare predictors with
 * different setup parameters */
int main(int argc, char **argv)
{
	printf("Starting branch predictors...\n");
	
	//char* trace_file = "../gccSmall.trace";
	char* trace_file = "gccSmall.trace"; //same level as binaries
	
	// run the static predictor
	static_prediction(trace_file);
	
	// declare different table sizes and history length
	int tab_size, reg_len, glob_hist_len;
	
	// call the two-level PAp predictor with different setups
	for(tab_size = 512; tab_size <= 4096; tab_size*=2){
		for(reg_len = 2; reg_len <= 8; reg_len*=2){
			twolev_pap_predictor(trace_file, tab_size, reg_len);
		}
	}
	// run the predictor with 8192 table size and only up to 4 bits
	// for the BHR b/c more bits cause crashes
	tab_size = 8192;
	for(reg_len = 2; reg_len <= 4; reg_len*=2){
		twolev_pap_predictor(trace_file, tab_size, reg_len);
	}
	
	// call the two-level GAp predictor with different setups
	for(tab_size=512; tab_size <= 4096; tab_size *= 2){
		for(glob_hist_len=2; glob_hist_len<=8; glob_hist_len*=2){
			twolev_gap_predictor(trace_file, tab_size, glob_hist_len);
		}
	}
	tab_size = 8192;
	for(glob_hist_len = 2; glob_hist_len <= 4; glob_hist_len*=2){
		twolev_gap_predictor(trace_file, tab_size, glob_hist_len);
	}
	
	// call the gshare predictor with different setups
	for(tab_size=512; tab_size <= 4096; tab_size *= 2){
		for(glob_hist_len=2; glob_hist_len<=8; glob_hist_len*=2){
			gshare_predictor(trace_file, tab_size, glob_hist_len);
		}
	}
	tab_size = 8192;
	for(glob_hist_len = 2; glob_hist_len <= 4; glob_hist_len*=2){
		gshare_predictor(trace_file, tab_size, glob_hist_len);
	}
	
	return 0;
}//main()
