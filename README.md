Luna is a a lightweight game/media library on top of SDL2 written in
Lua. It's inspired by [love2d](www.love2d.org), but strives to be a standard Lua
 module, allowing users to host their own programs (even if
it's just using the `lua` command included with the language install).

To use:
1. Ensure that the #include flags in luna.c are pointing to where they should
be (as sometimes libraries and header files are installed in different
directories).
2. You may need to change CC= in the makefile to gcc; it is set to clang by
default.
3. Run make
4. Copy luna.so to the folder for your project


Requires the following libraries to be installed:
Lua >= 5.2
SDL2
SDL2_mixer
SDL2_image
