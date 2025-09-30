#include <iostream>
#include <fstream>
//#include <regex>
#include <algorithm>

#include <tgmath.h>
#include "genetics.h"


#include <cstdlib>


using namespace std;



/* This program can serve two functions:  
 *  -> Solve a given puzzle quickly with a given dictionary
 *  -> Find large / small puzzles with a given dictionary
 * 
 */

/*
 * words in dictionary are encoded into N-hot vectors. For example, ABBA becomes: 
 * 
 *  ...0000111
 *      ^^^^^^
 *      EDCBA/
 * 
 * the trailing bit (as "/") is there to make other operations cleaner later on.  
 * 1 indicates that the letter is present at least once in the word. Because there are no Cs, the value is 0.
 * this encoding is also used to represent puzzle vocabularies, center letters, and potentially more.
 * 
 * Vecotrs in this list are not unique. some words will have the same vector. for example 
 *     anagrams:   
 *         "impugn" and "umping",  [gimnpu]
 *     same base letters:
 *         "stall", "stalls"       [alst]
 * Some vectors may be impossible to satisfy because they contain more than 7 distinct letters. 
 * Words longer than 7 letters may still be valid if they contain duplicate letters.
 *  
 */
uint32_t* makeDict(uint32_t * wVecs, int dict_count, uint32_t key, bool show=false ) {
    ifstream fio("words.txt", ios::in);
    string s;

    if (!fio.is_open()) {
        cerr << "Error opening the file!";
    }


    int lWord = 0;
    int i = 0;      //declaring flag variables outside of scope can lead to fewer memory allocations
    int j = 0;

    uint32_t wVec = 0; //local vector. the vector for a single word.

    uint32_t thisLetter = 0;
    uint32_t oneHot = 0;


    //std::regex pattern(R"(^[a-zA-Z]+,\".*)");
    //std::regex pattern(R"(.*)");

    while (getline(fio, s) && i < dict_count) {

        //std::cout << "Enter a line: ";
        
        if (true/*std::regex_match(s, pattern)*/) {
            // while loop to generate an N-hot vector representing given line.
                //Format note: the place value is shifted one to the right, so ""...00000010" is "a". 
                //the least significant bit is often set to 1. Some bitwise arithmetic is easier if we have a spare space.

            while (j <= 15){ //15 because it is the chosen (arbitrary) max length of a word

                thisLetter = uint32_t(s[j] - 96); //represent the current letter's position in the alphabet.
    
                if(thisLetter == 4294967200) {j=15;} //if this is the end of the string, set J to the final value.
                                                //WARNING: May break in alternate encodings of end of string.
    
                thisLetter = (thisLetter <= 0 || thisLetter >= 128)? 0 : thisLetter; //set to zero if outside a reasonable character bound.
            
    
                oneHot = (1 << (thisLetter)); //create a one-hot variable for this letter. 
                wVec = (wVec | oneHot); // the binary OR sets the value at the index to one. Addition would lead to overflow.
                                        // example: "spell" becomes "emps" instead of "elps",
                                        //  (q+ 2^12) + 2^12 => (q+2^13)   
                                        // where q is the n-hot vector at the time of the oprtation.
                                        // q may also be considered as a set of powers of two.
    
                j++;
                printf("");
            }
            
            /*
            foo = VOCAB;
            bar = ~VOCAB;
            foobar = bar | lVec;
            bar =(VOCAB | ~lVec);
            foobar = ~bar;
    
            if(  foobar == 1  ){
                // This section does not filter for center letters
                //printf("VALID WORD : ");
                //cout << s << endl;
            }*/

        } else {
            //std::cout << "no match " << i << std::endl;
        }




        wVecs[i] = wVec;
        wVec = 0;
        j= 0;
        //cout << lWord << endl;
        i++;
    }//end while loop iterating through file

    std::cout << "Completed reading dictionary file." << std::endl;


    fio.close();
    return wVecs;
}

