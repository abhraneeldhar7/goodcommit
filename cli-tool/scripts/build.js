#!/usr/bin/env node

// scripts/build.js - Compile the C++ binary
//
// This script compiles src/main.cpp into an executable.
// It detects the platform and uses the right compiler flags.
//
// Usage:
//   node scripts/build.js
//   node scripts/build.js --release
//   npm run build
//
// Output:
//   Windows: bin/goodcommit.exe
//   Linux:   bin/goodcommit
//   macOS:   bin/goodcommit-macos

const { execFileSync } = require("child_process");
const path = require("path");
const fs = require("fs");
const os = require("os");

const config = require("../config.json");

const srcDir = path.join(__dirname, "..", "src");
const includeDir = path.join(__dirname, "..", "include");
const binDir = path.join(__dirname, "..", "bin");

const release = process.argv.includes("--release");

// Create bin directory if it doesn't exist
if (!fs.existsSync(binDir)) {
  fs.mkdirSync(binDir, { recursive: true });
}

// Find all .cpp source files
const sources = fs.readdirSync(srcDir)
  .filter((f) => f.endsWith(".cpp"))
  .map((f) => path.join(srcDir, f));

const platform = os.platform();

let compiler = "g++", exe, linkFlags;

if (platform === "win32") {
  exe = path.join(binDir, config.binaries.win32);
  linkFlags = ["-lwininet", "-pthread"];
} else if (platform === "linux") {
  exe = path.join(binDir, config.binaries.linux);
  linkFlags = ["-lcurl", "-pthread"];
} else if (platform === "darwin") {
  exe = path.join(binDir, config.binaries.darwin);
  linkFlags = ["-lcurl", "-pthread"];
} else {
  console.error("Unsupported platform: " + platform);
  process.exit(1);
}

const defines = [
  `-DGOODCOMMIT_VERSION="${config.version}"`,
  `-DGOODCOMMIT_GITHUB="${config.github}"`,
  `-DGOODCOMMIT_WEBSITE="${config.website}"`,
  `-DGOODCOMMIT_ASSET_WIN="${config.assets.win32}"`,
  `-DGOODCOMMIT_ASSET_LINUX="${config.assets.linux}"`,
  `-DGOODCOMMIT_ASSET_MACOS="${config.assets.darwin}"`,
];

// c++ version 20 might cause problem if your g++ is super old.
const flags = ["-std=c++20", "-O2"];
if (platform === "win32") flags.push("-static");
flags.push(...defines);
if (!release) flags.push("-D", "GOODCOMMIT_DEV");

const args = [...flags, "-I", includeDir, "-o", exe, ...sources, ...linkFlags];

const printable = [compiler, ...args]
  .map((a) => (a.includes(" ") ? `"${a}"` : a))
  .join(" ");

console.log("Building goodcommit for " + platform + (release ? " (release)" : " (dev)") + "...");
console.log("> " + printable);
console.log("");

try {
  execFileSync(compiler, args, { stdio: "inherit", cwd: __dirname });
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
