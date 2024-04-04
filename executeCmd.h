#ifndef _EXECUTECMD_H
#define _EXECUTECMD_H

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>


typedef struct command{
    int type; // type of command, 0 - normal, 1 - pipe, 2 - then, 3 - else, WIP (THEN ELSE < >)
    int pipeIn; // 0 - no, 1 - yes, it pipes into a command (its on the left side of a pipe (HERE | ...)) 
    int pipeOut; // 0 - no, 1 - yes, it pipes out of a command (its on the right side of a pipe (... | HERE))
    int redirectIn; // -1 if no redirection; > 0 the FD to redirect to
    int redirectOut; // -1 if no redirection; > 0 the FD to redirect to
    int size; // how many words allocated 
    int length; // how many words in command
    int exitStatus; // exit status of previous command
    char **words; // an array of words
} command;

/**
 * takes a pointer to a command struct
 * checks the command against our built in functions then tries to execute using execv
 * return -1 error
 * return 0 wants to exit
 * return 1 success, valid, invalid or no command given but functioned as expected
*/
int check_command(command *cmd);

/**
 * take a pointer to a command struct
 * execute the given command
 * 
 * **only to be used by check_command();**
*/
int execute_command(command *cmd);

#endif