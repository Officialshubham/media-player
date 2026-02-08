#!/bin/bash
# fix-codecs-arch.sh

echo "🎬 Installing ALL media codecs for Arch Linux..."

# Update system first
sudo pacman -Syu --noconfirm

# Install ALL GStreamer plugins and codecs
echo "Installing GStreamer plugins..."
sudo pacman -S --noconfirm --needed \
    gstreamer \
    gst-plugins-base \
    gst-plugins-good \
    gst-plugins-bad \
    gst-plugins-ugly \
    gst-libav \
    gstreamer-vaapi \
    gst-plugins-base-libs \
    gst-plugins-good-libs \
    gst-plugins-bad-libs \
    gst-plugins-ugly-libs \
    gst-plugin-pipewire \
    gst-plugin-gtk

# Install essential codec libraries
echo "Installing codec libraries..."
sudo pacman -S --noconfirm --needed \
    ffmpeg \
    libavcodec \
    libavformat \
    libavutil \
    libavfilter \
    libavdevice \
    libavresample \
    libpostproc \
    libswscale \
    libswresample

# Install video codecs
echo "Installing video codecs..."
sudo pacman -S --noconfirm --needed \
    x264 \
    x265 \
    xvidcore \
    libvpx \
    libtheora \
    libvorbis \
    libvorbis-aotuv \
    opus \
    speex \
    libdv \
    schroedinger \
    libmpeg2 \
    libdca \
    a52dec \
    faac \
    faad2 \
    flac \
    wavpack \
    libmad \
    twolame \
    lame \
    libfdk-aac

# Install AUR packages for additional codecs (using paru)
if command -v paru &> /dev/null; then
    echo "Installing AUR codec packages..."
    paru -S --noconfirm --needed \
        gst-plugins-rs \
        libde265 \
        libheif \
        gst-plugin-clutter-3 \
        gst-plugin-wpe
fi

# Install video sinks (critical for playback)
echo "Installing video output plugins..."
sudo pacman -S --noconfirm --needed \
    gst-plugins-good \
    gst-plugins-bad-free \
    gst-plugins-ugly-free \
    gstreamer-vaapi \
    mesa \
    mesa-vdpau \
    libva-mesa-driver \
    libva-vdpau-driver \
    vdpauinfo \
    libvdpau-va-gl

# Test the installation
echo ""
echo "✅ Installation complete!"
echo ""
echo "Testing installed plugins..."
gst-inspect-1.0 --version
echo ""
echo "Available video sinks:"
gst-inspect-1.0 | grep -i "sink$" | grep -i video | head -10
echo ""
echo "Available audio sinks:"
gst-inspect-1.0 | grep -i "sink$" | grep -i audio | head -10