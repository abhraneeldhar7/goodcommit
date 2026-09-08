#!/usr/bin/env node

const { execSync } = require("child_process");
const path = require("path");
const fs = require("fs");
const os = require("os");

const srcDir = path.join(__dirname, "src");
const binDir = path.join(__dirname, "bin");

if (!fs.existsSync(binDir)) {
  fs.mkdirSync(binDir, { recursive: true });
}

const sources = fs.readdirSync(srcDir)
  .filter((f) => f.endsWith(".cpp"))
  .map((f) => path.join(srcDir, f));

const platform = os.platform();
const flags = "-std=c++17 -O2";

let exe, linkFlags;

if (platform === "win32") {
  exe = path.join(binDir, "goodcommit.exe");
  linkFlags = "-lwininet";
} else if (platform === "linux") {
  exe = path.join(binDir, "goodcommit-linux");
  linkFlags = "-lcurl";
} else if (platform === "darwin") {
  exe = path.join(binDir, "goodcommit-macos");
  linkFlags = "-lcurl";
} else {
  console.error("Unsupported platform: " + platform);
  process.exit(1);
}

const cmd = `g++ ${flags} -o "${exe}" ${sources.map((s) => '"' + s + '"').join(" ")} ${linkFlags}`;

console.log("Building goodcommit for " + platform + "...");
console.log("> " + cmd);

try {
  execSync(cmd, { stdio: "inherit", cwd: __dirname });
  console.log("Build successful: " + exe);
} catch (err) {
  console.error("Build failed. Make sure g++ is installed and in your PATH.");
  process.exit(1);
}
