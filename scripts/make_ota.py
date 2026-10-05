#!/usr/bin/env python3
from pathlib import Path
import hashlib, json, shutil, sys

ROOT = Path(__file__).resolve().parents[1]
VERSION = "0.1.0"
REPO = "syschelle/esp8266-moisture-sensor--for-growtend-backend"
BIN = ROOT / ".pio" / "build" / "d1_mini" / "firmware.bin"
OUT = ROOT / "ota-dist"

if not BIN.exists():
    raise SystemExit(f"Missing {BIN}. Run: pio run -e d1_mini")

OUT.mkdir(exist_ok=True)
target = OUT / "firmware.bin"
shutil.copy2(BIN, target)
data = target.read_bytes()
sha = hashlib.sha256(data).hexdigest()
(OUT / "firmware.bin.sha256").write_text(f"{sha}  firmware.bin\n", encoding="utf-8")
manifest = {
    "version": VERSION,
    "url": f"https://raw.githubusercontent.com/{REPO}/ota/firmware.bin",
    "size": len(data),
    "sha256": sha
}
(OUT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
(OUT / "README.md").write_text(
    f"# OTA channel\n\nFirmware: v{VERSION}\n\nGenerated from the main source tree.\n",
    encoding="utf-8"
)
print(f"OTA files created in {OUT}")
print(f"SHA-256: {sha}")
