CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -g
INCLUDES  := -I include

SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin
TARGET    := $(BIN_DIR)/interpreter

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
	@mkdir -p $@

run: $(TARGET)
	$(TARGET) $(PROGRAM)

test: $(TARGET)
	@echo
	@echo "Running all test programs in data/..."
	@for f in data/*.txt; do \
	  echo "[ $$f ]:"; \
	  $(TARGET) $$f; \
	  echo "----------"; \
	done

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: run test clean
