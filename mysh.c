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

    printf("\n\nWelcome to mysh\n\n");
    
    int isRunning = 1;        

    while (isRunning) 
    {
        int arrSize = 10;
        char **arr=NULL;
        int wordCount = 0;

        printf("mysh> ");
        // read in from the terminal
        int termError = terminalStream(&arrSize,&arr,&wordCount);
        if(termError <= 0) {
            printf("Terminal Stream Error: %i\n",termError);
            free(arr);
            exit(EXIT_FAILURE);
        }
        if (arr==NULL) {
            printf("arr is null\n");
            free(arr);
            exit(EXIT_FAILURE);
        }

        if (strcmp(arr[0],"exit") == 0) {
            printf("exit typed\n");
            break;
        }

        // use the cmd in the terminal

        //int execl(const char *path, const char *arg, ...);
        char *bins[] = {"/usr/local/bin/","/usr/bin/","/bin/"};
    
        int childpid;
        int status;
        if ((childpid = fork()) == -1) {
            // fork failed
            perror("Can't fork:");
            exit(EXIT_FAILURE);
        } else if (childpid == 0) {
            // only in child
            for (int i=0;i<sizeof(bins)/sizeof(bins[0]); i++)
            {
                int bufSize = strlen(arr[0]) + strlen(bins[i]) + 1;

                char *buf = malloc((char)bufSize);
                if (buf == NULL) {perror("Buf wasn't allocated"); exit(EXIT_FAILURE);}


                snprintf(buf,bufSize,"%s%s",bins[i],arr[0]);

                DEBUG printf("\nSearching:%s\n",buf);

                if (execv(buf, arr) != -1) {
                    exit(EXIT_SUCCESS);
                }
            }
            
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
        free(arr); // free the arr obj to prevent memory leaks
    }

    printf("mysh: exiting");
    return 0;
}