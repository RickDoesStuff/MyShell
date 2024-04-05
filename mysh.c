#include "linestream.h"
#include "mysh.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int runningMode(int interactive);

int main(int argc, char **argv) {
    int interactive = 1; // set interactive mode true by default
    char *path = NULL;

    DEBUG printf("argc:%i\n",argc);

    if (argc >= 2) {
        DEBUG printf("arg found:%s\n",argv[1]);
        path = argv[1];

        // Open file given
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("Error opening file: %s\n", path);
            return 1; // Return with error code
        }

        // Redirect stdin to read from the file
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("Failed to redirect stdin");
            close(fd); // Close the file descriptor if redirection fails
            return 1; // Return with error code
        }

        // Set the application to run in batch mode
        printf("Running in batch mode\n");
        interactive = 0;

        // Since we've redirected stdin, there's no need to check if it's a tty
    } else {
        // No path given, check if stdin is a terminal or being piped
        if (isatty(STDIN_FILENO)) {
            printf("Is a terminal\n");
            interactive = 1;
        } else {
            printf("Not a terminal\n");
            interactive = 0;
        }
        DEBUG printf("Running in interactive mode, no path given\n");
    }

    // Run in interactive mode
    if (interactive) {
        runningMode(interactive);
    } else {
        // For batch mode, you might want to call a similar function here that handles batch operations
        // For simplicity, we'll just call interactiveMode with interactive set to 0
        runningMode(interactive);
    }

    // Cleanup if a file was opened
    if (argc >= 2) {
        close(STDIN_FILENO); // It's a good practice to close the duplicated file descriptor
    }

    return 0;
}

/**
 * Running the shell
 * int interactive, 1 - interactive, 2 - batch
 * return 0 on fail
 * return 1 on success
*/
int runningMode(int interactive) {

    //int pipeFD = open("./pipeFD", O_WRONLY);

    if (interactive == 1) {
        printf("\n\nWelcome to mysh\n\n");
    }

    int isRunning = 1;        

    while ((interactive && isRunning) || (!interactive && !feof(stdin))) 
    {

        // maybe move these inside of terminal Stream?? idea?
        int wordCount = 0;
        char **wordArr=NULL;   

        if (interactive == 1) {
            printf("\nmysh> ");
        }
        fflush(stdout); // flush the stdout that way the mysh> gets printed before it starts looking for out input

        

        // read in from the terminal
        
        // setup the command struct and initialize it
        command cmd;
        int linestreamReturnCode = linestream(&wordArr, &wordCount, &cmd, interactive);
        
        // check if error
        if(linestreamReturnCode == -1) {
            printf("Terminal Stream Error\n");

            free(wordArr);
            exit(EXIT_FAILURE);
        }
        // check if user wants to exit
        else if(linestreamReturnCode == 0) {
            //printf("calling free at exit;2\n");
            //freeArr(&wordArr, &wordCount);
            free(wordArr);
            isRunning = 0;
            continue;
        }
        // terminalStream was sucessfull
        free(wordArr);

        // batch file read, exiting
        if (!interactive)
        {
            printf("closing stdin\n");
            close(STDIN_FILENO); // It's a good practice to close the duplicated file descriptor
            isRunning = 0;
            break;
        }else{
            //printf("not eof\n");
        }

        continue;
    }

    if (interactive == 1){
        printf("\nmysh: exiting\n");
    }
    return 0;
}