/* This function takes a string of lowercase characters 
It generates an N--hot vector representing the vocabulary. 

the function does NOT account for any required center letters.

if there are duplicate letters in the string, (ex. "ABBA") the output will only be hot at A and B indexes. 
*/
uint32_t makeKey(string s){
    uint32_t i = 0;
    uint32_t lim = uint32_t(s.length()-1); //size of string / limit
    uint32_t flag = 0;
    uint32_t this_letter = 0;

    for(i = 0; i<lim; i++ ) {
        //index -96, brings us from start of lowercase ASCII letters to 0. 
        uint32_t temp = uint32_t(s[i]-96);

        this_letter = 1;
        this_letter = this_letter << temp;
        flag = flag | this_letter;
    }
    return flag;


}







/*This function turns our uint32_t into a string of 7 characters
 *
 * */
char* key_to_string(char* buffer, uint32_t key){
    uint32_t k = key; //copy for cleanliness, we will be altering the value
    int letters_found = 0; //position

    //uint32_t lint = 1;
    for(int i=0; i<26;i++){
      if ( (k % 2) != 0 ) { //is the trailing value 1?
        //then this letter is in the string!
        buffer[letters_found] = char(96+i);
        letters_found++;
      }

      k = k >> 1; //slide... to the right! 
                //examine the next bit on the next cycle
    }

    return 0;
}



//creating binary integers for all letters:
/*
 * let w be the word
 * let N be the integer encoding known values
 * let N be all 0
 *
 * for each c in w,
 *      represent c as a one-hot variable:
 *      declare 1, and then bitshift to left by ORD(c). assign to integer V
 *      N = N | V
 *
 * at the end, duplicate letters are preserved.
 *
 * */
int generate_keys(uint32_t vocabs[], uint32_t centers[], int count){
    std::cout << "Generating Keys...   [" << count << "]" << std::endl;
    //uint32_t * vocabs = (uint32_t*) malloc ( count * sizeof(uint32_t) ) ;
    //uint32_t * centers = (uint32_t*) malloc ( count * sizeof(uint32_t) ) ;
    int rd = 0;
    uint32_t selection = 0;
    uint32_t l_candidate = 0;
    float bar = 0;
    float r;

    for(int i=0; i< count; i++){ //st
        //rd = rand() % 26;
        //std::cout << i << " " << rd  << std::endl;
        int center = 0;
        int choices = 7; //number of letters in the puzzle.
        int curr_let = 0; //our current letter
        for(int j=0; j<=25; j++){ //do 26 times, one for each letter of english alphabet. start at 1 to protect leading zero space.
            r =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            bar = float(choices) / float(25 - curr_let) ;
            //r = r % bar;
            if (r < bar) { //rolled well! select this latter!
                choices--; //one fewer choice in future
                selection = selection + (uint32_t(1) << uint32_t(curr_let + 1));
                //now we ask: is this the center letter?
                if (center == 0) { //if center has not been selected..
                    if(curr_let == 6) //if this is the last letter, we MUST choose it
                        center=uint32_t(curr_let +1);
                    else{
                        bar = float(choices) / float(7 - curr_let);
                        r =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                        if (r < bar){
                            center= uint32_t(curr_let +1);
                        }

                    }
                }
            }
            curr_let++; //examine the next letter in our stack
        }

        /* potential faster method. variable runtime.
        for(int j=0; j<7; j++){
            rd = rand() % 26;
            l_candidate = uint32_t(2) << rd << 1; //shift an extra 1 due to format
            //if there is a matching 1 in selection, we catch it!
            foo = (l_candidate ^ selection );
            if((foo - selection) == 0){
                std::cout << "panic!" << std::endl;
                j--;
            }
            if((foo - selection) != 0){
                std::cout << "Valid term " << rd << std::endl;
                selection = l_candidate ^ selection;
            }
        }//end "loop of seven"
        */
        //std::cout << "our new key:" << selection << std::endl;
        vocabs[i] = selection;
        centers[i] = 1 << center +1;
        selection = 0;


    }//end loop of count Row

    return 0; //std::make_pair( vocabs,  centers);
}


