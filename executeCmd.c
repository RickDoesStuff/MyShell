#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#include "executeCmd.h"


/**
 * take a pointer to a command struct
*/
int execute_command(command *cmd){
    char *bins[] = {"/usr/local/bin/","/usr/bin/","/bin/",""};// searches bins before searching working dir
    int binCount = 4;
    int childpid;
    int status;
    
    if ((childpid = fork()) == -1) {
        // fork failed
        perror("Can't fork:");
        exit(EXIT_FAILURE);
        
    } else if (childpid == 0) {
        // only in child

        // search through each given bin in *bins[]
        for (int binIndex = 0; binIndex < binCount; binIndex++)
        {
            int cmdSize = strlen(cmd->words[0]) + strlen(bins[binIndex]) + 1;
            char *cmdDir = malloc((char)cmdSize);
            if (cmdDir == NULL) {perror("Buf wasn't allocated"); exit(EXIT_FAILURE);}

            snprintf(cmdDir,cmdSize,"%s%s",bins[binIndex],cmd->words[0]);// add the current commands first word to the end of the path to search

            printf("\nSearching:%s\n",cmdDir);

            if (execv(cmdDir, cmd->words) != -1) {
                free(cmdDir);
                
                // free the stuff in the child process??
                exit(EXIT_SUCCESS);
            }

            free(cmdDir);
        }
        

        // free the stuff in the child process??
        perror("execv error:");
        exit(EXIT_FAILURE);
    }
    // only in parrent
    if (wait(&status) == -1){
        perror("Error waiting for child:");
        exit(EXIT_FAILURE);
    }
    //
    if(!WIFEXITED(status) && WEXITSTATUS(status) != 0){
        printf("error with child process:");
        perror("::");
        exit(EXIT_FAILURE);
    }


    return 1;
}