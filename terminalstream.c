// for reading in from a terminal, given or current
#include "terminalstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
int readWordsIntoArray(char ***arr, int *wordCount);
/**
 * arr, pointer to 3d char array, cmd index, word index, char index
 * cmdIndex, pointer to a int that will be set to the amount of cmds in the array (incuding pipes, if, then, else)
*/
int terminalStream(char ****arr, int *cmdIndex,char ***wordArr, int *wordCount, int **allocatedWords) {
    
    int retError=0;

    // if not success, return error
    if ((retError = readWordsIntoArray(wordArr, wordCount)) != 1) {
        //printf("readWordsIntoArray error: %i\n",retError);
        return 0;
    }
    
    
    
    int alloctedCmdAmt = 3; // default size for amount of commands in the array//+2;5+4;9+8 //cmdAmt+=cmdAmt-1

    const int defaultCmdLen = 5; // default size for the amount of words in a command line
    int amtAllocatedWordsInCmd = defaultCmdLen; // current max size of command            
    (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command

    int wordIndex = 0; // curent index of the word in the current command

    // allocate cmdArray
    *arr = malloc(alloctedCmdAmt * sizeof(char*));
    if (*arr == NULL) {perror("Couldn't allocate cmd array"); return -1;}

    // allocate each cmd in the array
    for (int i = 0; i < alloctedCmdAmt; i++) {
        (*arr)[i] = malloc(defaultCmdLen * sizeof(char*));
        if ((*arr)[i] == NULL) {perror("Couldn't allocate word array"); return -1;}
        
        // allocate each word in the array
        for (int j = 0; j < defaultCmdLen; j++) {
            (*arr)[i][j] = NULL; // Initialize each cmd array to NULL
        }
    }

    // itterate through each word in the array
    for (int indexWordArr = 0; indexWordArr<*wordCount; indexWordArr++) {
        printf("word: \"%s\",\n",(*wordArr)[indexWordArr]);

        // if pipe
        if (strcmp((*wordArr)[indexWordArr],"|") == 0 ||
            strcmp((*wordArr)[indexWordArr],"if") == 0 ||
            strcmp((*wordArr)[indexWordArr],"then") == 0 ||
            strcmp((*wordArr)[indexWordArr],"else") == 0
            )
        // end curr command and start a new one
        {
            // Check if need to expand array
            // allocated cmd amt - current Cmd Amt <= 2 see if there is less than or equal to 2 more cmds i can fit 
            if (alloctedCmdAmt - (*cmdIndex + 1) <= 2) {
                alloctedCmdAmt *= 2; // Double the size of the array for commands.
                char ***tempArr = realloc((*arr), alloctedCmdAmt * sizeof(char**));
                if (tempArr == NULL) {
                    perror("Couldn't reallocate array of cmds");
                    // Handle error, e.g., by freeing previously allocated memory.
                    return -1;
                }
                *arr = tempArr;

                // Initialize new command arrays.
                for (int i = *cmdIndex + 1; i < alloctedCmdAmt; i++) {
                    (*arr)[i] = malloc(defaultCmdLen * sizeof(char*));
                    if ((*arr)[i] == NULL) {
                        perror("Couldn't allocate word array for new cmd");
                        // Handle error.
                        return -1;
                    }
                    for (int j = 0; j < defaultCmdLen; j++) {
                        (*arr)[i][j] = NULL; // Initialize each word pointer to NULL.
                    }
                }
            }

            (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command
            printf("allocatedWords:%i\n",(*allocatedWords)[*cmdIndex]);
            // end the current cmd
            (*cmdIndex)++;
            wordIndex=0;
            (*arr)[*cmdIndex][wordIndex] = (*wordArr)[indexWordArr]; // copy the word from wordArr into arr
            (*allocatedWords)[*cmdIndex] = 1; // this is so we know how many words are allocated in this command
            printf("allocatedWords:%i\n",(*allocatedWords)[*cmdIndex]);

            // end the special cmd, since its special (| if else then)
            (*cmdIndex)++;
            amtAllocatedWordsInCmd = defaultCmdLen;
            (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command

            wordIndex=0;


        } else 
        // normal word
        {
            // Check if need to expand array
            if (wordIndex >= amtAllocatedWordsInCmd - 1) {
                amtAllocatedWordsInCmd *= 2; // Double the size for words within a command.
                (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command

                char **tempWordArr = realloc((*arr)[*cmdIndex], amtAllocatedWordsInCmd * sizeof(char*));
                if (tempWordArr == NULL) {
                    perror("Couldn't reallocate word array within command");
                    // Handle error.
                    return -1;
                }
                (*arr)[*cmdIndex] = tempWordArr;

                // Initialize new word pointers to NULL.
                for (int i = wordIndex + 1; i < amtAllocatedWordsInCmd; i++) {
                    (*arr)[*cmdIndex][i] = NULL;
                }
                
            }
            //copy word over
            (*arr)[*cmdIndex][wordIndex] = (*wordArr)[indexWordArr];
            wordIndex++;
        }

    }
    //printf("]");

    (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command
    printf("allocatedWords:%i\n",(*allocatedWords)[*cmdIndex]);

    (*cmdIndex)++; // end the last command

    
//    free the unused cmds
    // printf("free unused cmds\n");

    // if (arr == NULL) {return -1;}
    // if (*arr == NULL) {return -1;}
    // for (int i = *cmdIndex; i < alloctedCmdAmt; i++) {

    //     if ((*arr)[i] == NULL) {continue;}
    //     for (int j = 0; j < defaultCmdLen; j++) {

    //         if ((*arr)[i][j] == NULL) {continue;}
    //         free((*arr)[i][j]);
    //     }
    //     free((*arr)[i]);
    // }

    // // causes issue when freeing here!
    // for (int i = 0; wordArr[i]!=NULL; i++) {

    //     wordArr[i]=NULL;
    // }
    // free(wordArr);

    // printf("freed\n");


    return 1; // it succeded
}


/**
 * take in a 2d array of chars
 * an array of strings
*/
int readWordsIntoArray(char ***arr, int *wordCount){

    int allocatedWordAmt = 5; // amt of words allocated for the current command line
    
    // allocate cmdArray
    
    *arr = malloc(allocatedWordAmt * sizeof(char*));
    if (*arr == NULL) {perror("Couldn't allocate word array"); return -1;}

    // allocate each word in the array
    for (int i = 0; i < allocatedWordAmt; i++) {
        (*arr)[i] = NULL; // Initialize each cmd array to NULL
    }

    // allocate buffer
    int bufSize = 128;
    char *buf = malloc(bufSize);
    if (buf == NULL) {perror("Couldn't allocate buffer"); return -1;}

    int wordLen = 0; // num of chars in current word 
    int currentWord = 0; // current word in the cmd array
    int currentWordSize = 8; // 8 chars max size

    while (1) 
    {

        // only read bufSize - 1 bytes to save room for string terminator
        int numBytesRead = read(STDIN_FILENO, buf, bufSize - 1);
        DEBUG printf("read %i bytes\n",numBytesRead);
        // has bytes to read
        if (numBytesRead > 1) 
        {

            // iterate through all the bytes, will not run out of memory I think
            for (int currentPos = 0; currentPos < numBytesRead; currentPos++) 
            {
                
                // Check if need to expand array
                if (currentWord >= allocatedWordAmt - 1) {
                    allocatedWordAmt *= 2; // double the size of the array
                    char **tempArr = realloc((*arr),allocatedWordAmt * sizeof(char*));
                    if (tempArr == NULL) { perror("Couldn't reallocate array of words"); free(buf); return -1; }
                    *arr = tempArr;
                    // Initialize new pointers to NULL
                    for (int i = currentWord + 1; i < allocatedWordAmt; i++) {
                        ((*arr))[i] = NULL;
                    }
                }

                char chr = buf[currentPos]; // get the current char we are checking
                printf("[currWord:%i][wordLen:%i][Char:\'%c\']\n", currentWord,wordLen,chr);
                
                // ensure current word is allocated
                if ((*arr)[currentWord] == NULL) {
                    (*arr)[currentWord] = malloc(sizeof(char) * currentWordSize);
                    if ((*arr)[currentWord] == NULL) { perror("Couldn't allocate word"); free(buf); return -1; }
                }
                // save room for string terminator 
                else if (wordLen + 1 >= currentWordSize) {
                    currentWordSize+=currentWordSize;
                    (*arr)[currentWord] = realloc((*arr)[currentWord], currentWordSize);
                    if ((*arr)[currentWord] == NULL) { perror("Couldn't reallocate word"); free(buf); return -1; }
                }

                // check if current char is a newline char 
                if (chr == '\n' && (wordLen > 0 || currentWord > 0)) {
                    printf("**returning readWordsIntoArray\n");
                    (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                    
                    // maybe realloc the size of the arr if we run out of index's to use
                    //(*arr)[currentWord] = NULL;    // put NULL as the last index in the 2d array
                    *wordCount = currentWord+1;

                    printf("wordCount:%i\n",*wordCount);

                    free(buf);

                    // free the end of the array, basically incase there is anything after the stuff we actually need, 
                    // such as random byte data
                    if ((*arr)==NULL || arr==NULL)
                    {
                        printf("arr error\n");
                        return -1;
                    }
                    // free all excess words if needed
                    for (int freeI = *wordCount; freeI<allocatedWordAmt; freeI++)
                    {
                        if ((*arr)[freeI] == NULL) {continue;}
                        free((*arr)[freeI]);
                    }

                    printf("freed end of wordArr\n");
                    return 1; // return with success
                } 

                // check if current char is a space
                else if (isspace(chr)) {
                    //printf("isspace[currWord:%i][wordLen:%i][Char:\'%c\']\n", currentWord,wordLen,chr);

                    // if there is something already in this word
                    if (wordLen > 0) {
                        // space comes after a letter
                        printf("End current word\n");
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                        currentWord ++;
                        wordLen = 0;  // current len of word
                        currentWordSize = 8; // set to default size again
                    } else {
                        // there isnt anything in the word yet
                        // skip the char
                        printf("Skipping char\n");
                    }
                }

                // current char is a pipe
                else if (chr == '|') {
                    // end current word
                    if (wordLen > 0) {
                        // pipe comes after a letter
                        printf("End current word\n");
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                        currentWord ++;
                    }

                    printf("put pipe in array\n");
                    (*arr)[currentWord]= malloc(2 * sizeof(char)); // Space for '|' and '\0'
                    (*arr)[currentWord][0] = '|';
                    (*arr)[currentWord][1] = '\0';

                    // pipe word is done, reset vals and increase the word count
                    currentWord ++;
                    wordLen = 0;  // current len of word
                    currentWordSize = 8; // set to default size again

                }
                else{
                    // not space, pipe, or new line

                    (*arr)[currentWord][wordLen] = chr; // Append character
                    (*arr)[currentWord][wordLen + 1] = '\0'; // Null-terminate
                    wordLen++;
                    //printf("wordLen:%i\n",wordLen);
                    // echo hi | echo hello
                }
            }
        } else {
            // no bytes to read
            //printf("no bytes to read\n");
            // hit enter before data was read
            free(buf);
            return 0;
        }
    }

    // reached end somehow
    printf("Reached end termStream error\n");
    return 0;
}

