# ---------- конфигурация ----------
CXX       := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -g -MMD -MP -Wno-unused-variable
INCLUDES  := -I include

CXXFLAGS += -Wno-unused-variable

SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin
TARGET    := $(BIN_DIR)/interpreter

# ---------- файлы ------------------
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# ---------- правила ----------------
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# «%.o» зависит только от своего .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

-include $(OBJS:.o=.d)

$(BUILD_DIR) $(BIN_DIR):
	@mkdir -p $@

run: $(TARGET)
	$(TARGET) $(PROGRAM)

test: $(TARGET)
	@echo
	@echo "Running all test programs in data/..."
	@cd test && ./run_tests.sh

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: run test clean
