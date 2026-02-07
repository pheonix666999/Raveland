# RaveLand (Raveland) - Build, Install, Distribute

This repo builds a JUCE audio plugin in these formats:
- macOS: Standalone `.app`, VST3 `.vst3`, Audio Unit (AU) `.component`
- Windows: Standalone `.exe`, VST3 `.vst3` (AU does not exist on Windows)

If you "can't find the AU": you are likely looking at a Windows build output. The AU (`.component`) is macOS-only.

## Where the built files are

After building, artefacts are written to:
- `build/Raveland_artefacts/Release/Standalone/`
- `build/Raveland_artefacts/Release/VST3/`
- `build/Raveland_artefacts/Release/AU/` (macOS only)

Expected bundles (macOS):
- `build/Raveland_artefacts/Release/Standalone/RaveLand.app`
- `build/Raveland_artefacts/Release/VST3/RaveLand.vst3`
- `build/Raveland_artefacts/Release/AU/RaveLand.component`

## The "damaged file" problem (important)

macOS "RaveLand.vst3 is damaged and can't be opened" is Gatekeeper. This is fixed for customers by:
- Signing with a Developer ID certificate
- Notarizing with Apple
- (Recommended) Stapling the notarization ticket

Windows "unknown publisher / blocked" warnings are fixed for customers by:
- Code-signing the `.exe` and the VST3 binary with an Authenticode certificate

This repo contains scripts + a GitHub Actions pipeline that can build installers and (optionally) sign/notarize when you provide the certificates as secrets.
When `REQUIRE_NOTARIZATION=1` is set (CI does this on push), the build will fail if notarization credentials are missing, preventing accidental distribution of unsigned builds.

## Customer delivery (what you should ship)

GitHub Actions uploads two artifacts on every push:

**macOS (`Raveland-macOS`)**
- Always: `dist/RaveLand-macOS-unsigned.zip` and `dist/RaveLand-Installer-unsigned.pkg` (internal testing only)
- If signing/notarization secrets are set: `dist/RaveLand-macOS.zip` and `dist/RaveLand-Installer.pkg` (recommended for customers)

**Windows (`Raveland-Windows`)**
- Always: `dist/RaveLand-windows.zip` (drag/drop)
- Always: `dist/RaveLand-Installer-Windows.exe` (double-click installer)
- If Windows signing secrets are set: the installer + binaries are code-signed (recommended)

## macOS - install locations (manual)

**Audio Unit (AU)**
- System-wide: `/Library/Audio/Plug-Ins/Components/`
- Per-user: `~/Library/Audio/Plug-Ins/Components/`

**VST3**
- System-wide: `/Library/Audio/Plug-Ins/VST3/`
- Per-user: `~/Library/Audio/Plug-Ins/VST3/`

After copying, restart your DAW and rescan plugins.

## Sample library location
The plugin scans for sample stacks in these locations (first match wins):
- macOS: `/Library/Application Support/NS Audio/RaveLand/Samples`
- Windows: `C:\ProgramData\NS Audio\RaveLand\Samples`
- Next to the plugin executable: `Samples`
- Override: set `RAVELAND_SAMPLE_PATH`

Each subfolder inside `Samples` is treated as a selectable stack in the layer dropdown.

## macOS - build from source (developer)

Prereqs:
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake`

Build (universal: Intel + Apple Silicon):
```bash
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build --config Release --target Raveland_Standalone Raveland_VST3 Raveland_AU
```

Notes:
- AU is only enabled on Apple platforms (see `CMakeLists.txt`).
- Default deployment target is `10.15` unless you override `CMAKE_OSX_DEPLOYMENT_TARGET`.

## Windows - build from source

Prereqs:
- Visual Studio 2022 (MSVC)
- CMake

Build:
```powershell
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_BUILD_EXAMPLES=OFF -DJUCE_BUILD_EXTRAS=OFF
cmake --build build --config Release --parallel
```

## CI secrets (to remove Gatekeeper/SmartScreen warnings)

**macOS signing/notarization**
- `MACOS_CERT_P12_BASE64`
- `MACOS_CERT_PASSWORD`
- `MACOS_SIGNING_IDENTITY` (Developer ID Application identity)
- `MACOS_INSTALLER_IDENTITY` (Developer ID Installer identity, optional but recommended)
- `APPLE_NOTARY_KEY_BASE64`
- `APPLE_NOTARY_KEY_ID`
- `APPLE_NOTARY_ISSUER_ID`

**Windows signing (optional but recommended)**
- `WINDOWS_CERT_PFX_BASE64`
- `WINDOWS_CERT_PASSWORD`

## Local testing workaround (not for customers)

If you must test an unsigned download on macOS, you can remove quarantine attributes:
- `bash scripts/macos_clear_quarantine.sh /path/to/RaveLand.vst3`

## About the UI

`demo/` contains an HTML prototype used for design iteration. The actual plugin UI is implemented in `source/`.
