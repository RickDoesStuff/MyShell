#include "executeCmd.h"


/**
 * Expand wildcards in a command struct
*/
int wildcardExpansion(command *cmd) {
    for (int i = 0; i < cmd->length; i++) {
        if (strpbrk(cmd->words[i], "*") != NULL) { // Check for wildcard characters
            glob_t glob_result;

            // init and alloc the glob_result to 0s
            memset(&glob_result, 0, sizeof(glob_result));
            
            if (glob(cmd->words[i], GLOB_NOCHECK, NULL, &glob_result) == 0) {

                int newLength = cmd->length + glob_result.gl_pathc - 1; // Adjust for replacement
                
                if (newLength > cmd->size) {
                    char **newWords = realloc(cmd->words, newLength * sizeof(char *));
                    if (newWords == NULL) {
                        perror("Failed to realloc cmd->words");
                        exit(EXIT_FAILURE);
                    }
                    cmd->words = newWords;
                    cmd->size = newLength;
                }
                
                // Free the original wildcard string to prevent a memory leak
                free(cmd->words[i]);
                
                // Shift existing elements if necessary, starts at the end of the word list and moves back up to the wildcard currently being expanded
                for (int j = cmd->length - 1; j > i; j--) {
                    cmd->words[j + glob_result.gl_pathc - 1] = cmd->words[j];
                }
                
                // Insert new paths
                for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                    cmd->words[i + j] = strdup(glob_result.gl_pathv[j]);
                }
                
                // Update the loop counter and the command length
                i += glob_result.gl_pathc - 1;
                cmd->length = newLength;
            }
            
            globfree(&glob_result);
        }
    }
    return 1;
}

/**
 * takes a pointer to a command struct
 * checks the command against our built in functions then tries to execute using execv
 * return -1 error
 * return 0 wants to exit
 * return 1 success, valid, invalid or no command given but functioned as expected
*/
int check_command(command *cmd) {

    if (cmd->words == NULL || cmd->words[0] == NULL){
        // word is null, error
        return -1;
    }
       
    // where I need to handle the * wildcard
    wildcardExpansion(cmd);

    // Exit command
    if (strcmp(cmd->words[0],"exit") == 0 && cmd->length == 1) {
        // Exiting the program!
        return 0;
    }

    // try to execute it now with built in commands and execv commands
    return execute_command(cmd);
}


int currPipe[2]={-1,-1};
int prevPipe[2]={-1,-1};

