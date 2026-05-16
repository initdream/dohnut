# D'oh Nut!

This is a port of The Simpsons Hit & Run for SDL2 (I only tested on Linux)

# Installation

This port uses the PC assets, so you will need to have the PC version of the game installed. Do not use the assets from the source code leak as those are not the final version, instead use the assets from the official release. Also make sure you're using the original `.rmv` movie files in the `movies` folder rather than the converted `.bk2` files that older releases of the port required.

To install the port simply copy the resulting binary to the game's folder.

## Dependencies

### Ubuntu / Debian
```bash
sudo apt install build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev ccache
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake sdl2 libpng openal ffmpeg ccache
```

# Build

To compile the project:

Clone the repository

Create and enter the build directory:
```bash
mkdir build
cd build
```

Configure and compile:
```bash
cmake ..
make -j$(nproc)
```
The resulting binary will be located in `build/code/`.

# tools

Due to the way the filesystem was rewritten, there is a need to use the utility from the folder tools/, it extracts sounds necessary for the game to work.

Copy all the files to your game's root folder and run the extract_n_move.sh utility, it will compile a binary and rename the files, matching them with what the game expects.
