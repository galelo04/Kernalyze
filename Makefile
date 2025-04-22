CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

SRC = src
OBJ = obj

SRC_FILES := $(wildcard $(SRC)/**/*.c) $(wildcard $(SRC)/*.c)
OBJ_FILES := $(patsubst $(SRC)/%.c, $(OBJ)/%.c.o, $(SRC_FILES))

# Rule to compile each .c file to .o
$(OBJ)/%.c.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJ_FILES)

clean:
	rm -f $(OBJ_FILES)
# rm -rf $(OBJ)