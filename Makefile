CC = gcc
CFLAGS = -Wall -ggdb
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# List of source files (assuming all .c files in src/)
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
HDR_FILES = $(wildcard $(SRC_DIR)/*.h)

# Generate a list of object file names by replacing .c with .o
OBJ_SRC_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Default target: build the executable
all: $(BIN_DIR)/squicel

# Copy source files to the obj directory
$(OBJ_DIR)/%.c: $(SRC_DIR)/%.c
	cp $< $@

$(OBJ_DIR)/%.h: $(SRC_DIR)/%.h
	cp $< $@

# Compile the object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create the executable from object files
$(BIN_DIR)/squicel: $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Clean target: remove object files
clean:
	rm -Force $(OBJ_DIR)/*.o

# Target with the "-DTESTS" flag
tests: CFLAGS += -DTESTS
tests: all

.PHONY: all clean tests