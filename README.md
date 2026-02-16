<div align="center">

# raviz

<img src="assets/logo.png" width="110" height="110" alt="Raviz Logo">

### High-performance audio-reactive 3D visualizer for Linux.

Renders a glowing, deformable 3D sphere that reacts to system audio in real time. Runs in a transparent, borderless window using OpenGL 3.3+.

[![License](https://img.shields.io/github/license/maskedsyntax/raviz)](https://github.com/maskedsyntax/raviz/blob/master/LICENSE)
[![Release](https://img.shields.io/github/v/release/maskedsyntax/raviz)](https://github.com/maskedsyntax/raviz/releases)

</div>

## Features

- **Transparent Window**: Floats on top of your desktop (X11 & Wayland).
- **Audio Reactive**: Automatically detects system audio output (PulseAudio).
- **3D Visuals**: Deformable sphere with Fresnel glow, reacting to bass and mid frequencies.
- **Configurable**: TOML configuration file and CLI arguments.
- **Low Latency**: Dedicated audio processing thread.
- **Efficient**: Native C implementation with OpenGL 3.3 Core.

## Installation

### Binary Packages (Debian/Ubuntu/Fedora)
Download the latest `.deb` or `.rpm` release from the [Releases Page](https://github.com/maskedsyntax/raviz/releases).

**Debian/Ubuntu:**
```bash
sudo apt install ./raviz-1.1.1-Linux.deb
```

**Fedora/RHEL:**
```bash
sudo rpm -i raviz-1.1.1-Linux.rpm
```

### Arch Linux
Install manually from the provided `PKGBUILD`:

```bash
git clone https://github.com/maskedsyntax/raviz.git
cd raviz/packaging/aur
makepkg -si
```

### Build from Source

**Dependencies:**
- `cmake`, `make`, `gcc`
- `libpulse-dev` (PulseAudio)
- `libfftw3-dev` (FFTW3)
- `libglfw3-dev` (GLFW3)
- `rpm` (Optional: for building RPMs)

```bash
git clone https://github.com/maskedsyntax/raviz.git
cd raviz
mkdir build && cd build
cmake ..
make
sudo make install
```

## Configuration

Raviz automatically creates a configuration file at `~/.config/raviz/config.toml` on first run.

```toml
[render]
fps = 30
sphere_lat = 40
sphere_lon = 40
rotation_speed = 0.05
window_opacity = 1.0       # 0.0 (Transparent) to 1.0 (Black)
color_mode = "none"        # "none", "static", "reactive"

[audio]
rate = 44100
fft_size = 512
fft_bins = 64
smoothing = 0.15
intensity = 1.0
# device = "alsa_output..." # Optional: Force specific source
```

## Controls

| Key | Action |
| :--- | :--- |
| **ESC** | Quit application |
| **F1** | Cycle Color Mode (None / Static / Reactive) |
| **F4** | Cycle Render Mode (Solid / Wireframe / Points) |
| **F5** | Reload Shaders (Hot-reload `render.c` logic if recompiled, mainly for dev) |
| **UP** | Increase Intensity |
| **DOWN** | Decrease Intensity |

## Usage

Simply run:
```bash
raviz
```

**CLI Overrides:**
- `--opacity <0.0-1.0>`: Set window opacity.
- `--device <name>`: Manually specify PulseAudio source.
- `--fps <int>`: Limit FPS.
- `--intensity <float>`: Reaction multiplier.

## Architecture

- **Main Thread**: Window management, OpenGL rendering, Input handling.
- **Audio Thread**: Audio capture (PulseAudio), FFT processing (FFTW3).
- **Synchronization**: Mutex-protected double buffering for FFT data.
