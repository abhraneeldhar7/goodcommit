#!/usr/bin/env node

const { execFileSync } = require("child_process");
const path = require("path");
const fs = require("fs");

const pkgPath = path.join(__dirname, "..", "package.json");
const pkg = require(pkgPath);

const bump = process.argv[2];
if (!bump) {
  console.error("Usage: npm run release -- <patch|minor|major|x.y.z>");
  process.exit(1);
}

const [major, minor, patch] = pkg.version.split(".").map(Number);
let version;
if (bump === "patch") version = `${major}.${minor}.${patch + 1}`;
else if (bump === "minor") version = `${major}.${minor + 1}.0`;
else if (bump === "major") version = `${major + 1}.0.0`;
else if (/^\d+\.\d+\.\d+$/.test(bump)) version = bump;
else {
  console.error("Invalid bump: " + bump);
  process.exit(1);
}

const repoRoot = execFileSync("git", ["rev-parse", "--show-toplevel"], { cwd: __dirname, encoding: "utf8" }).trim();
const branch = execFileSync("git", ["rev-parse", "--abbrev-ref", "HEAD"], { cwd: repoRoot, encoding: "utf8" }).trim();
const tag = "v" + version;
const rel = path.relative(repoRoot, pkgPath).split(path.sep).join("/");

pkg.version = version;
fs.writeFileSync(pkgPath, JSON.stringify(pkg, null, 2) + "\n");

execFileSync("git", ["add", rel], { cwd: repoRoot, stdio: "inherit" });
execFileSync("git", ["commit", "-m", `chore(release): ${tag}`], { cwd: repoRoot, stdio: "inherit" });
execFileSync("git", ["tag", "-a", tag, "-m", tag], { cwd: repoRoot, stdio: "inherit" });

console.log("");
console.log(`Tagged ${tag}. Push it to release:`);
console.log(`  git push origin ${branch}`);
console.log(`  git push origin ${tag}`);