int save_arr_to_file(int* arr, int size ){

    ofstream outfile("array.txt");
    if (!outfile.is_open()) {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }
    // Writing the array elements to the file
    for (int i = 0; i < size; ++i) {
        outfile << arr[i] << " ";
    }

    // Closing the file
    outfile.close();

    return 0;

}






//Non-functional debug function. turns strings of characters into vectors.
int xatov(char *s, int len) {
    int result [10];
    int i=0;
    while( i<= 10) {

        int nu = int(s[i]);
        //-97 because 97 is start of lowercase letters
        result[i] = nu -96 <= 0? 0 : nu-96 ;
        printf("iteration %d\n", i);
        i++;
    }

    return 0;
}
/**
 * Debug Function.
 * 
 * Visually compare arrays of Uint32 in the console.
 * 
 * The Output is not formatted, so integers will be misaligned.
 * Best for smaller arrays, or subsections.
 * 
 * A subsection can be selected by changing the input pointer from:
 *      compare_key_arrs(arr1, arr2, 100)
 * to
 *      compare_key_arrs(arr1[25], arr2[25], 50)
 * This will compare from index 25 for an additional 50 spaces (until [75]). Note these two values should be the same.
 */
int compare_key_arrs(uint32_t* arr1, uint32_t* arr2, int size){
    for(int i=0; i<size; i++){
        std::cout << i << "  " << arr1[i] << "    " << arr2[i] << std::endl;
    }
    return 0;
} 


/**
 * return index of highest value in an array of scores.
 * used during debugging for additional information.
 */

int generation_info(int* scores, int size ){
    //average score
    int flag = 0;
    int indexLargest = 0;
    int valueLargest = 0;
    int scr; //declare outside of scope for speed
    for(int i=0; i<size; i++){
        scr = scores[i];
        flag = flag + scr;
        if(scr > valueLargest){
            valueLargest = scr;
            indexLargest = i;
        }
    }
    //std::cout << "Generation average score: " << (float)flag / (float)size << std::endl;
    return indexLargest;
}



/* Save a generation to file. 
 * TODO: add planned feature
 */
int savegeneration(uint32_t vocabs, uint32_t centers, int size, int * scores, char* filename){
    return 0;
}