/**
 * take a pointer to a command struct
 * execute the given command
 * 
 * **only to be used by check_command();**
*/
int execute_command(command *cmd) {
    char *bins[] = {"/usr/local/bin/","/usr/bin/","/bin/",""};// searches bins before searching working dir
    int binCount = 4;
    int childpid;
    int status;

    //printf("\npipeIn:%i pipeOut:%i\n", cmd->pipeIn, cmd->pipeOut);

    // if the command is the start of a pipe
    if (cmd->pipeIn == 1) {
        prevPipe[0] = currPipe[0];
        prevPipe[1] = currPipe[1];
        if (pipe(currPipe) == -1) {printf("pipe error\n"); exit(-1);}
        //printf("pipeIn new : currPipe[0]:%i, currPipe[1]:%i\n", currPipe[0], currPipe[1]);
        //printf("pipeIn old : currPipe[0]:%i, currPipe[1]:%i\n", prevPipe[0], prevPipe[1]);
    }

    if ((childpid = fork()) == -1) {
        // fork failed
        perror("Can't fork:");
        exit(EXIT_FAILURE);
        
    } else if (childpid == 0) {
        // only in child

        // input redirection
        if (cmd->redirectIn != -1) {
            dup2(cmd->redirectIn, STDIN_FILENO); // redirect stdin from cmd->redirectIn
            close(cmd->redirectIn); // close when done with it
        }
        // output redirection
        if (cmd->redirectOut != -1) {
            dup2(cmd->redirectOut, STDOUT_FILENO); // redirect stdin from cmd->redirectIn
            close(cmd->redirectOut); // close when done with it
        }

        // cmd before the pipe HERE | ...          also makes sure there is no specified redirectOut
        if (cmd->pipeIn == 1 && cmd->pipeOut == 0 && cmd->redirectOut == -1) {
            dup2(currPipe[1], STDOUT_FILENO); // printf now writes to the write end of the pipe
            //printf("pipeIn == 1 : currPipe[0]:%i, currPipe[1]:%i\n", currPipe[0], currPipe[1]);

            // not needed
            close(currPipe[0]); // Close the read-end, not needed here
            close(currPipe[1]);
            close(prevPipe[0]);
            close(prevPipe[1]);
        } else
        // cmd after the pipe ... | HERE          also makes sure there is no specified redirectIn
        if (cmd->pipeIn == 0 && cmd->pipeOut == 1 && cmd->redirectIn == -1) {

            // redirect stdin to the read end of the pipe
            dup2(currPipe[0], STDIN_FILENO);

            // No longer needed after dup2
            close(currPipe[0]); 
            close(currPipe[1]);
            close(prevPipe[0]);
            close(prevPipe[1]);
        } else
        // cmd between pipes ... | HERE | ...           also makes sure there is no specified redirection
        if (cmd->pipeIn == 1 && cmd->pipeOut == 1) {
            // check if there it input redirection
            if (cmd->redirectIn == -1) {
                dup2(prevPipe[0], STDIN_FILENO); // Redirect stdin from prevPipe read end
            }
            // check if there it output redirection
            if (cmd->redirectOut == -1) {
                dup2(currPipe[1], STDOUT_FILENO); // Redirect stdout to currPipe write end
            }
            // not needed
            close(currPipe[1]); // Close unused write end
            close(currPipe[0]); // Close after dup2
            close(prevPipe[0]); // Close unused read end
            close(prevPipe[1]); // Close after dup2
        }

         
        // PWD command
        if (strcmp(cmd->words[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        // CD command
        else if (strcmp(cmd->words[0], "cd") == 0) {
            if (cmd->words[1] == NULL) {
                printf("No directory given\n");
                exit(EXIT_SUCCESS);
            } else {
                if (chdir(cmd->words[1]) != 0) {
                    perror("cd failed");
                    exit(EXIT_FAILURE);
                }
            }
            // successfully changed directories
            exit(EXIT_SUCCESS);
        }
        // which command
        else if (strcmp(cmd->words[0], "which") == 0) {
            if (cmd->length != 2) {
                // wrong amt of words | only accepts "which path"
                printf("wrong amt of args\n");
                exit(EXIT_SUCCESS);
            } else {
                // search through each given bin in *bins[]
                for (int binIndex = 0; binIndex < binCount-1; binIndex++)
                {
                    int cmdSize = strlen(cmd->words[1]) + strlen(bins[binIndex]) + 1;
                    char *cmdDir = malloc((char)cmdSize);
                    if (cmdDir == NULL) {perror("cmd dir wasn't allocated"); exit(EXIT_FAILURE);}

                    // add the current commands first word to the end of the path to search
                    // basically cmdDir = bins[i] + cmdWords[0]; => cmdDir = "/usr/bin/" + "echo"; => cmdDir = "/usr/bin/echo";
                    snprintf(cmdDir,cmdSize,"%s%s",bins[binIndex],cmd->words[1]);

                    // check if the file exists in a directory
                    if (access(cmdDir, F_OK | X_OK) == 0) { 
                        // success
                        printf("%s\n",cmdDir);
                        free(cmdDir);
                        exit(EXIT_SUCCESS);
                    }

                    free(cmdDir);
                }
                // command wasn't found, but no error
                printf("Command \'%s\' not found.\n", cmd->words[0]);
                exit(EXIT_FAILURE);
            }
            // successfully changed directories
            exit(EXIT_SUCCESS);
        }

        // search through each given bin in *bins[]
        for (int binIndex = 0; binIndex < binCount; binIndex++)
        {
            int cmdSize = strlen(cmd->words[0]) + strlen(bins[binIndex]) + 1;
            char *cmdDir = malloc((char)cmdSize);
            if (cmdDir == NULL) {perror("cmd dir wasn't allocated"); exit(EXIT_FAILURE);}

            // add the current commands first word to the end of the path to search
            // basically cmdDir = bins[i] + cmdWords[0]; => cmdDir = "/usr/bin/" + "echo"; => cmdDir = "/usr/bin/echo";
            snprintf(cmdDir,cmdSize,"%s%s",bins[binIndex],cmd->words[0]);

            //printf("\nSearching:%s\n",cmdDir);
            if (access(cmdDir, F_OK | X_OK) == 0) { 
                // command was found and is executable
                if (execv(cmdDir, cmd->words) != -1) {
                    // success
                    free(cmdDir);
                    exit(EXIT_SUCCESS);
                }
            } 
            // command wasnt found

            free(cmdDir);
        }
        // command wasn't found, but no error
        printf("Command \'%s\' not found.\n", cmd->words[0]);
        exit(EXIT_FAILURE);
    }
    // only in parent

    // before a pipe
    if (cmd->pipeIn == 1 && cmd->pipeOut == 0) { 
        close(currPipe[1]);
        //printf("pipeIn : close currPipe[1]:%i\n", currPipe[1]);
    } else
    // after a pipe
    if (cmd->pipeIn == 0 && cmd->pipeOut == 1) { 
        close(currPipe[0]);
        close(currPipe[1]);
        close(prevPipe[0]);
        close(prevPipe[1]);
        //printf("pipeOut : close all currPipe{%i,%i}, prevPipe{%i,%i}\n", currPipe[0],currPipe[1],prevPipe[0],prevPipe[1]);
    } else
    // middle of a pipe
    if (cmd->pipeIn == 1 && cmd->pipeOut == 1) {
        close(prevPipe[0]);
        close(currPipe[1]);        
        //printf("pipeOut && pipeIn : close prevPipe[0]:%i, currPipe[1]:%i\n", prevPipe[0], currPipe[1]);
    }

    // input redirection
    if (cmd->redirectIn != -1) {
        close(cmd->redirectIn); // close when done with it
        cmd->redirectIn = -1; // reset the value
    }
    // output redirection
    if (cmd->redirectOut != -1) {
        close(cmd->redirectOut); // close when done with it
        cmd->redirectOut = -1; // reset the value
    }

    // status is set to 1 on failure and 0 on success
    if (waitpid(childpid, &status, 0) == -1) {
        perror("Error waiting for child");
        return -1;
    }


    if ( WIFEXITED(status) )
    {
        int exit_status = WEXITSTATUS(status);        
        printf("Exit status of the child was %d\n", exit_status);
                                     
        cmd->exitStatus=exit_status;
    } else
    if (!WIFEXITED(status) && WEXITSTATUS(status) != 0){
        perror("error with child process:");
        return -1;
    }

     

    return 1;
}