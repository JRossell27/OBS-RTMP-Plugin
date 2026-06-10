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


## If OBS says the plugin failed to load

If OBS shows this startup warning:

```text
The following OBS plugins failed to load:

obs-rtmp-receiver
obs-rtmp-receiver
```

Start by removing duplicate copies. Seeing the same plugin name twice usually means OBS found more than one installed copy, such as one copy in your user plugin folder and another copy inside the OBS app/plugin folder, or an accidentally nested duplicate from unzipping/copying more than once.

1. Close OBS.
2. In Finder, press **Shift+Command+G** and inspect this folder:

```text
~/Library/Application Support/obs-studio/plugins/
```

3. Keep only one `obs-rtmp-receiver` folder there. The only expected user-plugin path is:

```text
~/Library/Application Support/obs-studio/plugins/obs-rtmp-receiver/bin/obs-rtmp-receiver.plugin
```

4. Also check that you did not create a nested duplicate like:

```text
~/Library/Application Support/obs-studio/plugins/obs-rtmp-receiver/obs-rtmp-receiver/bin/obs-rtmp-receiver.plugin
```

5. If you also copied `obs-rtmp-receiver.plugin` into OBS.app itself, remove that extra copy and use the per-user plugin folder above instead.
6. Reopen OBS and check **Help** → **Log Files** → **View Current Log**. Search the log for `obs-rtmp-receiver`; the lines near that name usually say whether macOS blocked the plugin, the architecture is wrong, or a linked library could not be loaded.

Other common causes:

- The downloadable ZIP is for Apple Silicon only. It will not load in Intel OBS or on an Intel Mac.
- Make sure you are running the normal Apple Silicon OBS app, not an Intel/Rosetta copy.
- If macOS quarantined or blocked the plugin, open **System Settings** → **Privacy & Security**, allow the blocked OBS/plugin item, and restart OBS.

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
