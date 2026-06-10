# Download and install the ready-to-use plugin ZIP

This repository now includes a GitHub Actions workflow that builds a downloadable plugin ZIP for you. You do **not** need to run CMake or compile code on your own computer if you use that ZIP.

## What to click in GitHub

1. Open this repository on GitHub.
2. Click the **Actions** tab.
3. Click **Build downloadable OBS plugin** on the left.
4. Click the newest successful run with a green check mark.
5. Scroll to **Artifacts**.
6. Download the artifact for your computer:
   - Apple Silicon Mac: `obs-rtmp-receiver-macos-arm64`
   - Linux PC: `obs-rtmp-receiver-linux-x86_64`
6. Download `obs-rtmp-receiver-linux-x86_64`.
7. Unzip the downloaded file.
8. Follow the `INSTALL.txt` file inside the ZIP.

If the project owner creates a GitHub Release, the same ZIP is attached directly to that release so you can download it from the **Releases** page instead.

## Important limitations

- OBS plugins are operating-system specific. A Linux plugin file will not work on macOS, and a Mac plugin file will not work on Linux.
- Apple Silicon Macs need the **macos-arm64** ZIP.
- Intel Macs are not covered by the Apple Silicon ZIP. They would need a separate macOS x86_64 build.
- Windows packages require a separate Windows build job.
- The plugin still requires FFmpeg on the OBS computer. Install FFmpeg first, then set the plugin's **FFmpeg executable** field if OBS cannot find `ffmpeg` automatically.

## Apple Silicon Mac instructions

If your Mac has an M1, M2, M3, or M4 chip, follow `docs/MAC_APPLE_SILICON_INSTALL.md`.

- OBS plugins are operating-system specific. A Linux plugin file will not work on Windows or macOS.
- The first automated package in this repository is for **Linux x86_64** because GitHub's Linux runner can install the OBS development package directly.
- Windows and macOS packages require additional OBS SDK/dependency setup and should be added as separate release jobs.
- The plugin still requires FFmpeg on the OBS computer. Install FFmpeg first, then set the plugin's **FFmpeg executable** field if OBS cannot find `ffmpeg` automatically.

## If you are on Windows right now

Until a Windows GitHub Actions job is added, use the no-code FFmpeg workflow in `docs/NON_CODER_SETUP.md`. That workflow is also completely free and lets you receive an RTMP feed in OBS today without compiling anything.
