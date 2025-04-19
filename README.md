
# Jack Analyzer

## Overview
This project is a **Jack Analyzer** which processes Jack source code using lexical analysis, parsing, and generating XML output. The analyzer follows the Jack language specification and is built using C. It includes a **lexer**, **parser**, and an **analyzer** for analyzing Jack programs.

## Directory Structure

```
/JackAnalyzer
├── Makefile            # Build script for compiling and linking the program
├── README.md           # Project documentation
├── analyzer.c          # Contains the main analysis logic
├── lexer.c             # Lexer implementation for tokenizing Jack code
├── lexer.h             # Lexer header defining token structures and functions
├── parser.c            # Parser implementation for Jack source code
├── parser.h            # Parser header defining parse functions
├── tests/              # Folder for test files
│   └── SquareGame.jack   # Example Jack source code to test the analyzer
└── build/              # Folder to hold object files during compilation
```

## Dependencies

- **Clang** or **GCC** (Clang is preferred if available)
- **POSIX** environment (e.g., Linux, macOS)

## Building the Project

To compile and build the **JackAnalyzer** executable, run the following command in the project directory:

```bash
make
```

This will compile all the `.c` files and generate an executable named `JackAnalyzer` in the project root.

### Custom Compilation Flags
You can pass custom compiler flags by setting the `CFLAGS` variable when running `make`. For example:

```bash
make CFLAGS="-Wall -Wextra -std=c99"
```

## Running the Analyzer

Once the project is built, you can run the **JackAnalyzer** on any Jack source file. The program will output XML representations of the Jack program's structure.

For example:

```bash
./JackAnalyzer tests/SquareGame.jack
```

This command will analyze `SquareGame.jack` and generate corresponding XML output.

## Cleaning Up

To remove the compiled files, object files, and any generated XML or `.out` files, run:

```bash
make clean
```

This will delete:
- The `JackAnalyzer` executable
- All object files in the `build/` directory
- Any `.xml` or `.out` files in the project directory

## File Descriptions

### `analyzer.c`
Contains the main logic for analyzing and interpreting the Jack source code. It uses the lexer and parser to generate XML output.

### `lexer.c` / `lexer.h`
The lexer is responsible for tokenizing the input Jack source code. It converts the raw text into meaningful tokens like keywords, symbols, integers, and identifiers.

### `parser.c` / `parser.h`
The parser takes the tokens produced by the lexer and builds a structured representation of the Jack program. It checks for syntax errors and produces an XML representation.

### `tests/SquareGame.jack`
An example Jack source file used for testing the analyzer. You can modify or add more Jack source files in this directory for testing purposes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
