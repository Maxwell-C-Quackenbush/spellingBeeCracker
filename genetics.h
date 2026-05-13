#include <cstdlib>
#include <iostream>
#include <cstdint>


void say_hi(){

    std::cout << "Hello from genetics.h!" << std::endl;

}


int mutate_one(uint32_t key, uint32_t cent, uint32_t * childV, uint32_t * childC){
    //change one letter
        //if it is the key letter, the mutation will not be useful
        //they key will remain unchanged, and fitness will likely suffer.
        // oh well. that's evolution, baby!

    //select a random number, 0-6 inclusive. this is our letter to swap.
    //select a random numer, 0-18 inclusive. this is our new number.

    float r1 =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    
    //add one to values, because if one of the random integers is Zero, we will shoot below 0 on the first loop
    //and fail to trigger the x==0 branch condition.
    int oldChar = int(r1 * 7) +1;   // the Nth character in the set of present characters to swap
    int newChar = int(r2 * 18) +1;  // the Nth character in the set of absent characters to swap
    uint32_t newOneHot;


    if(key==0){
        //hotfix for strange error elsewhere in program flow.
        //empty keys should never score well enough to mutate. 
        //With proper use of the framework, this should never be triggered.
        return 0;
    }
    uint32_t k = key >> 1 ; //copy to preserve data, remove leading bit.
                            // allocated in stack, not heap-> faster memory recall?

    //now we loop through to find what changes we make

    for(int i=1; i<26; i++){

        switch (k % 2){ //read least significant bit

            case 1: // Character is Present in our vector
                oldChar--; //seen one additional old character
                //if we have seen the proper number of prior characters, Old char will be zero.

                // a faster way of comprehension:
                newOneHot = bool(oldChar == 0) << i; //when on the proper character, bool evals to 0. all other cases 1.
                key = key - newOneHot;              //remove the component of the vector by subtraction.

                /** Prior code; slower than boolean casting in some compilers and CPUs.
                    Kept for reader clarity/

                if (oldChar == 0 && false){
                    //we have found the character that we are looking for!
                    newOneHot = 1 << i;    //creante N-hot for mutation
                    key = key - newOneHot;          //set to 0
                }*/
                break;

            case 0:
                newChar--; //seen one additional non-present character
                //if we have seen the proper number of prior characters..

                newOneHot = bool(newChar == 0) << i; //when on the proper character, bool evals to 0. all other cases 1.
                key = key + newOneHot;              //remove the component of the vector by subtraction.

                /*
                Prior code; slower than boolean casting in some compilers and CPUs.
                    Kept for reader clarity
             
                if(newChar == 0){   
                    //we have found the character we are looking for!
                    newOneHot = 1 << i;    // create N-hot for mutation
                    key = key + newOneHot;          // set to ONE by adding.
                }
                    */
                break;


        default:
            k; //do nothing
        }

        //we have processed the final character for the next gnearation.
        k = k >> 1; //shift for next iteration
        if (oldChar <= 0 && newChar <= 0){
            break; //break loop if we have found both characters
                    //branch prediction means that this will not greatly slow down throughput after the first run.
        }

    }
    
    *childV = key;
    //childC = childC;


    if(int(key) == int(1859618)){
        key=0;
    }

    return 0;
}


/* This function returns the number of 1s in an N-hot vector
it returns 1 if the key has more than 7 positive bits, and 0 otherwise.
*/
int test_key_charcount(uint32_t * keyOriginal){
    int flag = 0;
    uint32_t key = * keyOriginal;
    for(int i=0; i<26; i++){
        switch(key % 2){
            case 1:
                flag++;
                //no break statement. deliberate choice
            case 0:
                key = key >> 1;
            break;
        }
    }
    //std::cout << "Letter Count is " << flag << std::endl;
    if(flag != 7){
        //std::cout << "HIGH LETTER COUNT!" << flag << std::endl;
        return 1;
    }

    return 0;


}


int check_dict_cpu(uint32_t * wVecs, int count, uint32_t key, uint32_t centerLetter ){
    //std::cout << "checking key: " << key << std::endl;
    //declare variables outside of loop for added speed
    int foundCount = 0; //value returned
    uint32_t localWord;
    uint32_t isValid;
    uint32_t hasCenter;
    uint32_t foo;

    uint32_t i = 0;
    while (i <= count) {
        localWord = wVecs[i];      //we only want to call the word from memory once.

        foo = ~localWord; // declaring this means that the negation will not have to be done multiple times.
                           // NOTE: Negation is so fast, that waiting for the memory from L1 Cache /may/ be slower than
                           // flipping values already in a registry.

        isValid = ~(key | foo);    //now we check for center char validity.
                                    //checking conditionally will actually add operations and therefore time
                                    
        //see if center_letter is in local_word
        //should be zero if there are no bits that line up.
        hasCenter = centerLetter & localWord;
        // has_center will be non-zero if the local word contains the Center letter.
            // this is true because the binary AND operation, when used on a one-hot vector, can only return the OHV or 0.
            // these outputs are determined by the value of the local word at the "center index" (index of the center letter)
        
        foundCount += int(isValid==1 && hasCenter);

        /* //branch conditions slow down this loop greatly. 
        if((isValid==0 && hasCenter)) {
                foundCount ++;
        }*/

        //the above code was replaced in part with an elegant cast of a boolean to an integer.
        
        

        i++;
    }
    return foundCount;
}


/*
uint32_t * breed(uint32_t key_a, uint32_t cent_a, uint32_t key_b, uint32_t cent_b, int litter){
//for now, we will make a single gene swap.
//map all the "genes" of the 
}*/


/*
The approach:
All scores above average are saved.
Average takes O(N).


All values below a certain threshold are "Runts",  (weakest of the litter) and are not counted in the average.
Evolution will be faster, especially in the beginning. the drawback is that the locality of minima/maxima found are narrower. 

*/
int calc_fitness(uint32_t* dict, uint32_t* vocabs, uint32_t* centers, int* scores, int size, int dictSize, int runtCutoff){
    
    //int scores [size];


    //get scores for this generation
    int total = 0; //keep running average of total scores
    int j = 0;
    int runts = 0;  //number of poor quality puzzles. Do not consider in the average. 
    int myScore;    // holder variable declared in higher scope to reduce score[j] calls

    for(j = 0; j<size; j++){
            myScore = check_dict_cpu(dict, uint32_t(dictSize), vocabs[j], centers[j]);
            scores[j] = myScore;
            total = total + myScore;
            if(scores[j] < runtCutoff){
                runts++;
                total = total - myScore;
            }
    }
    //compute average score
    int avg = int(total / (size-runts)); //may be some rounding errors; that's OK.


    //DEBUG count the number of survivors above the avg.
    int numSurviving = 0;
    for(int i=0; i<size; i++){
        if (scores[i] > avg){ //remove bottom 10% of the samples
            numSurviving ++;
        }
    }
    std::cout << "number of survivors: " << numSurviving << " out of " << size << std::endl; 
    std::cout << "Runt count:" << runts << std::endl;

    

    return avg;
}
/* 
recieves a set of samples and a threshold.

Fills the provided buffer with all valid samples.

returns the number of survivors that "passed" / "Lived" from prior generation
*/
int fit_samples(
            uint32_t* inVocabs,    //size: size_in
            uint32_t* inCenters,   //size: size_in
            int* scores,            //size: size_in
            uint32_t inSize,       

            //minimum value. sometimes average, sometimes skewed.
            int cutoff,
            
            //output buffers
            uint32_t* outVocabs,
            uint32_t* outCenters,
            int* outScores,
            uint32_t outSize       
){
    
    int ptNGen = 0; //pointer in next, actually an index.
    int margin;
    
    for(int i=0; i<inSize; i++){
        margin = cutoff - scores[i];
        if (margin < 0){
            outVocabs[ptNGen]  = inVocabs[i];
            outCenters[ptNGen] = inCenters[i];
            //scores_out[ptNGen]  = scores[i];
            ptNGen++;
        }
        // DEBUG: TESTING MUTATION ERRORS
    }
    return ptNGen;



}//end fit_samples


/*
Recieves an incomplete set of samples, and populates the remainder of the set with mutated versions.
writes offspring to new arrays, beginning with the entry after the last parent. 

The expected contents of Cvocabs and Ccenters include the parents of the prior generation.



The mutation function's presence is modular, and a new muation function can be easily swapped.

TODO: examine impact of including mutate_one code to reduce function calls.
*/
int mutate_samples_n(
    uint32_t* Pvocabs,
    uint32_t* Pcenters,
    int size,
    int lastParent,

    uint32_t* Cvocabs,
    uint32_t* Ccenters
){
    int ptrParent = 0;
    int ptrChild = lastParent;
    
    uint32_t * childVocab; //placeholder for a given child vocab
    uint32_t * childCenter; //placeholder for a given child center
    
    int numTooMany = 0;

    for(int i=lastParent; i<size; i++){
        
        if(i==440){
            numTooMany = numTooMany;
        }
        //pass memory adresses as arguments
        childVocab = &Cvocabs[i];
        childCenter = &Ccenters[i];

        mutate_one(
            Pvocabs[ptrParent],
            Pcenters[ptrParent],
            childVocab,
            childCenter
        );
        //Cvocabs[i] = child_vocab;
        //Ccenters[i] = child_center;

        // DEBUG: during development, the number of selected letters could exceeed 7.
        numTooMany += test_key_charcount(childVocab);
        ptrParent++;
        ptrChild++;

        //we do not want to take copies from the children. 
        if (ptrParent > lastParent){   //if we are at the end of the parent section,
            ptrParent=0;                //restart at beginning.
        }

    }
    // DEBUG: during development, the number of selected letters could exceeed 7.
    //std::cout << "      too many count: " << numTooMany << std::endl;



    return 0;
}




