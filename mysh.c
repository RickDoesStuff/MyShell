#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int interactiveMode(char *path);

int main(int argc, char **argv) {
    int interactive = 1; // set interactive mode true by default
    char *path = NULL;

    printf("argc:%i\n",argc);

    // check if a 2nd arg (file path) is given
    if (argc >= 2) {
        
        printf("arg found:%s\n",argv[1]);
        path = (char *)(argv[1]);
        int fd = open(path,O_RDONLY);
        if (fd < 0) {printf("error opening file: %s\n",path);}

        // check if that file is a tty or a batch file
        if (isatty(fd) == 1) {
            // it is a tty
            printf("running in interactive mode, given terminal path\n");
            // here I may need to change the output from the current terminal to the terminal I was given I think
            // using 
            // dup2(fd, fd);

        } else {
            // is not a tty
            printf("running in batch mode\n");
            interactive = 0;
        }
    } else {
        // no path given, running in current terminal?
        printf("running in interactive mode, no path given\n");
    }

    // if its being run in interactive mode
    if (interactive) {
        interactiveMode(path);
    }

}

/**
 * print x amt of blank lines
*/
void blankLines(int count){
    for (int i=0;i<count;i++)
        printf("\n");
}
/**
 * Being run in interactive mode
 * return 0 on fail
 * return 1 on success
*/
int interactiveMode(char *path) {
    blankLines(4);
    printf("Welcome to mysh");
    blankLines(9);

    int isRunning = 1;
    char *userMessage;
    while (isRunning)
    {   
        // exit from loop
        printf("mysh> ");
        if(strcmp(userMessage, "exit") == 0) {isRunning = 0;}
    }




    printf("mysh: exiting");
    return 0;
}