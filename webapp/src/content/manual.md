Build from source. You need a few things installed first:

- [Git](https://git-scm.com/)
- [Node.js](https://nodejs.org/)
- A C++20 compiler (g++ or clang++)

**1. Clone the repo** from [github.com/abhraneeldhar7/xommit](https://github.com/abhraneeldhar7/xommit):

```terminal
git clone https://github.com/abhraneeldhar7/xommit.git
cd xommit/cli-tool
```

**2. Install dependencies** (for the build script):

```terminal
npm install
```

**3. Build:**

```terminal
node scripts/build.js
```

This compiles `src/main.cpp` into a binary in the `bin/` directory:

- Windows: `bin/xommit.exe`
- Linux: `bin/xommit`
- macOS: `bin/xommit-macos`

**4. Add it to your PATH:**

Linux:

```terminal
sudo cp bin/xommit /usr/local/bin/
```

macOS:

```terminal
sudo cp bin/xommit-macos /usr/local/bin/xommit
```

Windows (PowerShell):

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\Programs\xommit"
Copy-Item bin\xommit.exe "$env:LOCALAPPDATA\Programs\xommit\"
```

Or add `cli-tool/bin/` to your PATH permanently.

**5. Connect your Groq key:**

```terminal
xommit --connect
```

**Compiler requirements:**

- Windows: [MinGW-w64](https://www.mingw-w64.org/) (g++)
- Linux: `sudo apt install g++`
- macOS: `xcode-select --install`
