# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wno-unused-parameter -I src
LDFLAGS = -ldl -lreadline

# Directories
SRC_DIR = src
PLUGIN_DIR = $(SRC_DIR)/plugins
BIN_DIR = .

# Target executable
TARGET = $(BIN_DIR)/plugin_manager

# Source files
MAIN_SRC = $(SRC_DIR)/main.c
PLUGIN_SRCS = $(wildcard $(PLUGIN_DIR)/*.c)

# Output plugin files (in root directory)
PLUGIN_OBJS = $(patsubst $(PLUGIN_DIR)/%.c,$(BIN_DIR)/%.so,$(PLUGIN_SRCS))

# Default target
all: $(TARGET) $(PLUGIN_OBJS)

# Link the main program
$(TARGET): $(MAIN_SRC) $(SRC_DIR)/plugin_manager.h
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(TARGET) $(LDFLAGS)

# Compile each plugin into a shared library in the root directory
$(BIN_DIR)/%.so: $(PLUGIN_DIR)/%.c $(SRC_DIR)/plugin_manager.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

# Clean up
clean:
	rm -f $(TARGET) $(BIN_DIR)/*.so

# Phony targets
.PHONY: all clean