int main() {
    int test_count = 2000;

    uint32_t *vocabs = (uint32_t *) malloc(test_count * sizeof(uint32_t));
    uint32_t *centers = (uint32_t *) malloc(test_count * sizeof(uint32_t));

    //generate keys for first generation
    generate_keys(vocabs, centers, test_count);


    //generate dictionary
    int dict_count = 370099; //from the file. hardcoded temporarily.
    uint32_t *dict = (uint32_t *) malloc( dict_count * sizeof(uint32_t));
    makeDict(dict, dict_count, vocabs[0]);


    //now we are going to shuffle the dictionary
        //this is useful because it will allow us to easily take RANDOM samples of our dictionary for use in 
    
    //create a range of integer indexes to shuffle
    int *idxs = (int*) malloc(dict_count*sizeof(int));

    // re-order the indexes randomly
    //re-order the N-hot vectors according to the new order
        //create a new temporary array of each datatype
        //assign to vocabs
        //assign to centers
        //cleanup the backup arrays
        //TODO: add functionality to re-order the literal words.
            //this may involve creating a second, complimentary IDXs table.
        



    // test generated keys
    int scores[test_count];
    int scores2[test_count]; //alternate array of scores







    char* buffer = (char*) (calloc(sizeof(char),10)); 
    char* c_buffer = (char*) (calloc(sizeof(char),10)); 






    //check_dict_gpu()
    //cudaMallocManaged(&dict, size*sizeof(uint32_t) );
    //cudaFree(dict);


    //

    ofstream outfile;
    outfile.open("summary.csv");
    int data[] = {0,0,0,0,0};
    //data = [i, avg, runts, survivors, bestScore]

    uint32_t best_key = 1859618;
    uint32_t best_center = 2;

    //uint32_t *next_v = (uint32_t *) malloc(test_count * sizeof(uint32_t));
    //uint32_t *next_c = (uint32_t *) malloc(test_count * sizeof(uint32_t));
    uint32_t *next_v = (uint32_t *) calloc(sizeof(uint32_t), test_count);
    uint32_t *next_c = (uint32_t *) calloc(sizeof(uint32_t), test_count);
    uint32_t *holder;
    
    // DEBUG: test the encoding of the keys. Should be  aemnrst
    //key_to_string(buffer, 1859618);
    //std::cout << buffer <<std::endl;


    // GENETIC LOOP

    int gens = 35; //number of generations to go  through
    for(int i=0; i<gens; i++){
        std::cout << std::endl; //newline 
        std::cout << "generation #" << i << std::endl;
        int threshold = calc_fitness(dict, vocabs, centers, scores, test_count, dict_count, 100);
        threshold = threshold * 1.05;
        //next_v and next_c are now loaded.
        //std::cout << "first 5:" << vocabs[1] << " "<< vocabs[2]<< " " << vocabs[3]<< " " << vocabs[4]<< " " << vocabs[5] << std::endl;

        int bestIndex = generation_info(scores,  test_count);
        int survivors = fit_samples(vocabs, centers, scores, test_count, threshold, next_v, next_c, scores2, test_count);

        uint32_t bestKeyA = vocabs[bestIndex];
        uint32_t bestKeyB = next_v[bestIndex];


        // debug print statements
        
        //show highest score
        key_to_string( buffer, vocabs[bestIndex]);
        uint32_t test = centers[bestIndex];
        std::cout << "Best Puzzle: " << buffer << "   C="<< centers[bestIndex] <<" with "<< scores[bestIndex] <<  std::endl; //double endl for space
       
        // Debug Statements
        //std::cout << "as " << vocabs[bestIndex] << std::endl;
        //key_to_string( buffer, next_v[bestIndex]);
        //std::cout << "             " << buffer << " is the best here" << std::endl;
        //std::cout << "Best Puzzle: [" << buffer << "] C="<< /*next_c[bestIndex] <<*/" with "<< scores[bestIndex] <<  std::endl << std::endl; //double endl for space
        //std::cout << "as " << vocabs[bestIndex] << std::endl << std::endl;
        //std::cout << vocabs[bestIndex] << " At index " << bestIndex << std::endl;
        //compare_key_arrs(vocabs, next_v, test_count);


        //organize data to export
        data[0] = i;
        data[1] = threshold; //average score without runts
        data[2] = survivors;
        data[4] = scores[bestIndex];

        //export data
        for(int d=0; d<5; d++){
            //std::cout << data[d] << ", "; //debug, readline as seen in csv file.
            outfile   << data[d] << ", ";
        }

        char cChar = char(std::log(next_c[bestIndex] >> 1)+96); //extremely simple 1hot to character

        outfile   << buffer << ','; //write highest scoring vocab to the summary
        outfile   << std::endl;


        //generation complete. Prepare next generation 
        mutate_samples_n(vocabs, centers, test_count, threshold, vocabs, centers );


        //now we will switch the labels on the arrays.
        //this is done to save memory.
        holder = next_v;
        next_v = vocabs;
        vocabs = holder;

        holder  = next_c;
        next_c  = centers;
        centers = holder;
    }
    outfile.close();


    //now, we can optionally save our results:
    //save_arr_to_file(scores, test_count);

    free(dict);
    free(vocabs);
    free(centers);
    free(next_v);
    free(next_c);
    free(buffer);
    free(c_buffer);
    free(idxs);
    system("pause");
    return 0;
}
