# OBS RTMP Receiver Plugin

A free, open-source OBS source plugin that lets OBS receive an RTMP contribution feed from an outside encoder and use it like any other OBS source.

The plugin creates a source named **RTMP Receiver (FFmpeg)**. When the source becomes active it starts FFmpeg in RTMP listen mode, generates a server URL and stream key, and relays the received feed into OBS through a private OBS Media Source.

## What it does

- Hosts a local RTMP ingest endpoint from inside OBS.
- Generates a stream key you can give to a remote contributor.
- Shows both the split **Server URL + Stream Key** and a combined full RTMP URL.
- Receives the outside feed in OBS so you can mix it into scenes and stream onward to social channels.
- Uses only free/open-source components: OBS Studio and FFmpeg.

## Downloadable plugin ZIP

This repository includes a GitHub Actions workflow that builds a downloadable plugin ZIP. If you do not want to compile anything yourself, open the repository on GitHub, go to **Actions**, run or open **Build downloadable OBS plugin**, and download the artifact from a successful run.

Step-by-step download instructions are in [`docs/DOWNLOAD_PLUGIN_FROM_GITHUB.md`](docs/DOWNLOAD_PLUGIN_FROM_GITHUB.md).

## New to plugins or coding?

If you are not used to building OBS plugins, start with the non-coder guide. It explains what this repository is, how to test the same RTMP receiving workflow immediately with FFmpeg, and what still has to happen before OBS can load this code as an installed plugin.

See: [`docs/NON_CODER_SETUP.md`](docs/NON_CODER_SETUP.md)

## Requirements

- OBS Studio with plugin development headers available at build time.
- FFmpeg installed on the OBS machine and available as `ffmpeg` on `PATH`, or a full path configured in the source properties.
- Network access from the outside encoder to the OBS computer.
  - For LAN use, set **Public host / LAN IP** to the OBS computer's LAN IP.
  - For internet use, forward the configured RTMP port on your router/firewall to the OBS computer.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix <your-obs-install-prefix>
```

Depending on your OBS packaging, `libobs` may require `CMAKE_PREFIX_PATH` or `libobs_DIR` to point at the OBS development package.

## Use in OBS

1. Install the built plugin into OBS and restart OBS.
2. Add a source: **RTMP Receiver (FFmpeg)**.
3. Set **Public host / LAN IP to give streamers** to the address the outside encoder can reach.
4. Leave **RTMP port** at `1935` or choose another open port.
5. Click **Generate new stream key** if you want a fresh key.
6. Give the outside encoder:
   - **Server URL to give streamer**, for example `rtmp://203.0.113.25:1935/live`
   - **Stream key**, for example `d2a1...`
7. When the source is active in a scene, the plugin listens for the incoming RTMP feed and displays it in OBS.

## Example contributor settings

For OBS, Streamlabs, hardware encoders, or similar RTMP senders:

- Service: Custom
- Server: value from **Server URL to give streamer**
- Stream Key: value from **Stream key**

If the sender only accepts a single URL, use **Full RTMP URL**.

## Security notes

RTMP is not encrypted and the stream key is the only built-in secret. For public internet ingest, use a strong generated stream key, only open the required port, and regenerate the key after each event when appropriate.
