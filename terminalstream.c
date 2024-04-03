// for reading in from a terminal, given or current
#include "terminalstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>



// free the words in the current command
int freeWords(command *cmd) {
    
    // free all the current words
    printf("\ncurrent cmd words: ");
    for (int i = 0; i < cmd->length; i++) {
        printf("%s ", cmd->words[i]);
        free(cmd->words[i]);
    }
    printf("\n");
    free(cmd->words);
    return 1;
}

// malloc the words
int mallocWords(command *cmd) { 
    cmd->words = malloc(sizeof(char*)*cmd->size);
    if (cmd->words == NULL) {perror("Couldn't allocate cmd"); return -1;}
    for (int i = 0; i < cmd->size; i++) {
        cmd->words[i] = NULL; // set each pointer to null
    }

    return 1;
}

// setup the command struct for the first time
int initCommand(command *cmd) {
    cmd->size = 5;
    cmd->length = 0;
    cmd->type = 0;
    mallocWords(cmd);
    return 1;
}

// reset the command struct and re-initialize
// doesnt reset the type variable
int resetCommand(command *cmd) {
    freeWords(cmd);
    cmd->size = 5;
    cmd->length = 0;
    mallocWords(cmd);
    return 1;
}

// run the command in our shell
int runCommand(command *cmd) {

    // get the return code from execute_command
    int execCmdRetCode=execute_command(cmd);

    // check if user executed exit command
    if (execCmdRetCode == 0) {
        freeWords(cmd);
        exit(EXIT_SUCCESS);
    }
    // check if exited with error
    else if (execCmdRetCode == -1) {
        // error, exit the program
        printf("error in execute_command.c\n");
        freeWords(cmd);
        exit(EXIT_FAILURE);
    }
    // execute_command functioned as expected, reset to get next user input
    // reset command and get next
    resetCommand(&cmd);
    return 1;
}


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
    
    printf("checking word arr\n");
    for (int i = 0; i < *wordCount; i++){
        printf("word: %s\n", (*wordArr)[i]);
    }
    printf("end check for word arr\n");
    
    
    
    // setup the command struct and initialize it
    command cmd;
    initCommand(&cmd);
    

    // itterate through each word in the array
    for (int index = 0; index<*wordCount; index++) {
        printf("checking word: \"%s\",\n",(*wordArr)[index]);

        // if pipe
        if (strcmp((*wordArr)[index],"|") == 0 ||
            strcmp((*wordArr)[index],"if") == 0 ||
            strcmp((*wordArr)[index],"then") == 0 ||
            strcmp((*wordArr)[index],"else") == 0
            )
        // end curr command execute it, and wipe the cmd to start over
        {
            // check if its a pipe
            if (strcmp((*wordArr)[index],"|") == 0) {
                cmd.type = 1;
            }

            // runs the command
            runCommand(&cmd);

            printf("pipe found\n");

        } else 
        // normal word
        {
            // Check if need to expand array
            if (cmd.length >= cmd.size - 1) {
                cmd.size *= 2; // Double the size for words within a command.
                
                cmd.words = realloc(cmd.words, cmd.size * sizeof(char*));
                if (cmd.words == NULL) {
                    perror("Couldn't reallocate word array within command");
                    // Handle error.
                    return -1;
                }
                
                // Initialize new word pointers to NULL.
                for (int i = cmd.length + 1; i < cmd.size; i++) {
                    cmd.words[i] = NULL;
                }
            }
            //copy word over
            cmd.length++;
            cmd.words[cmd.length-1] = (*wordArr)[index];
        }

    }

    
    // runs the command
    runCommand(&cmd);

    // free the malloc'd array
    freeWords(&cmd);

    // //printf("]");

    // (*allocatedWords)[*cmdIndex] = amtAllocatedWordsInCmd; // this is so we know how many words are allocated in this command
    // printf("allocatedWords:%i\n",(*allocatedWords)[*cmdIndex]);

    // (*cmdIndex)++; // end the last command

    
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

                    // Space for '|' and '\0'
                    if ((*arr)[currentWord] == NULL) {
                        (*arr)[currentWord] = malloc(sizeof(char) * 2);
                        if ((*arr)[currentWord] == NULL) { perror("Couldn't allocate word"); free(buf); return -1; }
                    }
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