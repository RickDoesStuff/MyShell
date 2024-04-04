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

        // cmd before the pipe
        if (cmd->pipeIn == 1 && cmd->pipeOut == 0) {
            dup2(currPipe[1], STDOUT_FILENO); // printf now writes to the write end of the pipe
            //printf("pipeIn == 1 : currPipe[0]:%i, currPipe[1]:%i\n", currPipe[0], currPipe[1]);

            // not needed
            close(currPipe[0]); // Close the read-end, not needed here
            close(currPipe[1]);
            close(prevPipe[0]);
            close(prevPipe[1]);
        }
        // cmd after the pipe
        if (cmd->pipeIn == 0 && cmd->pipeOut == 1) {

            // redirect stdin to the read end of the pipe
            dup2(currPipe[0], STDIN_FILENO);

            // No longer needed after dup2
            close(currPipe[0]); 
            close(currPipe[1]);
            close(prevPipe[0]);
            close(prevPipe[1]);
        }
        // cmd between pipes
        if (cmd->pipeIn == 1 && cmd->pipeOut == 1) {
            dup2(prevPipe[0], STDIN_FILENO); // Redirect stdin from prevPipe read end
            dup2(currPipe[1], STDOUT_FILENO); // Redirect stdout to currPipe write end

            // not needed
            close(currPipe[1]); // Close unused write end
            close(currPipe[0]); // Close after dup2
            close(prevPipe[0]); // Close unused read end
            close(prevPipe[1]); // Close after dup2
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

            if (execv(cmdDir, cmd->words) != -1) {
                // success
                free(cmdDir);
                exit(EXIT_SUCCESS);
            }

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


    // status is set to 1 on failure and 0 on success
    if (waitpid(childpid, &status, 0) == -1) {
        perror("Error waiting for child");
        return -1;
    }


    if ( WIFEXITED(status) )
    {
        int exit_status = WEXITSTATUS(status);        
        printf("Exit status of the child was %d\n", 
                                     exit_status);
                                     
        cmd->exitStatus=exit_status;
    } else
    if (!WIFEXITED(status) && WEXITSTATUS(status) != 0){
        perror("error with child process:");
        return -1;
    }

     

    return 1;
}