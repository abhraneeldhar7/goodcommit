# Publishing xommit

How a code change turns into a GitHub release, an npm package, and a binary people actually run. Read this once and the release flow stops being scary.

## The one rule

`cli-tool/package.json` → `version` is the **only** place a version is written. Everything else derives from it:

- `build.js` passes `-DXOMMIT_VERSION="${pkg.version}"`, so `xommit --version` prints it.
- The git tag must equal it (`vX.Y.Z`).
- npm's version *is* that field.
- The binary download URL is `.../releases/download/v<version>/...`.

So:

```
tag === package.json === npm === xommit --version === the GitHub release
```

If those ever disagree, the CI tag guard fails the build on purpose.

## The moving parts

| File | Role |
| --- | --- |
| `cli-tool/package.json` | Single source of truth for the version + npm metadata (name, bin, repository, homepage, bugs) |
| `cli-tool/config.json` | GitHub URL, website URL, asset names (`xommit.exe` / `xommit` / `xommit-macos`) |
| `cli-tool/scripts/build.js` | Compiles the C++ and bakes in version/URLs as `-D` macros |
| `cli-tool/scripts/index.js` | npm shim: on first run it downloads the matching binary, then execs it |
| `cli-tool/scripts/release.js` | Release helper: bump version, commit, tag |
| `cli-tool/include/xommit/commands.h` | Implements `--version`, `--update`, `--connect`, `--reset` |
| `cli-tool/scripts/install.sh` / `install.ps1` | The curl / PowerShell one-line installers |
| `.github/workflows/release.yml` | CI pipeline that builds, releases and publishes |
| npm "Trusted publisher" setting | Lets GitHub Actions publish with no stored token (OIDC) |

## What happens, in order

### A. On your machine (cutting the release)

```bash
cd cli-tool
npm run release -- patch
```

`release.js` does, in order:

1. Reads `package.json` and computes the next version (`1.0.1` → `1.0.2`, or `minor` / `major` / an explicit `x.y.z`).
2. Writes the new version back into `package.json`.
3. `git rev-parse --show-toplevel` to find the repo root (the package lives in a subfolder).
4. `git add cli-tool/package.json`
5. `git commit -m "chore(release): v1.0.2"`
6. `git tag -a v1.0.2 -m "v1.0.2"` (annotated tag)
7. Prints the two push commands.

Then push:

```bash
git push origin master
git push origin v1.0.2
```

> **Why not just `npm version patch`?**
> npm's `@npmcli/git.is()` only does `stat(cwd/.git)`. The real `.git` is at the repo root, but the package is in `cli-tool/`, so npm decides "not a git repo" and silently skips the commit **and** the tag. It only bumps the file. `release.js` sidesteps that.

### B. On GitHub (the tag triggers CI)

Pushing the tag fires `release.yml` (`on: push: tags: v*`). It has two jobs.

**`build` job** — runs on Windows, Ubuntu and macOS:

1. Checks out the repo.
2. **Version guard**: reads `cli-tool/package.json` and compares it to the tag. Mismatch → hard fail. This is what stops a bad tag from shipping.
3. Ubuntu: installs `libcurl` headers (needed to link).
4. `node scripts/build.js --release` → produces, per OS:
   - Windows: `bin/xommit.exe` (links `-lwininet`, `-static`)
   - Linux: `bin/xommit` (links `-lcurl`)
   - macOS: `bin/xommit-macos` (links `-lcurl`)
5. Reads `config.json` to know the binary/asset name for that OS, and uploads it as a build artifact.

**`release` job** — runs once on Ubuntu:

1. Checks out the repo and sets up Node 24.
2. Downloads all three artifacts.
3. Creates the GitHub Release `v1.0.2` and attaches the three binaries.
4. `npm publish` from `cli-tool/`. **No token is configured.** GitHub mints a short-lived OIDC token (that's why the workflow has `id-token: write`), and the npm CLI exchanges it with npmjs for a one-shot publish token.

### C. On npm

`npm publish` uploads a tiny tarball containing only:

- `package.json`, `config.json`, `scripts/index.js`, and `README.md`

No binaries go to npm. The package is a ~2 KB shim plus metadata.

### D. On an end user's machine

Three ways in:

- **npm** — `npm install -g @abhraneeldhar7/xommit` installs the shim. On first `xommit` run, `scripts/index.js` computes `github + "/releases/download/v" + pkg.version`, downloads the OS-correct binary from that exact tag, `chmod 755`s it on Unix, then forwards your args. Version-matched: npm `1.0.2` always fetches the `v1.0.2` binary.
- **curl / PowerShell** — `install.sh` / `install.ps1` download the binary from `releases/latest` into `/usr/local/bin` or `%LOCALAPPDATA%\Programs\xommit`. No Node required.
- **`xommit --update`** — the compiled binary downloads `releases/latest`, size-checks it, fixes permissions on Unix, renames itself out of the way and slots the new one in.

## Command reference

**Developer / release**

```bash
npm run release -- patch        # bump + commit + tag (run inside cli-tool)
git push origin master          # push the release commit
git push origin v1.0.2          # push the tag -> triggers CI
```

**CI does automatically**

- tag guard → build 3 binaries → GitHub Release + assets → `npm publish` via OIDC.

**End user**

```bash
npm install -g @abhraneeldhar7/xommit   # or curl / PowerShell
xommit --connect                        # paste Groq key once
git add .
xommit "fixed the login bug"            # generate + commit
xommit --update                         # self-update to latest
xommit --version                        # prints baked-in version
```

## What "trusted publishing / OIDC" means

**Old way:** create an npm token, store it as a repo secret (`NPM_TOKEN`), CI sends it. Tokens expire, can leak, and bypass-2FA tokens are being retired (Jan 2027).

**New way (what this repo uses):**

1. On npmjs the package is told to trust publishes that come from repo `abhraneeldhar7/xommit`, workflow `release.yml`.
2. During that job, GitHub Actions can request a signed OIDC token proving "I am `release.yml` in `xommit`". That requires `id-token: write` in the workflow.
3. The npm CLI detects it and exchanges it for a publish grant.

Nothing long-lived to rotate, and `NPM_TOKEN` can be deleted.

## Failure cheat sheet

| Symptom | Meaning |
| --- | --- |
| `Check tag matches package version` fails | You tagged `vX` but `cli-tool/package.json` isn't `X`. Use the release script so both move together. |
| npm step: `EOTP` | A token was used that couldn't bypass 2FA. Shouldn't happen with OIDC. |
| npm step: `E404 / Not found` on PUT | Wrong token permissions/scope (or a stale local session). |
| npm step: `cannot publish over previously published version` | That version already exists on npm — bump again. |
| `xommit --update` says "looks invalid" | The size floor in `commands.h` is above the real binary size. |
| npm install works but binary download 404s | The GitHub Release for that exact `vX.Y.Z` doesn't exist yet. |
