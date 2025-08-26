# Tools
CXX		:= g++
CC		:= gcc
MKDIR		:= mkdir -p

# Project layout
SRC_DIR		:= src
INCLUDE_DIR	:= include
DEPS_DIR	:= deps
BUILD_DIR	:= build
OBJ_DIR		:= $(BUILD_DIR)/obj
BIN_DIR		:= $(BUILD_DIR)/bin
TARGET		:= $(BIN_DIR)/main

# Compilation flags
COMMON_FLAGS		:= -MMD -MP -Wall -Wextra -Wunused-macros -Wunused-parameter -Wunused-but-set-parameter
CXXSTD			:= -std=c++20
CSTD			:= -std=c11
CXXFLAGS		:= $(CXXSTD) $(COMMON_FLAGS) -Wpedantic
CFLAGS			:= $(CSTD) $(COMMON_FLAGS)

# Preprocessor flags
CPPFLAGS		:= -I$(INCLUDE_DIR) -I$(DEPS_DIR) -I$(DEPS_DIR)/glm -I$(DEPS_DIR)/glad -I$(DEPS_DIR)/stb_image

# Linker flags
LDFLAGS			:= 							# -L...
LDLIBS			:= -lglfw -ldl -lGL -lassimp

# Profiles flags
DEBUG_FLAGS		:= -g -O0 -fno-omit-frame-pointer -DDEBUG
RELEASE_FLAGS		:= -O3 -march=native -DNDEBUG

# Source files
SRCS := $(shell find $(SRC_DIR) -name '*.c' -o -name '*.cpp') \
        $(shell find $(DEPS_DIR) -name '*.c' -o -name '*.cpp' \
            ! -path "$(DEPS_DIR)/glm/*")	# Exclude glm since its header-only

# Object files
OBJS := \
  $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
  $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.c,$(SRCS))) \
  $(patsubst $(DEPS_DIR)/%.cpp,$(OBJ_DIR)/deps/%.o,$(filter $(DEPS_DIR)/%.cpp,$(SRCS))) \
  $(patsubst $(DEPS_DIR)/%.c,$(OBJ_DIR)/deps/%.o,$(filter $(DEPS_DIR)/%.c,$(SRCS)))

# Dependency files
DEPS := $(OBJS:.o=.d)

# Default target
all: release

# Release profile
release: PROFILE_FLAGS := $(RELEASE_FLAGS)
release: $(TARGET)

# Debug profile
debug: PROFILE_FLAGS := $(DEBUG_FLAGS)
debug: $(TARGET)

# Link rule
$(TARGET): $(OBJS)
	@$(MKDIR) $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile rules
# C++
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# C user-defined
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# C dependencies
$(OBJ_DIR)/deps/%.o: $(DEPS_DIR)/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# Include dependency files
-include $(DEPS)

# Convenience rules
run: release
	@$(TARGET)

gdb: debug
	@gdb --args $(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all release debug run gdb clean
