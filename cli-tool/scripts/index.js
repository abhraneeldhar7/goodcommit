#!/usr/bin/env node

// scripts/index.js - npm shim
//
// Finds the C++ binary, downloads it from GitHub Releases if missing,
// then forwards all args to it.

const { execFileSync } = require("child_process");
const path = require("path");
const fs = require("fs");
const https = require("https");

const config = require("../config.json");
const pkg = require("../package.json");

const REPO = config.github + "/releases/download/v" + pkg.version;

// Find binary for platform
const platform = process.platform;
let binary, asset;

if (config.assets[platform]) {
  binary = path.join(__dirname, "..", "bin", config.binaries[platform]);
  asset = config.assets[platform];
} else {
  console.error("Unsupported platform: " + platform);
  process.exit(1);
}

// Download binary from GitHub Releases if missing
function download() {
  const url = REPO + "/" + asset;
  const binDir = path.dirname(binary);

  if (!fs.existsSync(binDir)) {
    fs.mkdirSync(binDir, { recursive: true });
  }

  console.error("Downloading " + asset + " from GitHub Releases...");

  return new Promise((resolve, reject) => {
    https.get(url, { headers: { "User-Agent": "xommit" } }, (res) => {
      if (res.statusCode === 302 || res.statusCode === 301) {
        https.get(res.headers.location, (res2) => {
          const file = fs.createWriteStream(binary);
          res2.pipe(file);
          file.on("finish", () => { file.close(); resolve(); });
        }).on("error", reject);
      } else if (res.statusCode === 200) {
        const file = fs.createWriteStream(binary);
        res.pipe(file);
        file.on("finish", () => { file.close(); resolve(); });
      } else {
        reject(new Error("Download failed: HTTP " + res.statusCode));
      }
    }).on("error", reject);
  });
}

async function main() {
  if (!fs.existsSync(binary)) {
    try {
      await download();
      if (platform !== "win32") {
        fs.chmodSync(binary, 0o755);
      }
    } catch (err) {
      console.error("Failed to download binary: " + err.message);
      console.error("Download manually from: " + REPO);
      process.exit(1);
    }
  }

  try {
    execFileSync(binary, process.argv.slice(2), { stdio: "inherit" });
  } catch (err) {
    process.exit(err.status !== undefined ? err.status : 1);
  }
}

main();
