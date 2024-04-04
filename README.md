# MyShell
============

For Rutgers CS214 Systems Programming Project 3

## Names and RUIDs

Ron Cohen (rc1456) and Enrico Aquino (eja97)

## User Manuel

Type or copy and paste the following commands into the terminal after downloading



I tested the piping using 
ls | wc -w                              expected around 15
ls | wc -w | wc -w                      expected 1
ls | grep . | sort | uniq -c | wc -l    expected about 15
echo hi | echo hello                    expected only hello