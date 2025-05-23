# ======== Пути ========
BIN     := bin
SRC     := src
INC     := include
BUILD   := build

# ======== Компилятор и флаги ========
CXX     := g++
CXXFLAGS:= -Wall -Wextra -O2 -std=c++17 -I$(INC)
DEPFLAGS:= -MMD -MP

CXXDEBUGFLAGS := -g -DDEBUG

# ======== Исходники и цели ========
SRCS    := $(shell find $(SRC) -name "*.cpp" ! -name "test.cpp")
OBJS    := $(patsubst $(SRC)/%, $(BUILD)/%, $(SRCS:.cpp=.o))
DEPS    := $(OBJS:.o=.d)

TARGET  := $(BIN)/solver

# ======== Тестовые цели ========
TEST_SRC := $(SRC)/test.cpp
TEST_OBJ := $(BUILD)/test.o
TEST_TARGET := $(BIN)/test

# ======== Сборка ========

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $@"
	@mkdir -p $(BIN)
	$(CXX) $(OBJS) -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# ======== Сборка тестов ========

$(BUILD)/test.o: $(SRC)/test.cpp
	@echo "Compiling test.cpp"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJ) $(filter-out $(BUILD)/main.o, $(OBJS))
	@echo "Linking test..."
	@mkdir -p $(BIN)
	$(CXX) $^ -o $@

.PHONY: test
test: $(TEST_TARGET)
	@echo "Running test..."
	@./$(TEST_TARGET) $(ARGS)

# ======== Запуск ========

.PHONY: run
run: all
	@echo "Running..."
	@./$(TARGET) $(ARGS)

# ======== Сборка/запуск с отладкой ========

.PHONY: debugrun
debugrun: CXXFLAGS += $(CXXDEBUGFLAGS)
debugrun: clean $(TARGET)
	@echo "Running with DEBUG..."
	@./$(TARGET) $(ARGS)

.PHONY: debugtest
debugtest: CXXFLAGS += $(CXXDEBUGFLAGS)
debugtest: clean $(TEST_TARGET)
	@echo "Running test with DEBUG..."
	@./$(TEST_TARGET) $(ARGS)

# ======== Чистка ========

.PHONY: clean
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD) $(BIN)/*.o $(BIN)/solver $(BIN)/test

# ======== Зависимости ========
-include $(DEPS)
