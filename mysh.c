#include "terminalstream.h"
#include "mysh.h"

int interactiveMode(char *path);

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
        if (isatty(STDOUT_FILENO) == 1) {
            
            // it is a tty
            DEBUG printf("running in interactive mode, given terminal path\n");

            // here I may need to change the output from the current terminal to the terminal I was given
            //if (dup2(fd, STDOUT_FILENO) < 0) {perror("Failed to dup");}

        } else {
            // is not a tty
            DEBUG printf("running in batch mode\n");
            interactive = 0;

        }
    } else {
        // no path given, running in current terminal?
        DEBUG printf("running in interactive mode, no path given\n");
    }

    // if its being run in interactive mode
    if (interactive) {
        interactiveMode(path);
    }

}

/**
 * Being run in interactive mode
 * return 0 on fail
 * return 1 on success
*/
int interactiveMode(char *path) {

    int pipeFD = open("./pipeFD", O_CREAT, O_CREAT, O_RDWR, 0777);
    if (pipeFD==-1){
        perror("open error:");
        exit(EXIT_FAILURE);
    }

    printf("\n\nWelcome to mysh\n\n");
    
    int isRunning = 1;        

    while (isRunning) 
    {

        // maybe move these inside of terminal Stream?? idea?
        int wordCount = 0;
        char **wordArr=NULL;

        printf("\nmysh> ");
        fflush(stdout); // flush the stdout that way the mysh> gets printed before it starts looking for out input

        // read in from the terminal
        int termStreamRetCode = terminalStream(&wordArr, &wordCount);
        
        // check if error
        if(termStreamRetCode == -1) {
            printf("Terminal Stream Error\n");
            //freeArr(&wordArr, &wordCount);
            exit(EXIT_FAILURE);
        }
        // check if user wants to exit
        else if(termStreamRetCode == 0) {
            //printf("calling free at exit;2\n");
            //freeArr(&wordArr, &wordCount);
            isRunning = 0;
            continue;
        }
        // terminalStream was sucessfull

        // get next command
        //printf("calling free at end;3\n");
        //freeArr(&wordArr, &wordCount);
        continue;
    }

    printf("mysh: exiting\n");
    return 0;
}


int freeArr(char ***wordArr, int *wordCount) { 
    if (wordArr == NULL || *wordArr == NULL) return 0; // Nothing to free
    
    for (int i = 0; i < *wordCount; i++) {
        if ((*wordArr)[i] == NULL) {
            continue;
        }
        printf("freeing: \'%s\'\n",(*wordArr)[i]);
        free((*wordArr)[i]);
        (*wordArr)[i] = NULL; // prevent dangling pointer
    }
    free(*wordArr);
    *wordArr = NULL; // Prevent dangling pointer at the top level
    return 1; // Success
}