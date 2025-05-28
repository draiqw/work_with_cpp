CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(wildcard include/*.h)

TARGET = $(BIN_DIR)/interpreter

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

run: $(TARGET)
	$(TARGET)

test: $(TARGET)
	@for f in data/testProg*.txt; do \
		echo "Running $$f:"; \
		$(TARGET) $$f; \
		echo "----"; \
	done

clean:
	$(RM) -r $(OBJ_DIR) $(BIN_DIR)

.PHONY: run test clean
