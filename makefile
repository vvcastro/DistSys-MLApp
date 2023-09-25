CCX := g++
CXXFLAGS := -std=c++11 -pthread -Isrc -Isrc/network/include -Isrc/app/include

SRC_DIR := src
OBJ_DIR := obj
BUILD_DIR := build

SRC := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/network/*.cpp) $(wildcard $(SRC_DIR)/app/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(BUILD_DIR)/main

$(BUILD_DIR)/main: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CCX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

.PHONY: all clean