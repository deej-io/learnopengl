# Learn OpenGL

To use vcpkg with cmake use the following command to generate the build configuration:

```
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
```

Then use the follow to compile:

```
cmake --build build
```

Execute the built binary with:

```
./build/learnopengl
```
