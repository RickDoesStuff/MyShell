// for reading in from a terminal, given or current
#include "terminalstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 * arrSize, pointer to the amount of pointers in the array
 * arr, pointer to 2d char array malloced with the size in arrSize already
 * wordCount, pointer to a int that will be set to the amount of index's in the array, words, pipes, etc, but not the word that is NULL
*/
int terminalStream(int *arrSize, char ***arr, int *wordCount){

    // allocate array
    *arr = malloc(*arrSize * sizeof(char*));
    if (*arr == NULL) {perror("Couldn't allocate array"); return -1;}
    for (int i = 0; i < *arrSize; i++) {
        (*arr)[i] = NULL; // Initialize each word pointer to NULL
    }

    // allocate buffer
    int bufSize = 128;
    char *buf = malloc(bufSize);
    if (buf == NULL) {perror("Couldn't allocate buffer"); return -1;}

    int wordLen = 0; // num of chars in current word 
    int currentWord = 0; // current index in the 2d array
    int currentWordSize = 8; // 8 chars

    while (1) {

        // only read bufSize - 1 bytes to save room for string terminator
        int numBytesRead = read(STDIN_FILENO, buf, bufSize - 1);

        // has bytes to read
        if (numBytesRead > 1) {

            // iterate through all the bytes, will not run out of memory I think
            for (int currentPos = 0; currentPos < numBytesRead; currentPos++) {
                
                // Check if need to expand array
                if (currentWord >= *arrSize - 1) { 
                    *arrSize *= 2; // double the size of the array
                    *arr = realloc(*arr, *arrSize * sizeof(char*));
                    if (*arr == NULL) { perror("Couldn't reallocate array of words"); return -1; }
                    // Initialize new pointers to NULL
                    for (int i = currentWord + 1; i < *arrSize; i++) {
                        (*arr)[i] = NULL;
                    }
                }

                char chr = buf[currentPos]; // get the current char we are checking
                DEBUG printf("Char:\'%c\'\n",chr);
                

                // ensure current word is allocated
                if ((*arr)[currentWord] == NULL) {
                    (*arr)[currentWord] = malloc(sizeof(char) * currentWordSize);
                    if ((*arr)[currentWord] == NULL) { perror("Couldn't allocate word"); return -1; }
                }
                // save room for string terminator 
                else if (wordLen + 1 >= currentWordSize) {
                    currentWordSize+=currentWordSize;
                    (*arr)[currentWord] = realloc((*arr)[currentWord], currentWordSize);
                    if ((*arr)[currentWord] == NULL) { perror("Couldn't reallocate word"); return -1; }
                }   


                // check if current char is a newline char 
                if (chr == '\n') {

                    (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                    currentWord++;
                    // maybe realloc the size of the arr if we run out of index's to use
                    (*arr)[currentWord] = NULL;    // put NULL as the last index in the 2d array
                    *wordCount = currentWord+1;
                    return 1; // return with success
                } 

                // check if current char is a space
                else if (isspace(chr)) {

                    // if there is something already in this word
                    if (wordLen > 0) {
                        DEBUG printf("isspace");
                        // space comes after a letter
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                        currentWord ++;
                        wordLen = 0;  // current len of word
                        currentWordSize = 8;
                    } else {
                        // there isnt anything in the word yet
                        //skip the char
                    }
                }

                // check if current char is a pipeline
                else if((chr == '|')) {

                    // check if the pipe is the start of the word
                    if(wordLen > 0) {
                        // pipe isnt the start of the word
                        // ./mych text.exe|helper.exe -> text.exe,|,....
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate current word
                        currentWord ++;    // increase the index
                        
                        // maybe realloc the size of the arr if we run out of index's to use
                    }

                    // ./mych text.exe | helper.exe -> text.exe,|,....
                    (*arr)[currentWord] = malloc(2 * sizeof(char)); // Space for '|' and '\0'
                    (*arr)[currentWord][0] = '|';
                    (*arr)[currentWord][1] = '\0';
                    
                    currentWord ++;    // increase the index
                    wordLen = 0;  // start over since new word
                    currentWordSize = 8;
                }else{
                    // not a |, space, or new line

                        (*arr)[currentWord][wordLen] = chr; // Append character
                        (*arr)[currentWord][wordLen + 1] = '\0'; // Null-terminate
                        wordLen++;
                }
            }
        } else {
            // no bytes to read
            //printf("no bytes to read\n");
            //fflush(NULL);
            
            printf("\nmysh> ");
            //return 0;
        }
    }

    return 0; // didn't return or crash, it failed
}
