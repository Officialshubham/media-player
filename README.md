# vidc

**A simple, lightweight media player built with GTK3 and GStreamer**
<img width="877" height="690" alt="Image" src="https://github.com/user-attachments/assets/1f296d99-1ad7-4edb-acae-72f4b2b78330" />
*Optimized for Wayland desktops*

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![GTK3](https://img.shields.io/badge/GTK-3-green.svg)](https://www.gtk.org/)
[![GStreamer](https://img.shields.io/badge/GStreamer-1.0-orange.svg)](https://gstreamer.freedesktop.org/)

---

## ✨ Features

- **🎬 Fullscreen First** — Videos automatically play in fullscreen mode for an immersive experience
- **🖥️ Wayland Native** — Optimized for modern Wayland compositors (X11 compatible)
- **🎵 Format Support** — Plays MP4, MKV, AVI, MOV, WebM, MP3, WAV, FLV, and more
- **🎛️ Intuitive Controls** — Simple play, pause, stop, seek, and volume controls
- **⚡ Lightweight** — Minimal dependencies, fast startup, low resource usage

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| <kbd>Space</kbd> | Play/Pause |
| <kbd>F</kbd> | Toggle fullscreen |
| <kbd>Esc</kbd> | Exit fullscreen |
| <kbd>←</kbd> | Seek backward 5 seconds |
| <kbd>→</kbd> | Seek forward 5 seconds |

**Mouse Controls:** Double-click the video area to toggle fullscreen

---

## 🚀 Installation

### Dependencies

#### Arch Linux / Manjaro

```bash
sudo pacman -S gtk3 gstreamer gst-plugins-good gst-plugins-bad \
               gst-plugins-ugly gst-libav gst-plugin-gtk
```

#### Ubuntu / Debian

```bash
sudo apt install libgtk-3-dev libgstreamer1.0-dev \
                 libgstreamer-plugins-base1.0-dev \
                 gstreamer1.0-plugins-good \
                 gstreamer1.0-plugins-bad \
                 gstreamer1.0-plugins-ugly \
                 gstreamer1.0-libav \
                 libgstreamer-plugins-bad1.0-dev
```

#### Fedora

```bash
sudo dnf install gtk3-devel gstreamer1-devel \
                 gstreamer1-plugins-base-devel \
                 gstreamer1-plugins-good \
                 gstreamer1-plugins-bad-free \
                 gstreamer1-plugins-ugly \
                 gstreamer1-libav
```

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Officialshubham/media-player.git
cd vidc

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Install (optional)
sudo make install

# OR
./build.sh
```

---

## 🎯 Usage

### Basic Usage

```bash
# Launch with file picker
./gui-player

# Open a specific file
./gui-player /path/to/video.mp4

# OR 
./run.sh
```

### Quick Start

1. **Open a file** — Click the "Open" button or pass a file path as a command line argument
2. **Watch** — Video automatically starts playing in fullscreen
3. **Control** — Use on-screen buttons or keyboard shortcuts
4. **Adjust** — Change volume with the slider or seek through the timeline
5. **Exit** — Press <kbd>Esc</kbd> or use window controls

---

## 📁 Project Structure

```
vidc/
├── CMakeLists.txt       # Build configuration
├── README.md            # Documentation
├── LICENSE              # MIT License
├── src/
│   ├── main.cpp         # Application entry point
│   ├── PlayerGUI.cpp    # Main player implementation
│   └── PlayerGUI.hpp    # Player header file
└── build/               # Build artifacts (generated)
```

---

## 🔧 Troubleshooting

### No Video Display

If the video doesn't appear:

1. **Check GStreamer plugins:**
   ```bash
   # Test playback directly
   gst-play-1.0 your-video.mp4
   ```

2. **Install missing codecs:**
   ```bash
   # Arch Linux
   sudo pacman -S gst-libav
   
   # Ubuntu/Debian
   sudo apt install gstreamer1.0-libav
   ```

3. **Verify GTK plugin:**
   ```bash
   # Arch Linux
   sudo pacman -S gst-plugin-gtk
   ```

### Build Errors

If CMake cannot find required packages:

```bash
# Install pkg-config
sudo pacman -S pkg-config        # Arch
sudo apt install pkg-config      # Ubuntu/Debian
sudo dnf install pkgconfig       # Fedora
```

### Audio/Video Sync Issues

- Ensure all GStreamer plugin packages are installed
- Check if your system has hardware acceleration enabled
- Try different video files to isolate codec-specific issues

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

Please ensure your code follows the existing style and includes appropriate documentation.

---

## 📋 Roadmap

- [ ] Playlist support
- [ ] Subtitle support
- [x] Hardware acceleration preferences
- [x] Custom keyboard shortcuts
- [ ] Recent files menu
- [ ] Audio equalizer
- [ ] Video filters

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **[GTK3](https://www.gtk.org/)** — User interface toolkit
- **[GStreamer](https://gstreamer.freedesktop.org/)** — Multimedia framework
- **Unicode Consortium** — Icon symbols

---

## 💡 Note

**vidc** is optimized for Wayland compositors but maintains full compatibility with X11 systems. Performance may vary depending on available GStreamer plugins and hardware acceleration capabilities.

For bug reports and feature requests, please [open an issue](https://github.com/Officialshubham/media-player/issues).

---

**Made with ❤️ for the Linux desktop**
