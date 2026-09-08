#!/usr/bin/env node

const { execFileSync } = require("child_process");
const path = require("path");
const fs = require("fs");

const platform = process.platform;
let binary;

if (platform === "win32") {
  binary = path.join(__dirname, "..", "bin", "goodcommit.exe");
} else if (platform === "linux") {
  binary = path.join(__dirname, "..", "bin", "goodcommit-linux");
} else if (platform === "darwin") {
  binary = path.join(__dirname, "..", "bin", "goodcommit-macos");
} else {
  console.error("Unsupported platform: " + platform);
  process.exit(1);
}

if (!fs.existsSync(binary)) {
  console.error(
    "Binary not found: " + binary + "\n" +
    "Try reinstalling: npm install -g goodcommit"
  );
  process.exit(1);
}

const args = process.argv.slice(2);

if (args.includes("--help") || args.includes("-h")) {
  console.log("Usage: goodcommit [options] <commit message>\n");
  console.log("Turn a vague commit message into a Conventional Commit.\n");
  console.log("Options:");
  console.log("  --key add      Store your Groq API key");
  console.log("  --key remove   Remove stored API key");
  console.log("  --print        Print the commit message without committing");
  console.log("  --help         Show this help message");
  console.log("  --             Separator: everything after this is the message");
  process.exit(0);
}

if (args.includes("--key")) {
  try {
    execFileSync(binary, args, { stdio: "inherit" });
  } catch (err) {
    process.exit(err.status !== undefined ? err.status : 1);
  }
  process.exit(0);
}

if (args.length === 0) {
  console.error("Error: No commit message provided.");
  console.error("Usage: goodcommit <your vague message>");
  process.exit(1);
}

let result;
try {
  result = execFileSync(binary, ["--json", ...args], { encoding: "utf-8" });
} catch (err) {
  if (err.stderr) process.stderr.write(err.stderr);
  process.exit(err.status !== undefined ? err.status : 1);
}

let options;
try {
  const data = JSON.parse(result);
  options = data.options;
} catch (e) {
  console.error("Error: Failed to parse response from binary.");
  process.exit(1);
}

if (!options || options.length === 0) {
  console.error("Error: No options returned.");
  process.exit(1);
}

if (args.includes("--print")) {
  console.log(options[0]);
  process.exit(0);
}

const readline = require("readline");
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.setEncoding("utf8");

let selected = 0;

function draw() {
  process.stdout.write("\x1b[?25l");
  for (let i = 0; i < options.length; i++) {
    process.stdout.write("\x1b[2K");
    if (i === selected) {
      process.stdout.write("\x1b[1;32m  \u276F " + options[i] + "\x1b[0m\n");
    } else {
      process.stdout.write("    " + options[i] + "\n");
    }
  }
  process.stdout.write("\x1b[2K");
  process.stdout.write("  \x1b[90m(\u2191\u2193 to navigate, Enter to confirm)\x1b[0m\n");
  process.stdout.write("\x1b[" + (options.length + 1) + "A");
}

console.log("");
draw();

process.stdin.on("data", function (key) {
  if (key === "\x1b[A") {
    selected = (selected - 1 + options.length) % options.length;
    draw();
  } else if (key === "\x1b[B") {
    selected = (selected + 1) % options.length;
    draw();
  } else if (key === "\r") {
    process.stdout.write("\x1b[?25h");
    process.stdin.setRawMode(false);
    process.stdin.pause();
    for (let i = 0; i <= options.length; i++) {
      process.stdout.write("\x1b[2K");
      if (i <= options.length) process.stdout.write("\x1b[1A");
    }
    process.stdout.write("\x1b[2K");
    console.log("  \x1b[1;32m\u2714 " + options[selected] + "\x1b[0m");
    console.log("");
    try {
      execFileSync("git", ["commit", "-m", options[selected]], { stdio: "inherit" });
    } catch (err) {
      process.exit(err.status !== undefined ? err.status : 1);
    }
    process.exit(0);
  } else if (key === "\x03") {
    process.stdout.write("\x1b[?25h");
    process.exit(130);
  }
});
