CC = gcc
CFLAGS = -Wall -Wextra -O2

SRC = src
OBJ = bin
TARGET = os-sim

SRC_FILES := $(wildcard $(SRC)/**/*.c) $(wildcard $(SRC)/*.c)
OBJ_FILES := $(patsubst $(SRC)/%.c, $(OBJ)/%.c.o, $(SRC_FILES))

# Rule to compile each .c file to .o
$(OBJ)/%.c.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

all: $(TARGET) keyfile

run: all
	@echo "Running the program..."
	@./$(TARGET)

keyfile:
	@echo "Generating keyfile..."
	@mkdir -p keyfile
	@touch keyfile/key.txt

test_generator:
	@echo "Compiling test generator..."
	$(CC) $(CFLAGS) -o bin/test_generator test_generator.c

test_generator_run: test_generator
	@echo "Running test generator..."
	@./bin/test_generator

clean:
	rm -f $(OBJ_FILES)
	rm -rf $(OBJ)