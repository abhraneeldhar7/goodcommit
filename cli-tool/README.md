![XOMMIT - conventional commits, zero effort](https://raw.githubusercontent.com/abhraneeldhar7/xommit/main/assets/heroImg.png)

# xommit

**Conventional commits, zero effort.**

[![npm version](https://img.shields.io/npm/v/@abhraneeldhar7/xommit?color=fa583b&label=npm)](https://www.npmjs.com/package/@abhraneeldhar7/xommit) [![license](https://img.shields.io/npm/l/@abhraneeldhar7/xommit?color=fa583b)](https://opensource.org/licenses/MIT) ![platforms](https://img.shields.io/badge/platform-windows%20%7C%20macOS%20%7C%20linux-fa583b)

xommit is a small CLI that writes your git commits for you. You stage your files, type a vague message like `fixed the login bug`, and it reads the diff, asks Groq (a free AI API) for a few proper [Conventional Commits](https://www.conventionalcommits.org/), and lets you pick one. Then it commits with the one you choose.

It ships as a single C++ binary, around 3 MB, and runs on Windows, macOS and Linux.

## Install

```bash
npm install -g @abhraneeldhar7/xommit
```

That's it. The command is now `xommit`.

Not into npm? There's also a curl installer for macOS/Linux, a PowerShell one for Windows, and a manual build — all listed in the [repo](https://github.com/abhraneeldhar7/xommit#install).

## Set up your Groq key

xommit uses [Groq](https://groq.com), which is free, and just needs an API key:

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

Or just run `xommit` with no message and it'll generate options from the diff.

## Commands

| Command | What it does |
| --- | --- |
| `xommit "<message>"` | Generate commit options and commit |
| `xommit --help` | Show the tutorial |
| `xommit --connect` | Save or update your Groq API key |
| `xommit --reset` | Remove your stored key |
| `xommit --update` | Download the latest release from GitHub |
| `xommit --version` | Show the version |

## Requirements

- [Node.js](https://nodejs.org/) (for the npm install)
- `git` (xommit commits for you)

## Links

- [GitHub](https://github.com/abhraneeldhar7/xommit)
- [Website](https://xommit.antk.in)
- [Issues](https://github.com/abhraneeldhar7/xommit/issues)

## License

MIT
