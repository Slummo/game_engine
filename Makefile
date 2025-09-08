# Tools
CXX		:= g++
CC		:= gcc
MKDIR	:= mkdir -p

LIBS	:= glfw3 opengl assimp openal sndfile

# Project layout
SRC_DIR			:= src
INCLUDE_DIR		:= include
DEPS_DIR		:= deps
BUILD_DIR		:= build
OBJ_DIR			:= $(BUILD_DIR)/obj
BIN_DIR			:= $(BUILD_DIR)/bin
TARGET			:= $(BIN_DIR)/main
GLAD_DIR		:= $(DEPS_DIR)/glad
IMGUI_DIR		:= $(DEPS_DIR)/imgui
STB_IMAGE_DIR	:= $(DEPS_DIR)/stb_image

# Compilation flags
COMMON_FLAGS	:= -MMD -MP -Wall -Wextra -Wunused-macros -Wunused-parameter -Wunused-but-set-parameter
CXXSTD			:= -std=c++20
CSTD			:= -std=c11
CXXFLAGS		:= $(CXXSTD) $(COMMON_FLAGS) -Wpedantic
CFLAGS			:= $(CSTD) $(COMMON_FLAGS)

# Preprocessor flags
CPPFLAGS		:=	-I$(INCLUDE_DIR) \
					-I$(DEPS_DIR) \
					-I$(IMGUI_DIR) \
					-I$(IMGUI_DIR)/backends \
					-DIMGUI_IMPL_OPENGL_LOADER_GLAD \
					$(shell pkg-config --cflags $(LIBS))

# Linker flags
LDFLAGS			:= 							# -L...
LDLIBS			:= -ldl $(shell pkg-config --libs $(LIBS))

# Profiles flags
DEBUG_FLAGS		:= -g -O0 -fno-omit-frame-pointer -DDEBUG
RELEASE_FLAGS	:= -O3 -march=native -DNDEBUG

# Source files
SRCS			:= $(shell find $(SRC_DIR) -name '*.c' -o -name '*.cpp')



GLAD_SRCS		:= $(GLAD_DIR)/glad.c
IMGUI_SRCS		:= $(IMGUI_DIR)/imgui.cpp \
				   $(IMGUI_DIR)/imgui_draw.cpp \
				   $(IMGUI_DIR)/imgui_tables.cpp \
				   $(IMGUI_DIR)/imgui_widgets.cpp \
				   $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
				   $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
STB_IMAGE_SRCS	:= $(STB_IMAGE_DIR)/stb_image.c

SRCS			+= $(GLAD_SRCS) $(IMGUI_SRCS) $(STB_IMAGE_SRCS)

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
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
# C++ user-defined
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# C user-defined
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# C++ dependencies
$(OBJ_DIR)/deps/%.o: $(DEPS_DIR)/%.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

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
