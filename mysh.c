#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//#include <dirent.h>

int interactiveMode(char *path);

int main(int argc, char **argv) {
    int interactive = 1; // set interactive mode true by default
    char *path = NULL;

    printf("argc:%i\n",argc);

    // check if a 2nd arg (file path) is given
    if (argc >= 2) {
        
        printf("arg found:%s\n",argv[1]);
        path = (char *)(argv[1]);

        // open file given
        int fd = open(path,O_RDONLY);


        if (fd < 0) {printf("error opening file: %s\n",path);}


        // check if that file is a tty or a batch file
        if (isatty(STDOUT_FILENO) == 1) {
            
            // it is a tty
            printf("running in interactive mode, given terminal path\n");

            // here I may need to change the output from the current terminal to the terminal I was given
            if (dup2(fd, STDOUT_FILENO) < 0) {perror("Failed to dup");}

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
 * Being run in interactive mode
 * return 0 on fail
 * return 1 on success
*/
int interactiveMode(char *path) {

    printf("\n\nWelcome to mysh\n\n");

    int isRunning = 1;
    char *userMessage = malloc(sizeof(char) * 256);
    while (isRunning) {
        printf("mysh> ");

        if (!fgets(userMessage, 256, stdin)) {
            printf("Error reading input or EOF\n");
            isRunning = 0;
            break; // Exit the loop on EOF or error
        }

        // Remove newline character if present
        userMessage[strcspn(userMessage, "\n")] = '\0';
        printf("read:%s\n",userMessage);
        if(strcmp(userMessage, "exit") == 0) {
            
            printf("exiting");
            isRunning = 0;
            break;
        }



        // use the cmd in the terminal

        //int execl(const char *path, const char *arg, ...);
        char *bins[] = {"/usr/local/bin/","/usr/bin/","/bin/"};
        
        for (int i=0;i<sizeof(bins)/sizeof(bins[0]); i++)
        {
            int bufSize = strlen(userMessage) + strlen(bins[i]);
            char *buf = malloc((char)bufSize);
            snprintf(buf,bufSize,"%s%s",bins[i],userMessage);

            printf("Searching:%s\n",buf);
            if (execl(buf, userMessage, NULL) == -1) {
                printf("Failed to execl:%s\n",buf);
                perror("execl error:");
            }
        }
    }




    printf("mysh: exiting");
    return 0;
}