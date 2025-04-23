CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

SRC = src
OBJ = bin
TARGET = bin/os-sim

SRC_FILES := $(wildcard $(SRC)/**/*.c) $(wildcard $(SRC)/*.c)
OBJ_FILES := $(patsubst $(SRC)/%.c, $(OBJ)/%.c.o, $(SRC_FILES))

# Rule to compile each .c file to .o
$(OBJ)/%.c.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

all: $(OBJ_FILES)

run: $(TARGET)
	@echo "Running the program..."
	@./$(TARGET)

clean:
	rm -f $(OBJ_FILES)
# rm -rf $(OBJ)