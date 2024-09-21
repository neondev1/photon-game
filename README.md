# Photon

A (terrible) sprite-less puzzle game cobbled together with OpenGL, originally conceived as a brief project but which ended up taking the entire summer to finish and costing a sizeable portion of my sanity along the way. Possibly my worst but still proudest creation.

## Table of Contents

TBD

## Instructions

Releases can probably be found under the "Releases" heading on the right-hand side of the page or [here](https://github.com/neondev1/photon-game/releases), if I actually remember to eventually add them. No installation is required, but you should probably put the executable in its own directory since it'll make some files and I'm sure you wouldn't want this game cluttering up your otherwise organized directories.

If you want to build the binaries for Windows yourself, compile it in Visual Studio using the Release option. (I'm too lazy to get this to work with other IDEs/compilers or use CMake, and I'm sure you wouldn't want to use CMake either.) If for some reason you want to build GLFW yourself using CMake, uncheck `USE_MSVC_RUNTIME_LIBRARY_DLL`. (If statically linking the C++ runtime doesn't work, then change the runtime library setting to Multithreaded DLL (`/MD`) instead of Multithreaded (`/MT`)).

If you're targeting another operating system, you'll also need to compile the binaries yourself; in that case, make sure that you are using C++17 and to link to `opengl32.a`/`libopengl.a`. You'll also need to compile your own `.a` files for GLFW; you might want to link the program statically when compiling and accordingly build the correct version of GLFW for static linkage if you do so. The source code of this program hopefully shouldn't contain anything Windows-specific, but I could be wrong.

## Credits

This software uses the following libraries:

- [GLFW](https://www.glfw.org/): Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla Löwy; licensed under the [zlib License](https://github.com/glfw/glfw/blob/master/LICENSE.md)
- [Glad](https://glad.dav1d.de/): Copyright (c) 2013-2022 David Herberth; licensed under the [MIT License](https://github.com/Dav1dde/glad/blob/glad2/LICENSE)

## More Instructions
