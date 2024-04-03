#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#include "executeCmd.h"



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

    // Exit command
    if (strcmp(cmd->words[0],"exit") == 0 && cmd->length == 1) {
        // Exiting the program!
        printf("**exit typed!**\n");
        return 0;
    }
    // PWD command
    else if (strcmp(cmd->words[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
            return -1;
        }
        return 1;
    }
    // CD command
    else if (strcmp(cmd->words[0], "cd") == 0) {
        if (cmd->words[1] == NULL) {
            printf("No directory given\n");
            return 1;
        } else {
            if (chdir(cmd->words[1]) != 0) {
                perror("cd failed");
                return -1;
            }
        }
        // successfully changed directories
        return 1;
    }
    // not a built in command, so try to execute it

    return execute_command(cmd);

    // if (execute_command(cmd) == -1) {
    //     // error happened
    //     return -1;
    // } else {

    // }
    // return 1;
}

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

            // add the current commands first word to the end of the path to search
            // basically cmdDir = bins[i] + cmdWords[0]; => cmdDir = "/usr/bin/" + "echo"; => cmdDir = "/usr/bin/echo";
            snprintf(cmdDir,cmdSize,"%s%s",bins[binIndex],cmd->words[0]);

            //printf("\nSearching:%s\n",cmdDir);

            if (execv(cmdDir, cmd->words) != -1) {
                // success
                free(cmdDir);
                exit(EXIT_SUCCESS);
            }

            free(cmdDir);
        }
        // command wasn't found, but no error
        printf("Command \'%s\' not found.\n", cmd->words[0]);
        return 1;
    }
    // only in parrent
    if (wait(&status) == -1){
        perror("Error waiting for child:");
        return -1;
    }
    //
    if(!WIFEXITED(status) && WEXITSTATUS(status) != 0){
        printf("error with child process:");
        perror("::");
        return -1;
    }


    return 1;
}