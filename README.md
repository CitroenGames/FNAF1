# FNAF 1 Clone

![Main Menu](Images/Menu.png)


This project is an early work-in-progress clone inspired by the "Five Nights at Freddy's" (FNaF) game, built on the Paingine2D engine layer.

Currently, the game features a **main menu** (which is not yet like the original game) and an **office** scene that doesn't function properly. Key elements, such as the **animatronics**, **camera system**, **doors**, and **buttons**, are not yet implemented or fully functional.

### Features (WIP)
- **Main Menu**: Basic framework in place, but not representative of the original game's main menu yet.
- **Office**: The office scene is available, though it's currently not functional.
- **Honk Freddy's Nose**: You can honk Freddy's nose!
- **Missing Features**:
  - Animatronics AI
  - Camera system
  - Door controls
  - Functional buttons
  

### Installation Instructions for Linux Users

To get started with the game on Linux, follow these steps:

1. **Install Dependencies**

   Open a terminal and run the following commands to install the required dependencies:

   ```bash
    sudo apt update
    sudo apt install build-essential glew-utils libglew-dev
   ```

   This will install the necessary packages:
   - `build-essential`: Basic development tools, such as `make` and compilers.
   - `glew-utils` and `libglew-dev`: Libraries for OpenGL extensions.

2. **Verify OpenGL Installation (Optional)**

   If OpenGL is not already installed on your system, you can install the required tools using the following command:

   ```bash
   sudo apt install mesa-utils
   ```

   This package contains tools to check and verify your OpenGL installation.

### Build Instructions for macOS

Install Xcode Command Line Tools and sighmake, then run:

```bash
xcode-select --install
./generatesolution-macos.sh
```

The executable is written to:

```bash
build/bin/Release/Fnaf1
```

### Build Instructions for Windows

```bat
generatesolution-win64.bat
sighmake --build . --config Release --parallel 8
```

The executable is written to `build\bin\Release\Fnaf1.exe`.
