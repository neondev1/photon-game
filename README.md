# Photon

A (terrible) sprite-less game cobbled together with OpenGL, originally conceived as a brief project but which ended up taking the entire summer to finish and costing a sizeable portion of my sanity along the way. Possibly my worst but still proudest creation.

Almost all of the non-OpenGL/library stuff, with the exception of a bit of stuff (big thanks to the people who suggested using a non-cryptographic hash function for generating noise!), is written by me.

## Instructions

Releases can probably be found under the "Releases" heading on the right-hand side of the page or [here](/releases), if I actually remember to eventually add them. No installation required, but you should probably put the executable in its own folder.

If you want to build the binaries for Windows yourself, compile it in Visual Studio using the Release option. (I'm too lazy to get this to work with other IDEs/compilers or use CMake, and I'm sure you wouldn't want to use CMake either.)

If you're targeting another operating system, you'll also need to compile the binaries yourself; in that case, make sure that you are using C++17 and to link to `opengl32.lib`. This program hopefully shouldn't contain anything Windows-specific, but I could be wrong.

## More Instructions

## Credits

This software uses the following libraries:

- [GLFW](https://www.glfw.org/): Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla Löwy; licensed under the [zlib License](https://github.com/glfw/glfw/blob/master/LICENSE.md)
- [Glad](https://glad.dav1d.de/): Copyright (c) 2013-2022 David Herberth; licensed under the [MIT License](https://github.com/Dav1dde/glad/blob/glad2/LICENSE)
- [GLM](https://glm.g-truc.net/): Copyright (c) 2005 G-Truc Creation; licensed under the [MIT License](https://github.com/g-truc/glm/blob/master/copying.txt)
