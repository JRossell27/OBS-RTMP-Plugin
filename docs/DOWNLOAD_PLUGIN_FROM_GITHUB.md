# Download and install the ready-to-use plugin ZIP

This repository now includes a GitHub Actions workflow that builds a downloadable plugin ZIP for you. You do **not** need to run CMake or compile code on your own computer if you use that ZIP.

## What to click in GitHub

1. Open this repository on GitHub.
2. Click **Releases** on the right side of the repository page.
3. Open **Latest Apple Silicon OBS RTMP Receiver Plugin**.
4. Download `obs-rtmp-receiver-macos-arm64.zip` from the release assets.
5. Unzip the downloaded file.
6. Follow the `INSTALL.txt` file inside the ZIP.

If you do not see the release yet, click the **Actions** tab, run **Apple Silicon Plugin Release**, wait for the green check mark, then return to **Releases**. The workflow publishes the ZIP there automatically.

## Important limitations

- This repository currently builds only the Apple Silicon Mac ZIP because that is the target platform for this project right now.
- Apple Silicon Macs need the **macos-arm64** ZIP.
- Intel Macs, Windows, and Linux are intentionally not built by the current workflow.
- The plugin still requires FFmpeg on the OBS computer. Install FFmpeg first, then set the plugin's **FFmpeg executable** field if OBS cannot find `ffmpeg` automatically.

## Apple Silicon Mac instructions

If your Mac has an M1, M2, M3, or M4 chip, follow `docs/MAC_APPLE_SILICON_INSTALL.md`.

## If you are not on Apple Silicon Mac

Use the no-code FFmpeg workflow in `docs/NON_CODER_SETUP.md`. That workflow is also completely free and lets you receive an RTMP feed in OBS today without compiling anything.
