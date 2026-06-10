# Download and install the ready-to-use plugin ZIP

This repository includes a GitHub Actions workflow that builds a downloadable Apple Silicon macOS plugin ZIP for you. You do **not** need to run CMake or compile code on your own computer if you use that ZIP.

## Best option: download from Releases

1. Open this repository on GitHub.
2. Click **Releases** on the right side of the repository page.
3. Open **Latest Apple Silicon OBS RTMP Receiver Plugin**.
4. Download the release asset named `obs-rtmp-receiver-macos-arm64.zip`.
5. Unzip the downloaded file.
6. Follow the `INSTALL.txt` file inside the ZIP.

The release workflow publishes the ZIP as the repository's latest release so it appears on the main **Releases** page.

## If you just ran the action and do not see the release yet

1. Open the **Actions** tab.
2. Click **Apple Silicon Plugin Release** on the left.
3. Open the newest successful run with a green check mark.
4. Check the run summary for the **Plugin download** section. It contains:
   - a direct link to the release page, and
   - a direct link to the workflow artifact.
5. If GitHub is still refreshing the release page, use the workflow artifact link from the successful run.

The workflow artifact is a backup download location. The release asset is the preferred download because it is easier to find later.

## Important limitations

- This repository currently builds only the Apple Silicon Mac ZIP because that is the target platform for this project right now.
- Apple Silicon Macs need the **macos-arm64** ZIP.
- Intel Macs, Windows, and Linux are intentionally not built by the current workflow.
- OBS plugins are operating-system specific. A macOS Apple Silicon plugin will not work on Intel macOS, Windows, or Linux.
- The plugin still requires FFmpeg on the OBS computer. Install FFmpeg first, then set the plugin's **FFmpeg executable** field if OBS cannot find `ffmpeg` automatically.

## Apple Silicon Mac instructions

If your Mac has an M1, M2, M3, M4, or newer Apple Silicon chip, follow `docs/MAC_APPLE_SILICON_INSTALL.md`.

## If you are not on Apple Silicon Mac

Use the no-code FFmpeg workflow in `docs/NON_CODER_SETUP.md`. That workflow is also completely free and lets you receive an RTMP feed in OBS today without compiling anything.
