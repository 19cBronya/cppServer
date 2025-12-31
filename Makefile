CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -I./include
LDFLAGS = -pthread

# 源文件
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

SOURCES = $(wildcard $(SRC_DIR)/**/*.cpp) $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/ChatGPTServer

# 创建目录
$(shell mkdir -p $(BUILD_DIR)/logger $(BUILD_DIR)/utils $(BUILD_DIR)/server $(BIN_DIR))

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build completed: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -rf logs/*.log

run: $(TARGET)
	@mkdir -p logs
	$(TARGET)

test: $(TARGET)
	@echo "Running tests..."
	curl http://localhost:8080/health || echo "Server not running"

help:
	@echo "Available targets:"
	@echo "  all   - Build the project (default)"
	@echo "  clean - Remove build artifacts"
	@echo "  run   - Build and run the server"
	@echo "  test  - Run basic tests"

