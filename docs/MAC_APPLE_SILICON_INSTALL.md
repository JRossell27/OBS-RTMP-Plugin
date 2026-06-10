# Mac Apple Silicon install guide

Use this guide if your Mac has an Apple Silicon chip, such as an M1, M2, M3, or M4.

## Download the Mac plugin ZIP from GitHub

1. Open this repository on GitHub.
2. Click **Releases**.
3. Open **Latest Apple Silicon OBS RTMP Receiver Plugin**.
4. Download `obs-rtmp-receiver-macos-arm64.zip`.
5. Unzip the downloaded file.
6. Open `INSTALL.txt` inside the ZIP and follow it.

If the release is not there yet, click **Actions**, run **Apple Silicon Plugin Release**, wait for the green check mark, and then return to **Releases**.
If the release is not there yet, click **Actions**, run **Build Mac Apple Silicon OBS plugin**, wait for the green check mark, and then return to **Releases**.
2. Click **Actions**.
3. Click **Build downloadable OBS plugin**.
4. Open the newest successful run with a green check mark.
5. Under **Artifacts**, download `obs-rtmp-receiver-macos-arm64`.
6. Unzip the downloaded file.
7. Open `INSTALL.txt` inside the ZIP and follow it.

## Install the plugin on your Mac

1. Close OBS.
2. Open the unzipped download. It contains an `obs-rtmp-receiver` folder.
3. In Finder, press **Shift+Command+G**.
4. Paste this path and press **Return**:

```text
~/Library/Application Support/obs-studio/plugins/
```

5. If the `plugins` folder does not exist, create it.
6. Copy the whole `obs-rtmp-receiver` folder into that `plugins` folder.
7. The final plugin bundle should be here:

```text
~/Library/Application Support/obs-studio/plugins/obs-rtmp-receiver/bin/obs-rtmp-receiver.plugin
```

8. Reopen OBS.
9. Add a source named **RTMP Receiver (FFmpeg)**.

## Install FFmpeg on Mac

The plugin starts FFmpeg for you, but FFmpeg must still be installed on your Mac.

The easiest free method is Homebrew:

```bash
brew install ffmpeg
```

After installing, you can test it in Terminal:

```bash
ffmpeg -version
```

If OBS cannot find FFmpeg automatically, open the RTMP Receiver source properties and set **FFmpeg executable** to the full path shown by:

```bash
which ffmpeg
```

On Apple Silicon Homebrew, that path is often:

```text
/opt/homebrew/bin/ffmpeg
```

## If macOS blocks the plugin

The GitHub Actions build ad-hoc signs the plugin bundle. If macOS still blocks it, open **System Settings** → **Privacy & Security** and allow the plugin/OBS item shown there, then reopen OBS.
