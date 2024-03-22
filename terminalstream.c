// for reading in from a terminal, given or current
#include "terminalstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 * arr, pointer to 3d char array, cmd index, word index, char index
 * cmdCount, pointer to a int that will be set to the amount of cmds in the array (incuding pipes, if, then, else)
*/
int terminalStream(char ****arr, int *cmdCount) {
    int cmdAmt = 3; // default size for amount of commands in the array//+2;5+4;9+8 //cmdAmt+=cmdAmt-1
    int wordAmt = 5; // default size for the amount of words in a command line
    
    // allocate cmdArray
    *arr = malloc(cmdAmt * sizeof(char*));
    if (*arr == NULL) {perror("Couldn't allocate cmd array"); return -1;}

    for (int i = 0; i < cmdAmt; i++) {
        (*arr)[i] = malloc(wordAmt * sizeof(char*));
        for (int j = 0; j < wordAmt; j++) {
            (*arr)[i][j] = NULL; // Initialize each cmd array to NULL
        }
    }



    // allocate buffer
    int bufSize = 128;
    char *buf = malloc(bufSize);
    if (buf == NULL) {perror("Couldn't allocate buffer"); return -1;}

    int wordLen = 0; // num of chars in current word 
    int currentWord = 0; // current word in the cmd array
    int currentCmd = 0; // current cmd in the array
    int currentWordSize = 8; // 8 chars

    while (1) {

        // only read bufSize - 1 bytes to save room for string terminator
        int numBytesRead = read(STDIN_FILENO, buf, bufSize - 1);
        DEBUG printf("read %i bytes\n",numBytesRead);
        // has bytes to read
        if (numBytesRead > 1) {

            // iterate through all the bytes, will not run out of memory I think
            for (int currentPos = 0; currentPos < numBytesRead; currentPos++) {
                
                // Check if need to expand array
                if (currentWord >= wordAmt - 1) { 
                    wordAmt *= 2; // double the size of the array
                    (*arr)[currentCmd] = realloc((*arr)[currentCmd], wordAmt * sizeof(char*));
                    if ((*arr)[currentCmd] == NULL) { perror("Couldn't reallocate array of words"); return -1; }

                    // Initialize new pointers to NULL
                    for (int i = currentWord + 1; i < wordAmt; i++) {
                        ((*arr)[currentCmd])[i] = NULL;
                    }
                }

                char chr = buf[currentPos]; // get the current char we are checking
                //printf("[currCmd:%i][currWord:%i][wordLen:%i][Char:\'%c\']\n", currentCmd, currentWord,wordLen,chr);
                
                // ensure current word is allocated
                if (((*arr)[currentCmd])[currentWord] == NULL) {
                    ((*arr)[currentCmd])[currentWord] = malloc(sizeof(char) * currentWordSize);
                    if ((*arr)[currentCmd][currentWord] == NULL) { perror("Couldn't allocate word"); return -1; }
                }
                // save room for string terminator 
                else if (wordLen + 1 >= currentWordSize) {
                    currentWordSize+=currentWordSize;
                    (*arr)[currentCmd][currentWord] = realloc((*arr[currentCmd])[currentWord], currentWordSize);
                    if ((*arr)[currentCmd][currentWord] == NULL) { perror("Couldn't reallocate word"); return -1; }
                }

                // check if current char is a newline char 
                if (chr == '\n') {

                    (*arr)[currentCmd][currentWord][wordLen] = '\0'; // Null-terminate
                    currentWord++;
                    // maybe realloc the size of the arr if we run out of index's to use
                    (*arr)[currentCmd][currentWord] = NULL;    // put NULL as the last index in the 2d array
                    wordAmt = currentWord+1;
                    *cmdCount = currentCmd+1;
                    return 1; // return with success
                } 

                // check if current char is a space
                else if (isspace(chr)) {
                    printf("isspace[currCmd:%i][currWord:%i][wordLen:%i][Char:\'%c\']\n", currentCmd, currentWord,wordLen,chr);

                    // if there is something already in this word
                    if (wordLen > 0) {
                        // space comes after a letter
                        printf("End current word\n");
                        (*arr)[currentCmd][currentWord][wordLen] = '\0'; // Null-terminate
                        currentWord ++;
                        wordLen = 0;  // current len of word
                        currentWordSize = 8;
                    } else {
                        // there isnt anything in the word yet
                        // skip the char
                        printf("Skipping char\n");
                    }
                }

                // check if current char is a pipeline
                else if((chr == '|')) {

                     // If part of a word, end the current word.
                    if (wordLen > 0) {
                        printf("End current word\n");
                        (*arr)[currentCmd][currentWord][wordLen] = '\0';
                        wordLen = 0;  // current len of word
                        currentWordSize = 8;
                    } else {
                        printf("pipe is own word\n");
                        // check if its the first word in the command
                        if (currentWord == 0){
                            // printf("bash: syntax error\n");
                            (*arr)[0][0] = "|";
                            return -2;
                        }
                        (*arr)[currentCmd][currentWord] = NULL; // delete start of prev word, only god knows where it is
                    } // echo hello | echo hi

                    printf("Current word:%i\n",currentWord);
                    
                    currentCmd++;
                    // ./mych text.exe | helper.exe -> text.exe,|,....
                    // "| helper.exe"

                    (*arr)[currentCmd][0] = malloc(2 * sizeof(char)); // Space for '|' and '\0'
                    (*arr)[currentCmd][0][0] = '|';
                    (*arr)[currentCmd][0][1] = '\0';
                    
                    // Prepare for the next command.
                    currentCmd++;
                    currentWord = 0;
                    wordLen = 0; 
                    currentWordSize = 8;
                }else{
                    // not a |, space, or new line

                    (*arr)[currentCmd][currentWord][wordLen] = chr; // Append character
                    (*arr)[currentCmd][currentWord][wordLen + 1] = '\0'; // Null-terminate
                    wordLen++;
                    //printf("wordLen:%i\n",wordLen);

                }
            }
        } else {
            // no bytes to read
            //printf("no bytes to read\n");
            // hit enter before data was read
            return 0;
        }
    }

    return 0; // didn't return or crash, it failed
}
