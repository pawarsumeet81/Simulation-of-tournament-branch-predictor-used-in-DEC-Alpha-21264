/* Author: Surakshith Reddy Mothe, Venkata Lakshman Tummala, Sumeet Pawar;  
 * Description: This file defines the two required functions for the branch predictor.
*/


#include "predictor.h"

// --------------------- Implementing the local, global and selector tables & prediction states ---------------------

// output of the local predictor
bool localprediction_out;  

// output of the global predictor                            
bool globalprediction_out;                              


// local history table with 1024 entries
// local history of last 10 branches maintained
struct localhistory_entry { unsigned index : 10; };    
struct localhistory_entry localhistory_table[1024];


// local predictor table with 1024 entries
// every entry is a 3-bit saturating counter
struct localpredictor_entry { unsigned index: 3; };
struct localpredictor_entry localpredictiontable[1024];


// global predictor table with 4096 entries
// every entry is a 2-bit saturating counter
struct globalpredictortable { unsigned table: 2; };
struct globalpredictortable globalprediction[4096];


// selector predictor table with 4096 entries
// every entry is a 2-bit saturating counter
struct selectorpredictortable { unsigned table: 2; };
struct selectorpredictortable selectorprediction[4096];


// path history for last 12 branches
struct pathhistoryarray { unsigned history : 12;} ;
struct pathhistoryarray pathhistory;


// ----------------------------- initialization of the DEC Alpha 21264 predictor -----------------------------
void initialize ()
{
// initializing the local history and local prediction state bits
    for (int i=0; i<1024; i++)
    {
        localhistory_table[i].index = 0;
        localpredictiontable[i].index = 0;
    }

// initializing the global prediction and selector prediction bits
    for (int i=0; i<4096; i++)
    {
        globalprediction[i].table = 0;
        selectorprediction[i].table = 0;
    }

// initializing the path history    
    pathhistory.history = 0;
}

// Calling the initialize function
void initialize();


// ----------------------------- Get prediction function to obtain our prediction -----------------------------
    bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
        {
           
            // Grabbing the last 10 bits of the instruction address
            unsigned int maskbits = 0xFFC;
            unsigned int PCbits_masked = (br->instruction_addr) & maskbits;
            unsigned int PCbits = PCbits_masked >> 2;
            // Declarations of local variables used in get_prediction()
            bool prediction;
            unsigned short localindex;
            unsigned short localstate;
            unsigned short globalindex;
            unsigned short globalstate;
            unsigned short selectorindex;
            unsigned short selectorstate;
           
            // Prediction is only made for conditional branches
            if (br->is_conditional)
                {
                    // Indexing into the local prediction table using PCbits to obtain the 10-bit local history
                    // This 10-bit local history used to index into the local prediction table to obtain the local prediction
                    localindex = localhistory_table[PCbits].index;
                    localstate = localpredictiontable[localindex].index;
                    localprediction_out = ( localstate > 3) ? true : false;

                    // Indexing into the global prediction table using path history to obtain the global prediction
                    globalindex = pathhistory.history;
                    globalstate = globalprediction[globalindex].table;
                    globalprediction_out = (globalstate > 1) ? true : false;

                    // Indexing into the selector prediction table using path history to obtain selector prediction
                    selectorindex = pathhistory.history;
                    selectorstate = selectorprediction[selectorindex].table;
                   
                    // Obtaining the final prediciton value
                    prediction = (selectorstate > 1) ? localprediction_out : globalprediction_out;
                }

            // For branches other than conditional, prediction is always returned true
            else
                {
                    prediction = true;
                }

                // Returning the prediction value
                return prediction;
        }


// ------------------- Update predictor function to update our prediction tables and state bits -------------------
    void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
        {

            // Declarations of local variables used in update_predictor()
            unsigned int maskbits = 0xFFC;
            unsigned int PCbits_masked = (br->instruction_addr) & maskbits;
            unsigned int PCbits = PCbits_masked >> 2;
            unsigned short localindex;
            unsigned short localstate;
            unsigned short globalindex;
            unsigned short globalstate;
            unsigned short selectorindex;
            unsigned short selectorstate;

            // Prediction tables and states are updated only for conditional branches
            if (br->is_conditional)
            {

                // Obtaining the local prediction
                localindex = localhistory_table[PCbits].index;
                localstate = localpredictiontable[localindex].index;
                localprediction_out = ( localstate > 3) ? true : false;

                // Updating the 10-bit local history
                localhistory_table[PCbits].index = (localhistory_table[PCbits].index) << 1;
                if (taken)        
                {
                    localhistory_table[PCbits].index++ ;
                }
                       
                // Updating the 3-bit local prediction saturating counter
                if (taken)
                {  
                    if (localstate < 7)
                        localstate++;
                    else
                        localstate = localstate;
                }
                else
                {
                    if  (localstate > 0)
                        localstate--;
                    else
                        localstate = localstate;
                }
                localpredictiontable[localindex].index = localstate;

                // Obtaining the global prediction
                globalindex = pathhistory.history;
                globalstate = globalprediction[globalindex].table;
                globalprediction_out = (globalstate > 1) ? true : false;

                // Updating the 2-bit global prediction saturating counter
                if(taken)
                {
                    if (globalstate < 3)
                        globalstate++;
                    else
                        globalstate = globalstate;
                }
                else
                {
                    if (globalstate > 0)
                        globalstate--;
                    else
                        globalstate = globalstate;
                }
                globalprediction[globalindex].table = globalstate;

                // Obtaining the selector prediction
                selectorindex = pathhistory.history;
                selectorstate = selectorprediction[selectorindex].table;

                // Updating the 2-bit Selector prediciton saturating counter
                if (localprediction_out == taken && globalprediction_out != taken)
                    {
                        if (selectorstate < 3)
                            selectorstate++;
                        else
                            selectorstate = selectorstate;
                    }
                else if (localprediction_out != taken && globalprediction_out == taken)
                    {
                        if (selectorstate > 0)
                            selectorstate--;
                        else
                            selectorstate = selectorstate;
                    }
                else
                    selectorstate = selectorstate;
                selectorprediction[selectorindex].table = selectorstate;

                // Updating the 12-bit path history depending upon the actual outcome of the branch
                if (taken)
                {
                    pathhistory.history = (pathhistory.history) << 1;
                    pathhistory.history = (pathhistory.history) + 1;
                }
                else
                    pathhistory.history = (pathhistory.history) << 1;
            }
       
            else;
            } 