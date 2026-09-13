Works on **Linux** and **macOS**. Requires `curl` or `wget`.

```bash
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/main/cli-tool/scripts/install.sh | bash
```

What it does:

1. Detects your OS (Linux/macOS) and architecture (x64/arm64)
2. Downloads the correct binary from GitHub Releases
3. Makes it executable
4. Installs it to `/usr/local/bin` (may ask for sudo)
5. Prints next steps

**Windows users:** This installer does not work on Windows. Use npm or PowerShell instead.

Verify:

```bash
xommit --version
```
