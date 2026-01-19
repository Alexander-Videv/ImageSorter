 Simple pixel sorter implemented in c++ using [raylib](https://www.raylib.com/)/[raygui](https://github.com/raysan5/raygui).

Supports *.jpg* file extensions, using *R8G8B8A8* format. 

The image mask is there for the user to understand whats being sorted, in both normal and inverted range mode, ***White*** pixels are sorted based on their calculated luminosity, whle ***Black*** pixels stay in place. 

 Requires Cmake to build.