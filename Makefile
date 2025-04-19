CC := gcc
CFLAGS = 

# Directories
OBJ_DIR = build
SRC_DIR = .

# Files
OBJS = $(OBJ_DIR)/analyzer.o $(OBJ_DIR)/parser.o $(OBJ_DIR)/lexer.o
HEADERS = lexer.h parser.h
OUTPUT = JackAnalyzer

# Create object directory if it doesn't exist
$(shell mkdir -p $(OBJ_DIR))

.PHONY: all clean

# Default target
all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to compile analyzer.o
$(OBJ_DIR)/analyzer.o: $(SRC_DIR)/analyzer.c $(HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/analyzer.c -o $@

# Rule to compile parser.o
$(OBJ_DIR)/parser.o: $(SRC_DIR)/parser.c $(HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/parser.c -o $@

# Rule to compile lexer.o
$(OBJ_DIR)/lexer.o: $(SRC_DIR)/lexer.c $(HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/lexer.c -o $@

# Clean up object files, output files, and generated xml files
clean:
	rm -f $(OUTPUT) $(OBJS)
	find . -type f \( -name '*.xml' -o -name '*.out' \) -delete