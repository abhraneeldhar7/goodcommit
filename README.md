<p align="center">
  <img src="/webapp/public/og-image.png" alt="XOMMIT - conventional commits, zero effort" width="760">
</p>

<h1 align="center">xommit</h1>

<p align="center"><strong>Conventional commits, zero effort.</strong></p>

<p align="center">
  <a href="https://www.npmjs.com/package/@abhraneeldhar7/xommit"><img src="https://img.shields.io/npm/v/@abhraneeldhar7/xommit?color=fa583b&label=npm" alt="npm version"></a>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/npm/l/@abhraneeldhar7/xommit?color=fa583b" alt="license"></a>
  <img src="https://img.shields.io/badge/platform-windows%20%7C%20macOS%20%7C%20linux-fa583b" alt="platforms">
  <a href="https://github.com/abhraneeldhar7/xommit/stargazers"><img src="https://img.shields.io/github/stars/abhraneeldhar7/xommit?color=fa583b" alt="stars"></a>
</p>

## What is xommit?

xommit is a small CLI that writes your git commits for you. You stage your files, type a vague message like `fixed the login bug`, and it reads the diff, asks Groq (a free AI API) for a few proper [Conventional Commits](https://www.conventionalcommits.org/), and lets you pick one. It then commits with the one you choose.

It ships as a single C++ binary, around 3 MB, and runs on Windows, macOS and Linux.

## How it works

1. You stage some files with `git add`.
2. You run `xommit "your vague message"`.
3. xommit grabs the staged diff and sends it to Groq.
4. The AI returns 3 conventional commit options.
5. You pick one with the arrow keys and hit Enter.
6. xommit runs `git commit` with your chosen message.

## Install

Pick whichever fits your setup. The npm route is the easiest.

**npm** (Windows, macOS, Linux)

```bash
npm install -g @abhraneeldhar7/xommit
```

Needs [Node.js](https://nodejs.org/) installed.

**curl** (macOS, Linux)

```bash
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli-tool/scripts/install.sh | bash
```

Downloads the standalone binary to `/usr/local/bin`.

**PowerShell** (Windows)

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli-tool/scripts/install.ps1 | iex
```

Installs `xommit.exe` to `%LOCALAPPDATA%\Programs\xommit` and adds it to your PATH. Restart your terminal after.

**Manual** (build from source) — see [Build from source](#build-from-source).

All of the installers expect `git` to already be installed.

## Connect your Groq key

xommit uses [Groq](https://groq.com) under the hood, which is free. You just need an API key:

1. Create an account at [console.groq.com](https://console.groq.com).
2. Open [console.groq.com/keys](https://console.groq.com/keys) and hit **Create API Key**.
3. Copy the key (it starts with `gsk_` and is shown only once).
4. Run `xommit --connect` and paste it in.

```bash
xommit --connect
```

Your key is stored locally and is only ever used to talk to Groq.

## Usage

```bash
git add .
xommit "fixed the login bug"
```

Don't have a message in mind? Just run `xommit` and it'll generate options from the diff.

## Commands

| Command | What it does |
| --- | --- |
| `xommit "<message>"` | Generate commit options and commit |
| `xommit --help` | Show the tutorial |
| `xommit --connect` | Save or update your Groq API key |
| `xommit --reset` | Remove your stored key |
| `xommit --update` | Download the latest release from GitHub |
| `xommit --version` | Show the version |

## Build from source

You'll need [Git](https://git-scm.com/), [Node.js](https://nodejs.org/) and a C++20 compiler (g++ or clang++).

```bash
git clone https://github.com/abhraneeldhar7/xommit.git
cd xommit/cli-tool
node scripts/build.js
```

The binary lands in `bin/`:

- Windows: `bin/xommit.exe`
- Linux: `bin/xommit`
- macOS: `bin/xommit-macos`

Drop it somewhere on your PATH and you're good to go.

## Project structure

```
cli-tool/   the C++ CLI, plus the build and install scripts
webapp/     the website (Astro + Tailwind)
assets/     images
.github/    release workflow (builds binaries on every v* tag)
```

## Website

There's a site with a live demo and the same install guide: [xommit.antk.in](https://xommit.antk.in).

## A note on how this was built

I'm inspired by the black magic [@ruben](https://x.com/RubenVeidt) does on Twitter. Wanted to move past the DSA questions, so I used AI to set up the skeleton and then iterated on it. I'm using this project to learn C++ properly, and I wanted to share a tool I actually use.

## License

MIT
