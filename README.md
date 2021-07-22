# flowsolver

Solves Flow (aka Numberlink) puzzles using the Z3 constraint solver.

## building

Depends on Raylib and Z3.

To generate build files, you can run CMake normally:
```
mkdir build
cd build
cmake .. # Add your preferred CMake options here, such as -G or -DCMAKE_BUILD_TYPE
```

And then run whatever generator you configured CMake to use.

### on windows without vcpkg

When building on Windows without VcPkg, you will need to point CMake to Z3, and
create Raylib CMake files for CMake to point at. To create the Raylib files, it
should be OK to copy the Z3 `lib/cmake/` files and modify the version code and
the library handling code. As a general rule:
- `z3::libz3` -> `raylib`
- `bin/libz3.dll` -> `lib/raylib.dll`
- `lib/libz3.lib` -> `lib/raylib.lib`
- `MAJOR.MINOR.BUGFIX.TWEAK` (z3) -> `MAJOR.MINOR.BUGFIX` (raylib)

Then, you can point CMake to these files, by setting `CMAKE_PREFIX_PATH` to
`${z3_dir}\lib\cmake\z3;${raylib_dir}\lib\cmake\raylib`.

Once Raylib ships with MSVC CMake files, this shouldn't need to be done.
