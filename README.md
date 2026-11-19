# Wood Body ONE

One-knob acoustic guitar enhancer (VST3 / AU). Made for cheap acoustic guitars recorded with a condenser mic: it adds a thick, woody body and tames the thin, harsh top end.

**Developed by Ewin · Made with Claude**

## The WOOD knob
One knob drives five processes at once (shown live on the MACRO display):

| Process | What it does |
|---|---|
| Body IR | Built-in synthetic acoustic body impulse response (convolution) |
| Warmth | Low-mid boost around 250 Hz |
| Grain | Woody saturation on the low-mids (150–600 Hz), even harmonics; starts after ~25% |
| De-harsh | Dynamic cut around 3.2 kHz, only on loud peaks |
| Attack | Transient shaper; rises to the middle of the knob, then softens |

Stages: 0 → Subtle → Warm → Thick → Huge.

Other controls: **Wood type** (Spruce / Mahogany / Rosewood), **Stereo** (Mono / X-Y / Spaced), **Output**, **Tips** (hover help on/off). The window is resizable.

## Download
Builds are produced by GitHub Actions on every push:
- Windows: `WoodBodyONE-Windows-VST3.zip`
- macOS: `WoodBodyONE-macOS-AU.zip`, `WoodBodyONE-macOS-VST3.zip`

Tagged versions (`v*`) are published on the Releases page.

## Install
- **Windows (VST3):** unzip and copy `Wood Body ONE.vst3` to `C:\Program Files\Common Files\VST3\`, then rescan plugins in your DAW.
- **macOS (AU):** copy `Wood Body ONE.component` to `~/Library/Audio/Plug-Ins/Components/`.
- **macOS (VST3):** copy `Wood Body ONE.vst3` to `~/Library/Audio/Plug-Ins/VST3/`.

The macOS builds are ad-hoc signed, not notarized. If macOS blocks the plugin, run:
`xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"Wood Body ONE.component"`

## Build from source
```
cmake -B build
cmake --build build --config Release
```
JUCE 8 is fetched automatically.
