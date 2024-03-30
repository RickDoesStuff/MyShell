#include "terminalstream.h"
#include "mysh.h"

int interactiveMode(char *path);
int freeArr(char ****arr, int *cmdCount, int **allocatedWords);

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
        int cmdCount = 0;
        char ***arr=NULL;
        int wordCount = 0;
        char **wordArr=NULL;

        int *allocatedWords = malloc(256*sizeof(int));
        printf("\nmysh> ");
        fflush(stdout); // flush the stdout that way the mysh> gets printed before it starts looking for out input

        // read in from the terminal
        int termError = terminalStream(&arr, &cmdCount, &wordArr,&wordCount, &allocatedWords);
        if(termError == -1) {
            printf("Terminal Stream Error\n");
            exit(EXIT_FAILURE);
        } 
        else if(termError == -2) {
            printf("bash: syntax error: %s\n",arr[0][0]);
            freeArr(&arr, &cmdCount, &allocatedWords);
            continue;
        }
        // returned with nothing, (user didn't type anything, just hit enter)
        else if (termError == 0) {
            // jump back to getting user input
            continue;
        }

        // if the arr is somehow null
        if (arr==NULL) {
            printf("arr is null\n");
            exit(EXIT_FAILURE);
        }
        // if the arr is somehow null
        if (arr[0]==NULL) {
            printf("arr[0] is null\n");
            exit(EXIT_FAILURE);
        }
        // if the arr is somehow null
        if (arr[0][0]==NULL) {
            printf("arr[0][0] is null\n");
            exit(EXIT_FAILURE);
        }
        
        // Exit command
        if (strcmp(arr[0][0],"exit") == 0) {
            DEBUG printf("exit typed\n");
            freeArr(&arr, &cmdCount, &allocatedWords);
            break;
        } 
        // PWD command
        else if (strcmp(arr[0][0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd failed");
            }
            // get next command
            freeArr(&arr, &cmdCount, &allocatedWords);
            continue;
        } else if (strcmp(arr[0][0], "cd") == 0) {
            if (arr[0][1] == NULL) {
                printf("No directory given\n");
            } else {
                if (chdir(arr[0][1]) != 0) {
                    perror("cd failed");
                }
            }
            // get next command
            freeArr(&arr, &cmdCount, &allocatedWords);
            continue;
        }



        // Assuming cmdCount is the total number of commands (including pipes, if, then, else if any)
        for (int i = 0; i < cmdCount; i++) {
            printf("[ ");
            int j = 0;
            //[[ls,-l,NULL],[echo hello]]
            while ((arr)[i][j] != NULL && strcmp((arr)[i][j], "") != 0) { // Assuming NULL as the sentinel at the end of each command's word array
                printf("[\"%s\"] ",(arr)[i][j]); // Free each word in the array
                j++;
            }
            printf("]\n");
        }


        // use the cmd in the terminal
        for (int cmdIndex = 0; cmdIndex < cmdCount; cmdIndex++) {
        // first need to check if the cmd is a cmd or a pipe, if, then, or else

            // check if the next cmd is a pipe, if it is, set current output to a file
            if (cmdIndex+1 < cmdCount && strcmp(arr[cmdIndex+1][0], "|") == 0) {
                int pipeFDS[2] = {STDIN_FILENO,pipeFD};
                if(pipe(pipeFDS) != 0)
                {
                    perror("pipe error:");
                    freeArr(&arr, &cmdCount, &allocatedWords);
                    exit(EXIT_FAILURE);
                }
                printf("output set\n");
            }
            // current cmd is pipe, if it is, set the next cmds input to the file the prev command wrote to
            if(cmdIndex+1 < cmdCount && strcmp(arr[cmdIndex+1][0], "|") == 0)
            {
                int pipeFDS[2] = {pipeFD,STDIN_FILENO};
                if(pipe(pipeFDS) != 0)
                {
                    perror("pipe error:");
                    freeArr(&arr, &cmdCount, &allocatedWords);
                    exit(EXIT_FAILURE);
                }
                printf("input set\n");

                // go to next command

                //continue;
            }

            //int execl(const char *path, const char *arg, ...);
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
                    int cmdSize = strlen(arr[0][0]) + strlen(bins[binIndex]) + 1;
                    char *cmd = malloc((char)cmdSize);

                    if (cmd == NULL) {perror("Buf wasn't allocated"); exit(EXIT_FAILURE);}

                    snprintf(cmd,cmdSize,"%s%s",bins[binIndex],arr[cmdIndex][0]);// add the current commands first word to the end of the path to search

                    DEBUG printf("\nSearching:%s\n",cmd);

                    if (execv(cmd, arr[cmdIndex]) != -1) {
                        free(cmd);
                        
                        // free the stuff in the child process??
                        freeArr(&arr, &cmdCount, &allocatedWords);
                        exit(EXIT_SUCCESS);
                    }

                    // free buf
                    if (cmd != NULL) {
                        free(cmd);
                    }
                }
                

                // free the stuff in the child process??
                freeArr(&arr, &cmdCount, &allocatedWords);

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
        }
        // Assuming cmdCount is the total number of commands (including pipes, if, then, else if any)
        //freeWordArr(&wordArr, &wordCount);
        freeArr(&arr, &cmdCount, &allocatedWords);
    }

    printf("mysh: exiting\n");
    return 0;
}

int freeArr(char ****arr, int *cmdCount, int **allocatedWords) {
    if (arr == NULL || *arr == NULL) return 0; // Nothing to free
    for (int i = 0; i < *cmdCount; i++) {
        int j = 0;
        printf("freeing cmd with length: %i\n",(*allocatedWords)[i]);
        while (j < (*allocatedWords)[i]) {
            free((*arr)[i][j]);
            (*arr)[i][j] = NULL; // Prevent dangling pointer
            j++;
        }
        free((*arr)[i]);
        (*arr)[i] = NULL; // Again, prevent dangling pointer
    }
    free(*arr);
    *arr = NULL; // Prevent dangling pointer at the top level
    free(*allocatedWords);
    return 1; // Success
}

int freeWordArr(char ***wordArr, int *wordCount) { 
    if (wordArr == NULL || *wordArr == NULL) return 0; // Nothing to free
    
    for (int i = 0; i < *wordCount; i++) {
        if ((*wordArr)[i] == NULL) {
            continue;
        }
        printf("freeingWA:%s\n",(*wordArr)[i]);
        free((*wordArr)[i]);
        (*wordArr)[i] = NULL; // prevent dangling pointer
    }
    free(*wordArr);
    *wordArr = NULL; // Prevent dangling pointer at the top level
    return 1; // Success
}