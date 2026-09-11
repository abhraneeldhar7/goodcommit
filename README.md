# goodcommit - CLI Tool

This folder contains the goodcommit command-line tool.

## What is goodcommit?

goodcommit takes a vague commit message like "fixed the login bug" and turns it into a proper Conventional Commit like "fix(auth): resolve login timeout on expired sessions". It uses the Groq API (an AI service) to generate three options, and you pick the best one.

## How does it work?

1. You run: `goodcommit "fixed the login bug"`
2. The tool reads your staged git changes (the files you ran `git add` on)
3. It sends your changes and your vague message to the Groq API
4. The AI returns three conventional commit options
5. You select one (numbered selection in standalone, arrow keys in npm)
6. The tool runs `git commit` with your chosen message

## How is it built?

The tool has two parts:

1. **C++ binary** (`src/main.cpp` + headers) - Handles the core logic:
   - API call to Groq
   - Git operations (diff, stat, commit)
   - Key management (storage in appdata)
   - Numbered selection UI

2. **JS wrapper** (`lib/index.js`) - Handles npm delivery and fancy UI:
   - Calls C++ binary with `--json` flag
   - Shows arrow key selection with `>` icon
   - Runs git commit

The C++ binary is ~200KB. The JS wrapper is ~5KB. Total package size is tiny.

## How do I install it?

Three options:

### Option 1: npm (recommended)

Requires Node.js installed.

```
cd cli-tool
npm install -g .
```

This makes the `goodcommit` command available everywhere on your system.

### Option 2: curl installer (Linux/macOS)

Requires curl or wget.

```
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/goodcommit/main/cli-tool/scripts/install.sh | bash
```

This downloads a standalone binary (no Node.js required) and installs it to `/usr/local/bin`.

### Option 3: Manual build

Requires g++ compiler.

```
cd cli-tool
node scripts/build.js
# or manually:
g++ -std=c++17 -O2 -I include -o bin/goodcommit src/main.cpp -lcurl
```

Then copy `bin/goodcommit` to a directory in your PATH.

## Setup

After installing, set your Groq API key:

```
goodcommit --key add
```

This stores your key at:
- Windows: `%APPDATA%\goodcommit\key`
- Linux/macOS: `~/.config/goodcommit/key`

## Usage

```
goodcommit "your vague commit message"
goodcommit --print "your vague message"    # just print, don't commit
goodcommit --key add                        # store API key
goodcommit --key remove                     # remove stored key
goodcommit --help                           # show help
goodcommit --test                           # test UI (DEV mode only)
```

## Environment configuration

The tool supports environment configuration through a `.env` file.

### DEV mode

When `ENV=DEV`, the `--test` flag becomes available. This lets you test the UI without calling the API or needing staged files.

```
cp .env.example .env
# Edit .env and uncomment ENV=DEV
goodcommit --test
```

## Folder structure

- `include/goodcommit/` - C++ header files (the core logic)
  - `prompt.h` - AI instruction text
  - `json.h` - JSON escaping and parsing
  - `key_manager.h` - API key storage
  - `http_client.h` - HTTPS requests to Groq API
  - `git_utils.h` - Git operations
  - `terminal_ui.h` - Terminal UI (spinner, selection)
- `src/main.cpp` - C++ entry point
- `lib/index.js` - JS wrapper for npm delivery
- `bin/` - Compiled binaries (gitignored)
- `scripts/` - Build and install scripts
- `tests/` - Basic tests
- `CMakeLists.txt` - CMake build configuration
- `package.json` - npm package configuration
