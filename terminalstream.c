// for reading in from a terminal, given or current
#include "terminalstream.h"



// free the words in the current command
int freeWords(command *cmd) {
    
    // free all the current words
    printf("freeing: ");
    for (int i = 0; i < cmd->size; i++) {
        printf("\'%s\' ", cmd->words[i]);

        if (cmd->words[i] == NULL) {continue;}
        // only free if not null
        free(cmd->words[i]);
        cmd->words[i] = NULL;
    }
    //printf("\n");
    if (cmd->words != NULL){
        free(cmd->words);
    }
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
    cmd->pipeIn = 0;
    cmd->pipeOut = 0;
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

/**
 * run the command in our shell
 * frees the words on exit, error
 * resets the command on success
 * return 1 on success
 * return 0 on user request to exit
 * return -1 on error
 */
int runCommand(command *cmd) {

    // get the return code from check_command
    // checks the command against our built in functions then tries to execute using execv
    int execCmdRetCode = check_command(cmd);

    // check if user executed exit command
    if (execCmdRetCode == 0) {
        // user wants to exit
        freeWords(cmd);
        return 0;
    }
    // check if exited with error
    else if (execCmdRetCode == -1) {
        // error, exit the program
        printf("error in executeCmd.c\n");
        freeWords(cmd);
        return -1;
    }
    // execute_command functioned as expected, get next user input
    // doesn't free or reset here, do that in other functions!
    return 1;
}



/**
 * arr, pointer to a string array, will be written to, all words typed into a terminal for one command
 * wordCount, pointer to an int, will be written to with the # of words in the arr
 * cmdIndex, pointer to a int that will be set to the amount of cmds in the array (incuding pipes, if, then, else)
 * return 1 on success
 * return 0 on user request to exit
 * return -1 on error
*/
int terminalStream(char ***wordArr, int *wordCount) {
    
    int retError=readWordsIntoArray(wordArr, wordCount);

    // check if we could read words into the array
    if (retError == -1) {
        // error
        //printf("readWordsIntoArray error: %i\n",retError);
        return -1;
    } else if (retError == 0) {
        // typed nothing and just hit enter
        return 1;
    }
    
    //printf("checking word arr\n");
    //for (int i = 0; i < *wordCount; i++){
    //    printf("word: %s\n", (*wordArr)[i]);
    //}
    //printf("end check for word arr\n");
    
    
    
    // setup the command struct and initialize it
    command cmd;
    initCommand(&cmd);
    

    // itterate through each word in the array
    for (int index = 0; index<*wordCount; index++) {
        //printf("checking word: \"%s\",\n",(*wordArr)[index]);

        // if pipe
        if (strcmp((*wordArr)[index],"|") == 0 ||
            strcmp((*wordArr)[index],"if") == 0 ||
            strcmp((*wordArr)[index],"then") == 0 ||
            strcmp((*wordArr)[index],"else") == 0
            )
        // end curr command execute it, and wipe the cmd to start over
        {
            // check if current command is a pipe
            if (strcmp((*wordArr)[index],"|") == 0) {
                cmd.type=1; // start of a pipe / passing into a pipe
                cmd.pipeIn = 1;
            }

            // runs the command
            int runCmdRetCode = runCommand(&cmd);
            // exiting either user request(0) or on error (-1)
            if(runCmdRetCode != 1) {
                return runCmdRetCode;
            }
            // run cmd
            resetCommand(&cmd);
            
            // setup next command of the pipe
            if (strcmp((*wordArr)[index],"|") == 0) {
                cmd.pipeIn = 0; // next command is unknown if passing into a pipe so we reset
                cmd.pipeOut = 1; // next command is taking from a pipe
            }

            // free the special word
            printf("'%s'\n",(*wordArr)[index]);
            free((*wordArr)[index]);

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
            cmd.words[cmd.length] = (*wordArr)[index];
            cmd.length++;
        }

    }
    

    // runs the command
    int runCmdRetCode = runCommand(&cmd);
    // exiting either user request(0) or on error (-1)
    if(runCmdRetCode != 1) {
        return runCmdRetCode;
    }
    // free the words
    freeWords(&cmd);
    cmd.type=0;
    printf("***line ended***\n");

    return 1; // it succeeded!
}


/**
 * take in an array of strings and the amount of words in that array
 * and writes to them
 * return 1 on success
 * return 0 on user didnt type anything and just hit enter
 * return -1 on error
*/
int readWordsIntoArray(char ***arr, int *wordCount) {

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
                
                // Check if need to expand array (its -2 bc -1 causes an error here: echo hi| ls|echo hey)
                if (currentWord >= allocatedWordAmt - 2) {
                    //printf("realloc\n");
                    allocatedWordAmt *= 2; // double the size of the array
                    *arr = realloc((*arr), allocatedWordAmt * sizeof(char*));
                    if (*arr == NULL) { perror("Couldn't reallocate array of words"); free(buf); return -1; }
                    //*arr = tempArr;
                    // Initialize new pointers to NULL
                    for (int i = currentWord + 1; i < allocatedWordAmt; i++) {
                        ((*arr))[i] = NULL;
                    }
                }

                //printf("expansion done if needed\n");

                char chr = buf[currentPos]; // get the current char we are checking
                //printf("[currWord:%i][wordLen:%i][Char:\'%c\']\n", currentWord,wordLen,chr);
                //printf("curr char:\'%c\'\n",chr);
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

                
                //printf("expanded for terminator\n");

                // check if current char is a newline char 
                if (chr == '\n' && (wordLen > 0 || currentWord > 0)) {
                    //printf("**returning readWordsIntoArray\n");
                    (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                    
                    // maybe realloc the size of the arr if we run out of index's to use
                    //(*arr)[currentWord] = NULL;    // put NULL as the last index in the 2d array
                    *wordCount = currentWord+1;

                    //printf("wordCount:%i\n",*wordCount);

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
                        (*arr)[freeI] = NULL;
                    }

                    //printf("freed end of wordArr\n");
                    return 1; // return with success
                } 

                // check if current char is a space
                else if (isspace(chr)) {
                    //printf("isspace[currWord:%i][wordLen:%i][Char:\'%c\']\n", currentWord,wordLen,chr);

                    // if there is something already in this word
                    if (wordLen > 0) {
                        // space comes after a letter
                        //printf("End current word\n");
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                        currentWord ++;
                        wordLen = 0;  // current len of word
                        currentWordSize = 8; // set to default size again
                    } else {
                        // there isnt anything in the word yet
                        // skip the char
                        // printf("Skipping char\n");
                    }
                }

                // current char is a pipe
                else if (chr == '|') {
                    // end current word
                    if (wordLen > 0) { // echo hi |
                        // pipe comes after a letter
                        // printf("End current word\n");
                        (*arr)[currentWord][wordLen] = '\0'; // Null-terminate
                        printf("curr word:%s\n",(*arr)[currentWord]);
                        currentWord ++;
                    }

                    // printf("put pipe in array\n");

                    // Space for '|' and '\0'
                    if ((*arr)[currentWord] == NULL) {
                        (*arr)[currentWord] = malloc(sizeof(char) * 2);
                        if ((*arr)[currentWord] == NULL) { perror("Couldn't allocate word"); free(buf); return -1; }
                    }
                    (*arr)[currentWord][0] = chr;
                    (*arr)[currentWord][1] = '\0';

                    // pipe word is done, reset vals and increase the word count
                    currentWord ++;
                    wordLen = 0;  // current len of word
                    currentWordSize = 8; // set to default size again
                    //printf("reset\n");
                }
                else{
                    //printf("not space, pipe or new line\n");
                    // not space, pipe, or new line
                    //printf("(*arr)[%i]",wordLen);
                    //printf("[%i]:",wordLen);
                    //printf("\'%c\'\n",chr);
                    //printf("(*arr) is %p\n",*arr);
                    //printf("(*arr)[currentWord] is %p\n",(*arr)[currentWord]);
                    //if ((*arr)[currentWord][wordLen] == NULL) {printf("(*arr)[currentWord][wordLen] null");}
                    (*arr)[currentWord][wordLen] = chr; // Append character
                    //printf("\'%c\'\n",(*arr)[currentWord][wordLen]);
                    (*arr)[currentWord][wordLen + 1] = '\0'; // Null-terminate
                    wordLen++;
                    // echo hi | echo hello
                }
            }
        } else {
            // no bytes to read
            // hit enter before data was read
            free(buf);
            return 0;
        }
    }

    // reached end somehow
    printf("Reached end terminalStream error\n");
    return -1;
}