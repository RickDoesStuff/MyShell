#include "terminalstream.h"
#include "mysh.h"

int interactiveMode(char *path, int interactive);

int main(int argc, char **argv) {
    int interactive = 1; // set interactive mode true by default
    char *path = NULL;

    DEBUG printf("argc:%i\n",argc);

    // check if a 2nd arg (file path) is given
    if (argc >= 2) {
        
        DEBUG printf("arg found:%s\n",argv[1]);
        path = (char *)(argv[1]);

        // open file given
        int fd = open(path,O_RDONLY);


        if (fd < 0) {printf("error opening file: %s\n",path);}


        // check if that file is a tty or a batch file
        if (isatty(fd) == 1) {
            
            // it is a tty
            printf("running in interactive mode, given terminal path\n");

            // here I may need to change the output from the current terminal to the terminal I was given
            //if (dup2(fd, STDOUT_FILENO) < 0) {perror("Failed to dup");}

        } else {
            // is not a tty
            printf("running in batch mode\n");
            interactive = 0;

        }
    } else {
        // no path given, running in current terminal?
        // check if stdin is a terminal or being piped
        if (isatty(STDIN_FILENO) == 1) 
        {
            // it is a terminal
            printf("is a terminal\n");
            interactive = 1;
        } else {
            // it is not a terminal
            printf("not a terminal\n");
            interactive = 0;
        }
        DEBUG printf("running in interactive mode, no path given\n");
    }

    // if its being run in interactive mode
    if (interactive) {
        interactiveMode(path, interactive);
    }

}

/**
 * Being run in interactive mode
 * return 0 on fail
 * return 1 on success
*/
int interactiveMode(char *path, int interactive) {

    //int pipeFD = open("./pipeFD", O_WRONLY);

    if (interactive == 1) {
        printf("\n\nWelcome to mysh\n\n");
    }

    int isRunning = 1;        

    while (isRunning) 
    {

        // maybe move these inside of terminal Stream?? idea?
        int wordCount = 0;
        char **wordArr=NULL;   

        if (interactive == 1) {
            printf("\nmysh> ");
            fflush(stdout); // flush the stdout that way the mysh> gets printed before it starts looking for out input
        }

        // read in from the terminal
        
        // setup the command struct and initialize it
        command cmd;
        int termStreamRetCode = terminalStream(&wordArr, &wordCount, &cmd);

        // check if error
        if(termStreamRetCode == -1) {
            printf("Terminal Stream Error\n");

            free(wordArr);
            exit(EXIT_FAILURE);
        }
        // check if user wants to exit
        else if(termStreamRetCode == 0) {
            //printf("calling free at exit;2\n");
            //freeArr(&wordArr, &wordCount);
            free(wordArr);
            isRunning = 0;
            continue;
        }
        // terminalStream was sucessfull
        // get next command

        //freeWords(&cmd);
        free(wordArr);
        continue;
    }

    printf("\nmysh: exiting\n");
    return 0;
}