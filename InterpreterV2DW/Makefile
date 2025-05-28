CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

$(BIN_DIR)/interpreter: $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(BIN_DIR)/interpreter
	$(BIN_DIR)/interpreter $(PROGRAM)

test: $(BIN_DIR)/interpreter
	for prog in data/*; do \
		echo "Running $$prog"; \
		$(BIN_DIR)/interpreter $$prog; \
		echo ""; \
	done

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
