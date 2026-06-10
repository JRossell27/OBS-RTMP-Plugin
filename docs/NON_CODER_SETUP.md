# Non-coder setup guide

This project is source code for a free OBS plugin. Source code is not something you can double-click in OBS yet: it must be **built** into a plugin file for your operating system first.

If you are not comfortable building software, start with the "Use it today without building the plugin" section below. It uses the same free FFmpeg technology that the plugin automates.

## What this plugin is supposed to do

The goal is:

1. You create an RTMP address and stream key.
2. A guest, camera operator, phone app, hardware encoder, or another OBS user streams to that address/key.
3. The feed appears inside your OBS scene.
4. You then stream or record your final OBS program normally.

The plugin in this repository automates steps 1-3 inside OBS, but it still needs to be compiled and installed before OBS can load it.

## Use it today without building the plugin

This is the quickest completely-free path if you need to test the workflow now.

### 1. Install FFmpeg

Download and install FFmpeg for your operating system. After installation, open a terminal/command prompt and run:

```bash
ffmpeg -version
```

If the command prints version information, FFmpeg is ready.

### 2. Choose your settings

Use these example settings first:

- RTMP port: `1935`
- RTMP app/path: `live`
- Stream key: `test1234`
- Local OBS relay port: `10000`

Your contributor will stream to:

- Server URL: `rtmp://YOUR_PUBLIC_OR_LAN_IP:1935/live`
- Stream Key: `test1234`

If the contributor app only accepts one full URL, give them:

```text
rtmp://YOUR_PUBLIC_OR_LAN_IP:1935/live/test1234
```

For a same-computer test, use `127.0.0.1` instead of `YOUR_PUBLIC_OR_LAN_IP`.

### 3. Start the free RTMP listener

Run this command on the computer running OBS:

```bash
ffmpeg -hide_banner -loglevel warning -listen 1 -i rtmp://0.0.0.0:1935/live/test1234 -map 0 -c copy -f mpegts "udp://127.0.0.1:10000?fifo_size=1000000&overrun_nonfatal=1"
```

Leave this window open. It is the temporary RTMP server.

### 4. Add the feed to OBS

In OBS:

1. Click **Sources** → **+**.
2. Choose **Media Source**.
3. Uncheck **Local File**.
4. In **Input**, paste:

```text
udp://127.0.0.1:10000?fifo_size=1000000&overrun_nonfatal=1
```

5. Click **OK**.

When someone streams to your RTMP address/key, the video should appear in that Media Source.

### 5. Send your final show to social channels

Once the incoming feed is visible in OBS, use OBS normally:

1. Add overlays, cameras, microphones, scenes, and graphics.
2. Configure your destination in **Settings** → **Stream**.
3. Click **Start Streaming** to send your final OBS program to YouTube, Facebook, Twitch, or another platform.

## How to install the actual plugin later

To use the plugin instead of the manual FFmpeg command, someone needs to build this repository into an OBS plugin binary.

### What you need

- OBS Studio installed.
- FFmpeg installed.
- CMake installed.
- A C++ compiler for your operating system.
- OBS development files / `libobs` CMake package.

### Build commands

From this repository folder, a developer would run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix <your-obs-install-folder>
```

If CMake says it cannot find `libobs`, that means the OBS development files are not installed or CMake was not told where they are.

## Router/firewall notes

For someone outside your home or studio network to connect:

1. Your OBS computer must be turned on and running the listener/plugin.
2. Your firewall must allow inbound TCP traffic on the RTMP port, usually `1935`.
3. Your router must port-forward TCP `1935` to the OBS computer.
4. You must give the contributor your public IP address or DNS name.

For guests on the same local network, use the OBS computer's LAN IP address instead and no internet port-forwarding is usually required.

## Security notes

RTMP is not encrypted, and the stream key is the main secret. Use a long random stream key for real events, do not post it publicly, and change it after each event if you shared it with outside contributors.
