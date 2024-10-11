# Photon

A (terrible) sprite-less puzzle game cobbled together with OpenGL, originally conceived as a brief project but which ended up taking the entire summer to finish and costing a sizeable portion of my sanity along the way. Possibly my worst but still proudest creation.

### Table of Contents

1. [Instructions](#instructions)
2. [Level Editor](#level-editor)
3. [Credits](#credits)

## Instructions

Releases can probably be found under the "Releases" heading on the right-hand side of the page or [here](https://github.com/neondev1/photon-game/releases), if I actually remember to eventually add them. No installation is required, but you should probably put the executable in its own directory since it'll make some files and I'm sure you wouldn't want this game cluttering up your otherwise organized directories.

> [!IMPORTANT]
> In any case, if you are compiling the binaries yourself, make sure to exclude `editor/main.cpp` from compilation unless you are compiling the level editor. This should have already been done in the project settings if you use the Visual Studio project files provided with this repo, so no action is necessary if you are using MSVC.

If you want to build the binaries for Windows yourself, compile it in Visual Studio **using the Release option.** (I might switch to using CMake later, but for now I'm too lazy to do that.) If for some reason you want to build GLFW yourself using CMake, uncheck `USE_MSVC_RUNTIME_LIBRARY_DLL`. If for whatever reason statically linking the C++ runtime doesn't work, then change the runtime library setting to Multithreaded DLL (`/MD`) instead of Multithreaded (`/MT`) to dynamically link the runtime library (you might need to copy some required DLLs since the program might not know where to look for them).

If you're targeting another operating system, you'll also need to compile the binaries yourself; in that case, make sure that you are using C++17 and to link to `opengl32.a`/`libopengl32.a`. You'll also need to compile your own `.a` files for GLFW; you might want to link the program statically when compiling and accordingly build the correct version of GLFW for static linkage if you do so. The source code of this program hopefully shouldn't contain anything Windows-specific, but I could be wrong.

## Level Editor

I'll probably include the level editor alongside [releases](https://github.com/neondev1/photon-game/releases) of the game.

To compile the level editor on Windows with MSVC, build the project in Visual Studio using the EditorRelease configuration. If you are compiling using something other than MSVC, simply exclude the original `main.cpp` file from compilation and include `editor/main.cpp` instead.

> [!NOTE]
> To load a custom level into the regular game client, rename the exported level file to `level` (no extension) and put it in the same directory as the executable. The game should automatically load the level when restarted.

## Credits

This software uses the following libraries:

- [GLFW](https://www.glfw.org/): Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla Löwy; licensed under the [zlib License](https://github.com/glfw/glfw/blob/master/LICENSE.md)
- [glad](https://glad.dav1d.de/): Created by David Herberth; in the [public domain](https://github.com/Dav1dde/glad/blob/glad2/README.md#license)
