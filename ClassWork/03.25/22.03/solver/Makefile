# Solver project directory structure

BIN := bin
SRC := src
INC := include
BUILD := build

# Compiler flags

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17
DEPFLAGS := -MMD -MP

SRCS := $(shell find $(SRC) -name "*.cpp")
OBJS := $(subst $(SRC)/,$(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))
DEPS := $(OBJS:.o=.d)

TARGET := $(BIN)/solver.o

# Run task rules

run: build
	@echo "Running..."
	./$(TARGET) $(ARGS)

# Build task

build: clean all

# Main task

all: $(TARGET)

# Link target task

$(TARGET): $(OBJS)
	@echo "Building..."
	mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@

# Compile task

$(BUILD)/%.o: $(SRC)/%.cpp
	@echo "Compiling $<..."
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INC) -c -o $@ $<

# Clean rules

.PHONY: clean

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD)

# Include all dependencies

-include $(DEPS)
