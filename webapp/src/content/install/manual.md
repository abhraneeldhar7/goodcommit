Build from source. Requires a C++ compiler (g++ or clang++).

**1. Clone the repo:**

```bash
git clone https://github.com/abhraneeldhar7/xommit.git
cd xommit/cli-tool
```

**2. Install dependencies (for the build script):**

```bash
npm install
```

**3. Build:**

```bash
node scripts/build.js
```

This compiles `src/main.cpp` into a binary in the `bin/` directory.

- Windows: `bin/xommit.exe`
- Linux: `bin/xommit`
- macOS: `bin/xommit-macos`

**4. Add to PATH:**

Copy the binary to a directory in your PATH, for example:

```bash
# Linux/macOS
sudo cp bin/xommit /usr/local/bin/

# Windows (PowerShell)
copy bin\xommit.exe "$env:LOCALAPPDATA\Programs\xommit"
```

Or add `cli-tool/bin/` to your PATH permanently.

**Compiler requirements:**

- Windows: [MinGW](https://www.mingw-w64.org/) (g++)
- Linux: `sudo apt install g++`
- macOS: `xcode-select --install`
