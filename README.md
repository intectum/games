# Games Monorepo

This is a monorepo of all of Intectum's internal game projects. It includes an in-house game engine called ludo.

## Structure

- astrum
- ludo
- ludo-assimp
- ludo-bullet
- ludo-glfw
- ludo-opengl

## Deps

`apt install xorg-dev`

3rd party dependencies are included as git submodules.

### FreeImage

FreeImage does not compile under c++17 or higher. To account for this you may need to change this line in `Makefile.fip`:

`CXXFLAGS ?= -O3 -fPIC -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy`

to

`CXXFLAGS ?= -O3 -fPIC -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy -std=c++14`
