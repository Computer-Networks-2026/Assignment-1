CXX      := clang++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++17 -Iinclude -g \
		   -fcolor-diagnostics
LDFLAGS  := 
TARGET   := build/main

SRC      := $(wildcard src/*.cpp)
OBJ      := $(patsubst src/%.cpp, build/%.o, $(SRC))

.PHONY: all clean run test

all: $(TARGET)

# Pattern rule: src/foo.c -> build/foo.o
build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Final linking
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

# Create build directory if it is missing
build:
	@mkdir -p build

run: $(TARGET)
	@./$(TARGET)

clean:
	$(RM) -r build


