# COSC1114 Operating Systems Assignment 2 Makefile
# Multithreading & Synchronisation

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -O2 -pthread

# Target executables
TARGETS = mmcopier mscopier

# Source files
MMCOPIER_SRC = mmcopier.cpp
MSCOPIER_SRC = mscopier.cpp

# Object files
MMCOPIER_OBJ = $(MMCOPIER_SRC:.cpp=.o)
MSCOPIER_OBJ = $(MSCOPIER_SRC:.cpp=.o)

# Default target - build all programs
all: $(TARGETS)

# Task 1 - Multiple file copier
mmcopier: $(MMCOPIER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Task 2 - Single file copier with multithreading
mscopier: $(MSCOPIER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Generic rule for compiling .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f *.o $(TARGETS)

# Test targets for development
test-mmcopier: mmcopier
	@echo "Testing mmcopier..."
	@mkdir -p tests/source_dir tests/destination_dir
	@echo "Test content 1" > tests/source_dir/source1.txt
	@echo "Test content 2" > tests/source_dir/source2.txt
	@echo "Test content 3" > tests/source_dir/source3.txt
	@./mmcopier 3 tests/source_dir tests/destination_dir
	@echo "mmcopier test completed. Check tests/destination_dir/"

test-mscopier: mscopier
	@echo "Testing mscopier..."
	@echo "Creating test input file..."
	@echo -e "Line 1\nLine 2\nLine 3\nLine 4\nLine 5" > test_input.txt
	@./mscopier 4 test_input.txt test_output.txt
	@if cmp -s test_input.txt test_output.txt; then \
		echo "SUCCESS: Files are identical"; \
	else \
		echo "ERROR: Files differ"; \
	fi
	@rm -f test_input.txt test_output.txt

# Run both tests
test: test-mmcopier test-mscopier

# Memory leak testing with valgrind
valgrind-test: mscopier
	@echo "Running valgrind memory check..."
	@echo -e "Line 1\nLine 2\nLine 3\nLine 4\nLine 5" > valgrind_input.txt
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./mscopier 4 valgrind_input.txt valgrind_output.txt
	@rm -f valgrind_input.txt valgrind_output.txt

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build both mmcopier and mscopier"
	@echo "  mmcopier      - Build Task 1 program"
	@echo "  mscopier      - Build Task 2 program"
	@echo "  clean         - Remove object files and executables"
	@echo "  test          - Run basic tests for both programs"
	@echo "  test-mmcopier - Test Task 1 program"
	@echo "  test-mscopier - Test Task 2 program"
	@echo "  valgrind-test - Run memory leak check on mscopier"
	@echo "  help          - Show this help message"

# Declare phony targets (targets that don't create files)
.PHONY: all clean test test-mmcopier test-mscopier valgrind-test help