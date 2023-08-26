CC := gcc
LIBS := -lpthread
CFLAGS := -Wall -Wextra -Isrc/include

SRC_DIR := src
OBJ_DIR := obj
BUILD_DIR := build

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OPS := -p

ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32
	OPS := 
endif

all: directories sender receiver node

directories:
	mkdir $(OPS) $(BUILD_DIR) $(OBJ_DIR)

sender: $(OBJ_DIR)/sender.o $(OBJ_DIR)/network.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/sender $(OBJ_DIR)/sender.o $(OBJ_DIR)/network.o $(LIBS)

receiver: $(OBJ_DIR)/receiver.o $(OBJ_DIR)/network.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/receiver $(OBJ_DIR)/receiver.o $(OBJ_DIR)/network.o $(LIBS)

node: $(OBJ_DIR)/node.o $(OBJ_DIR)/network.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/node $(OBJ_DIR)/node.o $(OBJ_DIR)/network.o $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(OBJ_DIR)

.PHONY: all clean