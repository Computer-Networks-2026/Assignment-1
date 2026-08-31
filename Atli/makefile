CXX         := clang++
CXXFLAGS    := -Wall -Wextra -pedantic -std=c++17 -Iinclude -g \
			   -fcolor-diagnostics
LDFLAGS     := 

MAIN_TARGET := build/main
CLIENT_TARGET := build/client

.PHONY: all clean run run-client test

all: $(MAIN_TARGET) $(CLIENT_TARGET)

$(MAIN_TARGET): src/main.cpp | build
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(CLIENT_TARGET): src/client.cpp | build
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

build:
	@mkdir -p build

run: $(MAIN_TARGET)
	@./$(MAIN_TARGET) 130.208.246.98 4000 4100

run-client: $(CLIENT_TARGET)
	@./$(CLIENT_TARGET) 130.208.246.98

clean:
	$(RM) -r build


