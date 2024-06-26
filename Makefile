# Define the compiler
CC=gcc

# Compiler flags
CFLAGS=-Wall -g -fsanitize=address

# Debugging flags
DFLAGS=-Wall -g -fsanitize=address -D__DEBUG 

# Define the target executable name
TARGET_MYSH=mysh

# List of source files for mysh
SOURCES_MYSH= mysh.c linestream.c executeCmd.c

# Object files to generate for mysh
OBJECTS_MYSH=$(SOURCES_MYSH:.c=.o)

# Default rule to build the mysh
mysh: $(TARGET_MYSH)
	make clean

# Debug rule to build the program mysh with debug flags
debug: CFLAGS = $(DFLAGS)
debug: $(TARGET_MYSH)
	make clean

# Rule for linking the program
$(TARGET_MYSH): $(OBJECTS_MYSH)
	$(CC) $(CFLAGS) -o $@ $^

# Rule for compiling source files to object files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

# Special case for mysh.c which might not need a mysh.h
# mysh.o: mysh.c
# 	$(CC) $(CFLAGS) -c $<

# Rule for cleaning up
clean:
	rm -f $(OBJECTS_MYSH)

# Dependencies
linestream.o: linestream.c linestream.h
executeCmd.o: executeCmd.c executeCmd.h
mysh.o: mysh.c mysh.h # Add this line if mysh.h exists and is used