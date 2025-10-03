#include <cstdlib>

class Generation {
    public:
        uint32_t * vocabs;
        uint32_t * centers;
        int * scores;

        int size;
        int lastParent = 0;




        Generation (int sizeIn){
            size = sizeIn;

            vocabs  = (uint32_t *) malloc(sizeIn * sizeof(uint32_t));
            centers = (uint32_t *) malloc(sizeIn * sizeof(uint32_t));
            scores  = (int *) malloc(sizeIn * sizeof(int));

        }
        /*
        Free the memory containing our  
        */
        int kill(){
            free(vocabs);
            free(centers);
            free(scores);
            return 0;
        }
        /*At the start of this function, the generation is a newly created child.
        This function is not to be called on the "progenitor" generation. 
        this function requires:
            an existing population inside of the generation to mutate FROM
            space inside of the population to mutate TO

            A pointer to a method for MUTATING from a parent


                TODO: make this method support
        */
        int createOffspring(int(*f)(int, int) ){
                        if ( lastParent != 0 ){
                std::cout << "Error! No parents to populate from!" << std::endl;
                return 1;
            } 
            
            return 0;
        }


        int reproduce(){
            return 0;
        }
};

struct Puzzle{
    uint32_t vocab;
    uint32_t center;
    uint32_t score;
};

