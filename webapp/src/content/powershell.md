Works on **Windows** (PowerShell 5.1+). Needs `git` installed.

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/main/cli-tool/scripts/install.ps1 | iex
```

What it does:

1. Downloads `xommit.exe` from GitHub Releases
2. Installs it to `%LOCALAPPDATA%\Programs\xommit`
3. Adds the install directory to your user PATH
4. Prints next steps

**Restart your terminal** after installation so the PATH changes take effect.

Verify:

```powershell
xommit --version
```
