# C++ Headers - include/goodcommit/

This folder contains the C++ header files that implement the core logic.

## What are header files?

Header files (`.h`) contain function declarations, constants, and type definitions.
They are "included" by other files using `#include "filename.h"`.

Why separate headers from source files?
- Headers define WHAT functions exist
- Source files define HOW functions work
- This lets multiple source files use the same functions

## How to read these files

Each header file has detailed comments explaining:
- What each function does
- Why it was designed that way
- How platform-specific code works (Windows vs Linux)
- Beginner-friendly explanations of C++ concepts

## The files

### prompt.h
Contains the AI instruction text. This tells the Groq AI how to generate commit messages.

### json.h
JSON helper functions. Handles escaping strings for JSON and parsing API responses.

### key_manager.h
API key storage. Saves/loads/deletes the Groq API key from a file on disk.

### http_client.h
HTTPS requests. Sends POST requests to the Groq API. Uses WinINet on Windows, libcurl on Linux/macOS.

### git_utils.h
Git operations. Runs git commands (diff, stat, commit) and captures their output.

### terminal_ui.h
Terminal user interface. Handles the spinner animation, arrow key selection, and VT processing setup.

## Building

These headers are compiled together with `src/main.cpp` to create the binary. The build command is:

```
g++ -std=c++17 -O2 -I include -o bin/goodcommit src/main.cpp -lcurl
```

The `-I include` flag tells the compiler to look for headers in the `include/` directory.
