# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -g

# Target executable
TARGET = lab5

# Source files
SRC = lab5.c 

# Object files
OBJ = $(SRC:.c=.o)

# Default target: compile the program
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove object files and executable
clean:
	rm -f $(OBJ) $(TARGET)
