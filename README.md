# Caesar - Cross-Platform Debugger

A modern C++20 cross-platform debugger.

## Features

- **Expression Evaluation**: Built-in interpreter for evaluating expressions during debugging
- **Interactive REPL**: Command-line interface for interactive debugging sessions
- **Cross-Platform**: Designed to work across different operating systems

## Architecture

The debugger is built with a modular architecture:

- **Core Engine** (`src/core/`): Core debugger engine functionality
- **Command System** (`src/cmd/`): Expression parser, interpreter, and AST components

## Building

### Prerequisites

- CMake 3.31.0 or higher
- C++20 compatible compiler
- Standard development tools

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the debugger without arguments to enter interactive mode:

```bash
./caesar
```

This starts a REPL where you can enter expressions and commands.

### Development Environment

The project includes a development setup script:

```bash
./start.sh
```

This creates a tmux session with editor, compiler, and miscellaneous panes for efficient development. This is my preffered environment, feel free to use or adapt it to your own needs.

## Components

### Expression Engine
- **Scanner**: Tokenizes input expressions
- **Parser**: Builds abstract syntax trees from tokens  
- **Interpreter**: Evaluates expressions with support for literals, grouping, unary and binary operations
- **AST Printer**: Debug utility for visualizing abstract syntax trees

## Project Status

This is an early-stage project under active development. The core expression evaluation system is functional, with binary analysis and other features in development.

## License

MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
