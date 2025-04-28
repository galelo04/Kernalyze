CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2
LDFLAGS = -lm

SRC = src
OBJ = bin
TARGET_PROCESS = process
TARGET_PROCESS_GENERATOR = os-sim

SRC_FILES := $(wildcard $(SRC)/**/*.c) $(wildcard $(SRC)/*.c)

# Define the object files for process and process_generator
OBJ_FILES_PROCESS := $(patsubst $(SRC)/%.c, $(OBJ)/%.c.o, $(filter $(SRC)/process.c $(SRC)/clk.c $(SRC)/utils/%.c , $(SRC_FILES)))
OBJ_FILES_PROCESS_GENERATOR := $(patsubst $(SRC)/%.c, $(OBJ)/%.c.o, $(filter $(SRC)/process_generator.c $(SRC)/utils/%.c $(SRC)/scheduler.c $(SRC)/clk.c , $(SRC_FILES)))

# Rule to compile each .c file to .o
$(OBJ)/%.c.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build process executable
$(TARGET_PROCESS): $(OBJ_FILES_PROCESS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

# Build process_generator executable
$(TARGET_PROCESS_GENERATOR): $(OBJ_FILES_PROCESS_GENERATOR)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)


# Build all targets (process and process_generator)
all: $(TARGET_PROCESS) $(TARGET_PROCESS_GENERATOR) keyfile

# Run process executable
run_process: $(TARGET_PROCESS)
	@echo "Running process..."
	@./$(TARGET_PROCESS)

# Run process_generator executable
run: all
	@echo "Running process_generator..."
	@./$(TARGET_PROCESS_GENERATOR)

# Generate keyfiles
keyfile:
	@echo "Generating keyfile..."
	@mkdir -p keyfolder
	@touch keyfolder/queueKey.txt
	@touch keyfolder/shmKey.txt
	@touch keyfolder/semKey.txt

# Clean up object files and executables
clean:
	rm -f $(OBJ_FILES_PROCESS) $(OBJ_FILES_PROCESS_GENERATOR)
	rm -rf $(OBJ)
	rm -f $(TARGET_PROCESS) $(TARGET_PROCESS_GENERATOR)


# Test generator

test_generator:
	@echo "Compiling test generator..."
	$(CC) $(CFLAGS) -o bin/test_generator test_generator.c

test_generator_run: test_generator
	@echo "Running test generator..."
	@./bin/test_generator