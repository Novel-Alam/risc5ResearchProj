# Vars
CC = cc # C compiler, platform independent
SRC_DIR = src
INC_DIR = inc
OUT_DIR = out
ARCH = $(shell uname -m) #In case it's needed for cross compiling
BIN_DIR = $(OUT_DIR)/bin
OBJ_DIR = $(OUT_DIR)/obj
TARGET = $(BIN_DIR)/main
CFLAGS = -I$(INC_DIR) -Wall -Wextra# Flags for C Compiler

# Find all .c files in the src directory
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Glob src/ for .c files, replace file names with .o
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Make All
all: $(TARGET)

# Start Chain
# Create binary directory and compile the target
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create object directory
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Clean up the build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean