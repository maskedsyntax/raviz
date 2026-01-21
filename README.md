# raviz

High-performance audio-reactive 3D visualizer for Linux.

Renders a glowing, deformable 3D sphere that reacts to system audio in real time. Runs in a transparent, borderless window using OpenGL 3.3+.

## Features

- **Transparent Window**: Floats on top of your desktop.
- **Audio Reactive**: Uses PulseAudio to capture system audio.
- **3D Visuals**: Deformable sphere with Fresnel glow, reacting to bass and mid frequencies.
- **Low Latency**: Dedicated audio processing thread.
- **Efficient**: Written in C with OpenGL 3.3 Core.

## Dependencies

- **Build Tools**: CMake, Make, GCC/Clang
- **Libraries**:
  - `libpulse-dev` (PulseAudio)
  - `libfftw3-dev` (FFTW3)
  - `libglfw3-dev` (GLFW3)
  - `libcglm-dev` (Math) - *Auto-fetched if missing*
  - OpenGL Drivers

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the visualizer:

```bash
./build/raviz
```

Options:
- `--device <name>`: Specify PulseAudio source (e.g., `alsa_output.pci....monitor`).
- `--fps <int>`: Limit FPS (default 30).
- `--intensity <float>`: scaling factor for reaction.
- `--opacity <float>`: Window background opacity 0.0 (transparent) to 1.0 (solid black, default).
- `--bins <int>`: FFT bins (default 64).

## Architecture

- **Main Thread**: Window management, OpenGL rendering.
- **Audio Thread**: Audio capture (PulseAudio), FFT processing (FFTW3).
- **Synchronization**: Mutex-protected double buffering for FFT data.