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
    int type; // type of command, 0 - normal, 1 - pipe
    int size; // how many words allocated 
    int length; // how many words in command
    int indexStart; // what index in the wordArr does this command array start on
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