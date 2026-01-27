CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -I./include
LDFLAGS = -pthread -lsqlite3 -lstdc++fs

# 源文件
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
DATA_DIR = data
LOGS_DIR = logs
STATIC_DIR = static

# 递归查找所有 .cpp 文件
SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/ChatGPTServer

# 项目根目录（用于 run 目标）
PROJECT_ROOT = $(shell pwd)

# 创建所有必要的目录
$(shell mkdir -p $(BUILD_DIR)/logger $(BUILD_DIR)/utils $(BUILD_DIR)/server $(BUILD_DIR)/database $(BIN_DIR) $(DATA_DIR) $(LOGS_DIR))

.PHONY: all clean run test help info install-deps

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "==================================="
	@echo "Linking $(TARGET)..."
	@echo "==================================="
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "✅ Build completed: $(TARGET)"
	@echo ""

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	@echo "✅ Clean completed"

clean-all: clean
	@echo "Cleaning logs and data..."
	rm -rf $(LOGS_DIR)/*.log
	rm -rf $(DATA_DIR)/*.db
	@echo "✅ Full clean completed"

run: $(TARGET)
	@echo "==================================="
	@echo "Starting ChatGPT Server..."
	@echo "==================================="
	@mkdir -p $(LOGS_DIR) $(DATA_DIR)
	@echo "Working directory: $(PROJECT_ROOT)"
	@echo "Static files: $(STATIC_DIR)/"
	@echo ""
	@cd $(PROJECT_ROOT) && $(TARGET) 8080

run-debug: $(TARGET)
	@echo "Starting server in debug mode..."
	@mkdir -p $(LOGS_DIR) $(DATA_DIR)
	@cd $(PROJECT_ROOT) && $(TARGET) 8080

test: $(TARGET)
	@echo "==================================="
	@echo "Running integration tests..."
	@echo "==================================="
	@echo "Testing health endpoint..."
	@curl -s http://localhost:8080/health | jq . || curl -s http://localhost:8080/health
	@echo ""
	@echo "Testing metrics endpoint..."
	@curl -s http://localhost:8080/metrics | jq . || curl -s http://localhost:8080/metrics
	@echo ""
	@echo "✅ Tests completed"

test-chat:
	@echo "Testing chat endpoint..."
	@curl -X POST http://localhost:8080/chat \
		-H "Content-Type: application/json" \
		-d '{"message":"Hello from Makefile test!","session_id":""}' | jq .

install-deps:
	@echo "==================================="
	@echo "Installing dependencies..."
	@echo "==================================="
	@echo "This requires sudo privileges."
	@echo ""
	@if command -v apt-get >/dev/null 2>&1; then \
		echo "Using apt-get..."; \
		sudo apt-get update && sudo apt-get install -y libsqlite3-dev; \
	elif command -v yum >/dev/null 2>&1; then \
		echo "Using yum..."; \
		sudo yum install -y sqlite-devel; \
	else \
		echo "⚠️  Unknown package manager. Please install libsqlite3-dev manually."; \
		exit 1; \
	fi
	@echo "✅ Dependencies installed"

info:
	@echo "==================================="
	@echo "ChatGPT Server - Milestone 3"
	@echo "==================================="
	@echo "Project root:    $(PROJECT_ROOT)"
	@echo "Source dir:      $(SRC_DIR)"
	@echo "Build dir:       $(BUILD_DIR)"
	@echo "Target:          $(TARGET)"
	@echo "Static files:    $(STATIC_DIR)"
	@echo "Data dir:        $(DATA_DIR)"
	@echo "Logs dir:        $(LOGS_DIR)"
	@echo ""
	@echo "Source files:"
	@echo "$(SOURCES)" | tr ' ' '\n' | sed 's/^/  - /'
	@echo ""
	@echo "Object files:"
	@echo "$(OBJECTS)" | tr ' ' '\n' | sed 's/^/  - /'
	@echo ""
	@echo "Compiler flags:  $(CXXFLAGS)"
	@echo "Linker flags:    $(LDFLAGS)"
	@echo ""

help:
	@echo "==================================="
	@echo "ChatGPT Server - Makefile Help"
	@echo "==================================="
	@echo "Available targets:"
	@echo ""
	@echo "  make all          - Build the project (default)"
	@echo "  make run          - Build and run the server on port 8080"
	@echo "  make run-debug    - Run with debug output"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make clean-all    - Remove build artifacts, logs, and database"
	@echo "  make test         - Run basic API tests"
	@echo "  make test-chat    - Test chat endpoint"
	@echo "  make install-deps - Install system dependencies (requires sudo)"
	@echo "  make info         - Show project information"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build the project"
	@echo "  make run          # Build and run on port 8080"
	@echo "  make clean all    # Clean and rebuild"
	@echo ""
	@echo "Access the server:"
	@echo "  Web UI:    http://localhost:8080"
	@echo "  Health:    http://localhost:8080/health"
	@echo "  Metrics:   http://localhost:8080/metrics"
	@echo ""

