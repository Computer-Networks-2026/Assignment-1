CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGET = scanner
SRC = scanner.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)