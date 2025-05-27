# ---------------- Параметры проекта ----------------
BIN   := bin
SRC   := src
INC   := include
BUILD := build

CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17
DEPFLAGS := -MMD -MP           # авто-зависимости *.d

# ---------------------------------------------------
SRCS := $(shell find $(SRC) -name "*.cpp")
OBJS := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

TARGET := $(BIN)/solver        # итоговый исполняемый файл

# ---------------- Правила --------------------------
.PHONY: all build run clean

all: build

build: $(TARGET)

run: build
	@echo "Запуск..."
	./$(TARGET) $(ARGS)

$(TARGET): $(OBJS)
	@echo "Link   $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	@echo "Compil $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INC) -c $< -o $@

clean:
	@echo "Clean"
	@rm -rf $(BUILD) $(BIN)

-include $(DEPS)
