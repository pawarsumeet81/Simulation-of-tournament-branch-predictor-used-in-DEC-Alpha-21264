////////////////////////////////////////////////////////////////////////////////
//
//	AUTHOR 	: Venkata lakshman Tummala, Surakshit Reddy Mothe, Sumeet Subhash Pawar
//	FILENAME: Competetive_predictor.CC
//
//	DESCRIPTION: TOURNAMENT PREDICTOR WITH GLOBAL & SELECTOR PREDICTOR AS GSHARE.
//
////////////////////////////////////////////////////////////////////////////////


#include "predictor.h"
#include "predictor.h"

											// Global variable for local predictor.
bool localprediction_out;
											// Global variable for global predictor.
bool globalprediction_out;
											// Global variable for final/selector predictor.
bool prediction;
											// Local history table with 1024 entries with each entry of 10 bits.
struct localhistory_entry { unsigned index : 10; };
struct localhistory_entry localhistory_table[1024];
											// Local predictor table with 1024 entries with each entry of 3 bits(state bits).
struct localpredictor_entry { unsigned index: 3; };
struct localpredictor_entry localpredictiontable[1024];
											// Global predictor table with 4096 entries with each entry of 3 bits(state bits).
struct predictortable { unsigned table: 2; };
struct predictortable predictiontable[4096];
											// Global predictor table with 4096 entries with each entry of 2 bits(state bits).
struct selectorpredictortable { unsigned table: 2; };
struct selectorpredictortable selectorprediction[4096];
											// Path history consisting of last 12 branch entries.
struct pathhistoryarray { unsigned history : 12;} ;
struct pathhistoryarray	pathhistory;

    bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
        {
			unsigned int maskbits = 0xFFC;													// Mask bits used for getting [11:2] of pc.
			unsigned int PCbits_12 = (br->instruction_addr) & maskbits;
			unsigned int PCbits = PCbits_12 >> 2;
			unsigned int index = PCbits_12 ^ pathhistory.history;								// xor of pc with pathhistory for gshare predictor.
			
			unsigned int localindex;		// Internal variables.
			unsigned int localstate;
			
			unsigned int predictorstate;
					 
			unsigned int selectorindex;
			unsigned int selectorstate;
			
            if (br->is_conditional)															// Prediction is done only for conditional branches.
                {
                    localindex = localhistory_table[PCbits].index;
                    localstate = localpredictiontable[localindex].index;
                    localprediction_out = ( localstate > 3) ? true : false;					// Local predictor output based on its state's MSB bit.

					predictorstate = predictiontable[index].table;
                    globalprediction_out = (predictorstate > 1) ? true : false;				// Global predictor state based on its state's MSB bit.

					selectorstate = selectorprediction[index].table;
					prediction = (selectorstate > 1) ? localprediction_out : globalprediction_out;
                }
                else																		// always predicted for uncondtional branches.
                {
                    prediction = true;
                }
        return prediction;
        }

    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)	// updating the states and histories.
    {
        	unsigned int 	maskbits = 0xFFC;
			unsigned int 	localindex;
			unsigned int 	localstate;
			unsigned int	globalpredstate;
		
			unsigned int PCbits_12 = (br->instruction_addr) & maskbits;
			unsigned int PCbits = PCbits_12 >> 2;
			//unsigned int PCbits = (br->instruction_addr) & maskbits;
			unsigned int gindex = PCbits_12 ^ pathhistory.history;

		
		
		if (br->is_conditional)
		{																							// Local history. 
			localindex = localhistory_table[PCbits].index;
            localstate = localpredictiontable[localindex].index;
			localprediction_out = ( localstate > 3) ? true : false;
			localhistory_table[PCbits].index = (localhistory_table[PCbits].index) << 1;
            if (taken){        																					
                localhistory_table[PCbits].index++ ; }        
            if (taken){ 
                if (localstate < 7)
                    localpredictiontable[localindex].index++;}
			else{
				if	(localstate > 0)
                    localpredictiontable[localindex].index--;}
																									// Global history.
			globalpredstate = predictiontable[gindex].table;
            globalprediction_out = (globalpredstate > 1) ? true : false;
			if(taken != globalprediction_out){
				if (taken)
                  (predictiontable[gindex].table)++;
            else
           		(predictiontable[gindex].table)--; }
																									// Selector states.
            if (localprediction_out == taken && globalprediction_out != taken){   
                if (selectorprediction[gindex].table < 3)
                    (selectorprediction[gindex].table)++;}                
            else if (localprediction_out != taken && globalprediction_out == taken){                
                if (selectorprediction[gindex].table > 0)
                    (selectorprediction[gindex].table)--;}
																									// pathhistory.
            if (taken){            
                pathhistory.history = (pathhistory.history) << 1;
                pathhistory.history = (pathhistory.history) + 1;}
            else
                pathhistory.history = (pathhistory.history) << 1;
		}
		else;
	}




