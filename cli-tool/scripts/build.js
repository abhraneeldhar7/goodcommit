#!/usr/bin/env node

// scripts/build.js - Compile the C++ binary
//
// This script compiles src/main.cpp into an executable.
// It detects the platform and uses the right compiler flags.
//
// Usage:
//   node scripts/build.js
//   npm run build
//
// Output:
//   Windows: bin/goodcommit.exe
//   Linux:   bin/goodcommit
//   macOS:   bin/goodcommit-macos

const { execSync } = require("child_process");
const path = require("path");
const fs = require("fs");
const os = require("os");

const srcDir = path.join(__dirname, "..", "src");
const includeDir = path.join(__dirname, "..", "include");
const binDir = path.join(__dirname, "..", "bin");

// Create bin directory if it doesn't exist
if (!fs.existsSync(binDir)) {
  fs.mkdirSync(binDir, { recursive: true });
}

// Find all .cpp source files
const sources = fs.readdirSync(srcDir)
  .filter((f) => f.endsWith(".cpp"))
  .map((f) => path.join(srcDir, f));

const platform = os.platform();
// c++ version 20 might cause problem if your g++ is super old.
const flags = "-std=c++20 -O2 -static";

let compiler="g++", exe, linkFlags;

if (platform === "win32") {
  exe = path.join(binDir, "goodcommit.exe");
  linkFlags = "-lwininet -pthread";
} else if (platform === "linux") {
  exe = path.join(binDir, "goodcommit");
  linkFlags = "-lcurl -pthread";
} else if (platform === "darwin") {
  exe = path.join(binDir, "goodcommit-macos");
  linkFlags = "-lcurl -pthread";
} else {
  console.error("Unsupported platform: " + platform);
  process.exit(1);
}

// Build the g++ command
// -I includeDir tells the compiler where to find our header files
const cmd = `"${compiler}" ${flags} -I "${includeDir}" -o "${exe}" ${sources.map((s) => '"' + s + '"').join(" ")} ${linkFlags}`;

console.log("Building goodcommit for " + platform + "...");
console.log("> " + cmd);
console.log("");

try {
  execSync(cmd, { stdio: "inherit", cwd: __dirname });
  console.log("");
  console.log("Build successful: " + exe);

  // Show file size
  const stats = fs.statSync(exe);
  const sizeKB = Math.round(stats.size / 1024);
  console.log("Binary size: " + sizeKB + " KB");
} catch (err) {
  console.error("");
  console.error("Build failed. Make sure g++ is installed and in your PATH.");
  console.error("");
  console.error("Windows: Install MinGW from https://www.mingw-w64.org/");
  console.error("Linux:   sudo apt install g++");
  console.error("macOS:   xcode-select --install");
  process.exit(1);
}
