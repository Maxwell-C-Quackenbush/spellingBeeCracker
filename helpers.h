/*
Helping and debugging functions used across projects.
None of these functions should call other functions defined elsewhere in this project.

*/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>



/*
This is a helper functoin that finds the index of a numbered gene.
It is often used in random selecton for mutations.
Note that similar code is used in mutation functions.

find_Nth_gene is not called in mutation functions to reduce calls.
Any future efficiencies may be applied there as well.

*/
int find_Nth_gene(uint32_t puzzle, int N){
    uint32_t k = puzzle;
    for(int i=1; i<26; i++){
        switch (k % 2){ //read least significant bit

            case 1: // Character is Present in our vector
                N--; //seen one additional    //remove the component of the vector by subtraction.
                break;

            default: k; //if not a one, do nothing.

        }
        k = k >> 1;
        if (N==0){
            return i-1;
        }
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


/*This function turns our uint32_t into a string of characters
 * it works integers that have more or less than 7 characters.
 * */
char* key_to_string(char* buffer, uint32_t key){
    uint32_t k = key; //copy for cleanliness, we will be altering the value
    int lettersFound = 0; //position

    //uint32_t lint = 1;
    for(int i=0; i<26;i++){
      if ( (k % 2) != 0 ) { //is the trailing value 1?
        //then this letter is in the string!
        buffer[lettersFound] = char(96+i);
        lettersFound++;
      }

      k = k >> 1; //slide... to the right! 
                //examine the next bit on the next cycle
    }

    return 0;
}
