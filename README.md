# Raveland - Professional Synthesizer Plugin

A cutting-edge synthesizer plugin featuring supersaw oscillators, advanced sample layering, and comprehensive effects processing. Built with JUCE framework for cross-platform compatibility.

## 🎹 Features

### Core Synthesis
- **3 Supersaw Oscillators** with up to 32 voices each
- **Advanced Detuning** capabilities for rich, wide sounds
- **Sample Layer System** supporting multi-sampled instruments
- **Professional Waveform Displays** with oscilloscope-style visualization

### Effects Chain
- **Reverb**: Hall/room modes with size and damping controls
- **Delay**: Configurable time and feedback parameters
- **Chorus**: Rate and depth modulation
- **Distortion**: Drive and tone shaping

### Performance Controls
- **Mono/Legato Modes** with portamento
- **Master Volume** control
- **Preset System** with factory sounds

### Modern UI
- **27 Labeled Controls** with professional rotary knobs
- **Real-time Animations** and visual effects
- **Clean, Modern Design** with contemporary aesthetics
- **Responsive Layout** optimized for all screen sizes

## 🚀 Getting Started

### Prerequisites
- **Windows**: Visual Studio 2019/2022 with MSVC
- **macOS**: Xcode 12+ with Command Line Tools
- **Linux**: GCC 9+ with ALSA/JACK development libraries

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/raveland.git
cd raveland

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release
```

### Installation

#### Windows (VST3)
1. Copy `build/Raveland_artefacts/Release/VST3/RaveLand.vst3/` to your VST3 plugins directory
2. Restart your DAW and scan for new plugins

#### macOS (VST3/AU)
1. Copy `build/Raveland_artefacts/Release/VST3/RaveLand.vst3/` to `/Library/Audio/Plug-Ins/VST3/`
2. Copy `build/Raveland_artefacts/Release/AU/RaveLand.component/` to `/Library/Audio/Plug-Ins/Components/`
3. Restart your DAW

##### Why you don’t see an AU in your screenshot
The folder in your screenshot contains `*.vcxproj`, `*.sln`, `x64/`, etc. That means it is a **Windows Visual Studio build directory**.
**Audio Unit (AU) is macOS-only**, so it will never be produced by a Windows build folder (even if you open that folder on a Mac via sync/zip).

To get the AU, you must build on macOS (Xcode/clang). After a macOS build you should see:
- `build/Raveland_artefacts/Release/AU/RaveLand.component`

##### “Double click to install” for customers (recommended)
For customer-friendly delivery on macOS, ship a signed installer package:
- `dist/RaveLand-Installer.pkg` (created on macOS by CI when signing secrets are configured)

This installs to the standard system locations:
- `/Library/Audio/Plug-Ins/VST3/RaveLand.vst3`
- `/Library/Audio/Plug-Ins/Components/RaveLand.component`

##### macOS Gatekeeper ("damaged file") and FL Studio not loading the VST3
If macOS shows **"RaveLand.vst3 is damaged and can’t be opened"**, that is Gatekeeper blocking the plugin bundle.
This typically happens when the bundle is **not notarized** (or is quarantined due to download).

To prevent this for clients/end-users, you must distribute a **Developer ID signed + notarized** build.

- CI supports optional signing + notarization on macOS (see `.github/workflows/ci.yml` and `scripts/macos_sign_and_notarize.sh`).
- If you are testing locally and understand the implications, you can clear quarantine:
  - `scripts/macos_clear_quarantine.sh`

#### Linux (VST3/LV2)
1. Copy `build/Raveland_artefacts/Release/VST3/RaveLand.vst3/` to your VST3 directory
2. Copy `build/Raveland_artefacts/Release/LV2/RaveLand.lv2/` to your LV2 directory

### Standalone Usage
Run `build/Raveland_artefacts/Release/Standalone/RaveLand.exe` (Windows) or the equivalent executable for your platform.

## 🎛️ Control Reference

### Oscillators (OSC 1, 2, 3)
- **VOICES**: Number of unison voices (1-32)
- **DETUNE**: Detuning amount for chorus effect
- **LEVEL**: Output level for each oscillator

### Sample Layers (Layer A, B, C)
- **GAIN**: Layer volume level
- **START RAND**: Randomization of sample start position

### Effects
- **Reverb**: MIX, SIZE, DAMP
- **Delay**: MIX, TIME (ms), FEEDBACK
- **Chorus**: MIX, RATE (Hz), DEPTH
- **Distortion**: MIX, DRIVE, TONE

### Performance
- **MONO/LEGATO**: Play modes
- **PORTAMENTO**: Glide time between notes
- **MASTER**: Overall output level

## 🔧 Development

### Project Structure
```
raveland/
├── source/                 # C++ source files
│   ├── PluginProcessor.*   # Audio processing logic
│   ├── PluginEditor.*      # UI implementation
│   ├── FancyKnob.*         # Custom rotary controls
│   ├── WaveformDisplay.*   # Oscilloscope visualization
│   └── SampleLayer.*       # Sample management
├── demo/                   # Web prototype
├── extern/JUCE/           # JUCE framework
├── build/                 # Build artifacts
└── .github/workflows/     # CI/CD pipelines
```

### Key Technologies
- **JUCE Framework**: Cross-platform audio plugin development
- **CMake**: Build system configuration
- **C++17**: Modern C++ features
- **Real-time Audio Processing**: Low-latency signal processing

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with the [JUCE framework](https://juce.com/)
- Inspired by professional synthesizers like Serum, Sylenth1, and Virus
- Special thanks to the audio programming community

## 📞 Support

For issues, questions, or contributions:
- Open an [issue](https://github.com/yourusername/raveland/issues) on GitHub
- Check the [discussions](https://github.com/yourusername/raveland/discussions) section

---

**Raveland** - Where modern synthesis meets professional production. 🎵
