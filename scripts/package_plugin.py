#!/usr/bin/env python3
"""Create a user-downloadable OBS plugin ZIP from a CMake install folder."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import zipfile

PLUGIN_NAME = "obs-rtmp-receiver"
PLUGIN_SUFFIXES = (".plugin", ".so", ".dll", ".dylib")


def find_plugin_binary(package_root: Path) -> Path:
    candidates: list[Path] = []
    for suffix in PLUGIN_SUFFIXES:
        candidates.extend(package_root.rglob(f"{PLUGIN_NAME}{suffix}"))

    if not candidates:
        raise SystemExit(f"Could not find a built {PLUGIN_NAME} plugin binary under {package_root}")

    # Prefer bundle directories (macOS .plugin) over files if both are present.
    candidates.sort(key=lambda path: (path.suffix != ".plugin", str(path)))
    return candidates[0]


def copy_if_exists(source: Path, destination: Path) -> None:
    if source.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def install_instructions(platform: str) -> str:
    if platform == "macos-arm64":
        return """INSTALL ON MAC (APPLE SILICON / M1-M4)
1. Close OBS.
2. Open this ZIP. It contains an `obs-rtmp-receiver` folder.
3. In Finder, press Shift+Command+G and paste:
   ~/Library/Application Support/obs-studio/plugins/
4. If the `plugins` folder does not exist, create it.
5. Copy the whole `obs-rtmp-receiver` folder into that `plugins` folder.
6. The final plugin bundle should be here:
   ~/Library/Application Support/obs-studio/plugins/obs-rtmp-receiver/bin/obs-rtmp-receiver.plugin
7. Reopen OBS.
8. Add a source named "RTMP Receiver (FFmpeg)".
"""

    return """INSTALL ON LINUX
1. Close OBS.
2. Open this ZIP. It contains an `obs-rtmp-receiver` folder.
3. Copy that whole `obs-rtmp-receiver` folder to:
   ~/.config/obs-studio/plugins/
4. The final plugin file should be here:
   ~/.config/obs-studio/plugins/obs-rtmp-receiver/bin/64bit/obs-rtmp-receiver.so
5. Reopen OBS.
6. Add a source named "RTMP Receiver (FFmpeg)".
"""


def write_install_notes(staging_root: Path, platform: str) -> None:
    notes = f"""OBS RTMP Receiver Plugin ({platform})

WHAT THIS ZIP IS
This ZIP contains the compiled OBS plugin binary created by GitHub Actions.
It still requires FFmpeg to be installed on the computer running OBS.

{install_instructions(platform)}
IMPORTANT
- If OBS does not show the source, check OBS logs for missing libraries.
- The plugin starts FFmpeg for you, but FFmpeg must be installed and available as `ffmpeg` on PATH, or configured in the source properties.
- For internet guests, you still need firewall/router port forwarding for your RTMP port.

See README.md and docs/NON_CODER_SETUP.md in the project for detailed usage instructions.
"""
    (staging_root / "INSTALL.txt").write_text(notes, encoding="utf-8")


def make_zip(source_dir: Path, zip_path: Path) -> None:
    zip_path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(source_dir.rglob("*")):
            if path.is_file():
                archive.write(path, path.relative_to(source_dir))


def copy_plugin(plugin_binary: Path, binary_destination: Path) -> None:
    binary_destination.parent.mkdir(parents=True, exist_ok=True)
    if plugin_binary.is_dir():
        if binary_destination.exists():
            shutil.rmtree(binary_destination)
        shutil.copytree(plugin_binary, binary_destination)
    else:
        shutil.copy2(plugin_binary, binary_destination)


def plugin_destination(staging_root: Path, plugin_binary: Path) -> Path:
    # Package in OBS's per-user plugin layout:
    #   macOS: <plugin-name>/bin/<plugin>.plugin
    #   Linux/Windows: <plugin-name>/bin/64bit/<binary>
    if plugin_binary.suffix == ".plugin":
        return staging_root / PLUGIN_NAME / "bin" / plugin_binary.name
    if plugin_binary.suffix in {".so", ".dll"}:
        return staging_root / PLUGIN_NAME / "bin" / "64bit" / plugin_binary.name
    return staging_root / PLUGIN_NAME / "bin" / plugin_binary.name


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--package-root", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--platform", required=True)
    args = parser.parse_args()

    package_root = args.package_root.resolve()
    output_dir = args.output_dir.resolve()
    staging_root = output_dir / f"{PLUGIN_NAME}-{args.platform}"

    if staging_root.exists():
        shutil.rmtree(staging_root)
    staging_root.mkdir(parents=True)

    plugin_binary = find_plugin_binary(package_root)
    copy_plugin(plugin_binary, plugin_destination(staging_root, plugin_binary))

    copy_if_exists(Path("README.md"), staging_root / "README.md")
    copy_if_exists(Path("LICENSE"), staging_root / "LICENSE")
    copy_if_exists(Path("docs/NON_CODER_SETUP.md"), staging_root / "docs" / "NON_CODER_SETUP.md")
    copy_if_exists(Path("docs/DOWNLOAD_PLUGIN_FROM_GITHUB.md"), staging_root / "docs" / "DOWNLOAD_PLUGIN_FROM_GITHUB.md")
    copy_if_exists(Path("docs/MAC_APPLE_SILICON_INSTALL.md"), staging_root / "docs" / "MAC_APPLE_SILICON_INSTALL.md")
    write_install_notes(staging_root, args.platform)

    zip_path = output_dir / f"{PLUGIN_NAME}-{args.platform}.zip"
    if zip_path.exists():
        zip_path.unlink()
    make_zip(staging_root, zip_path)
    print(f"Created {zip_path}")


if __name__ == "__main__":
    main()
