# MyShell
============

For Rutgers CS214 Systems Programming Project 3

## Names and RUIDs

Ron Cohen (rc1456) and Enrico Aquino (eja97)

## User Manuel

Type or copy and paste the following commands into the terminal after downloading

```bash
tar -xf your-p3.tar 
cd P3
make
```
Start the Mysh
```bash 
./mysh
```
Run the program using batch mode
```bash
./mysh `<commands/file>`
```
## Testing Plan

When making this program, gradually tested each command from the normal shell `(cd, ls, echo, which, etc.)` to see what the problems were and fix them accordingly. For example, with `cd` we made sure it found the desired directory and we made sure it changed to the correct directory in the parent process not the child. If the directory didn't exist, we printed an error but continued the progam. Same with `ls`, we make sure it was reading and printing the files and folders of the current or desired directory. We continued to do this with different commands to make sure they were working properly. After they were working, we then commands like `ls -a` to make sure it can read the `-a` option was working. We tested other commands to make sure their options were working. 

We moved on to test piping and redirecting. First thing we had to do was was make sure `|` was being read. We would feed `mysh` commands like `echo hi | echo hello` and `echo hi|echo hello` to make sure the pipe is working as intended. We then used other commands with piping to see if they were functioning properly. From there, we began testing redirecting. We started off by sending outputs, `>` of various commands like `echo`, `cat`, and `ls` to files we have created. Afterward, we began work on inputs, `<`. Same with output, we tested various commands to see if it as working correctly. From there, we combined piping and redirecting in several different commands. A few things we added that seem usefull is when there is a redirection without a file, it ignores the redirection that caused an error, prints an error, but continues the command as if that redirection was never there. Within this shell I tried to implement a few error correcting things.

Once we were comfortable with commands, we moved on to batch mode. We first made sure that `mysh` was able to read the file we were piping to it. Once `mysh` was being read files properly, we then moved on to see if commands could be read properly from the script we made, `hello.sh`. With this we were also able to see how then-else statements were working.


### Test Cases
#### Various test we have used for testing:
```sh 
ls | wc -w                                  expected around 15
ls | wc -w | wc -w                          expected 1
ls | grep . | sort | uniq -c | wc -l        expected about 15
echo hi | echo hello                        expected only hello
cd .. | pwd                                 expected printing out previous dictionary
cat < a                                     expected "echo hello world"
echo a file > a.txt                         expected truncate file a write "a file"
ls > a | sort < a | uniq -c | wc -w > b     expected truncate files and write file list in a and word count in b
```
#### Example test file
```sh
echo hello world                            expected "hello world"
then echo this should run                   expected "this should run"
else exit whoops                            This should not exit the program
echo this should also run                   expected "this should also run" because there isn't a else or then 
                                            at the beginning
```
```sh
echo hello world                            expected "hello world"
then echo this should run                   expected "this should run"
else exit whoops                            This should not exit the program
then echo this should also run              expected "this should also run"
```