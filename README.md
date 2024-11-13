# ast-tree-comparer

This repository contains the comparing algorithm of two trees built from the ASTs of C++ projects written in C++

## Build clang-dump-tool on **Windows**

1. Install MSYS2 on your computer, can be downloaded from: [MSYS2](https://www.msys2.org/), pay attention to install it just simply on "C:/"
2. After the installation a _C:/msys64_ directory should be available, navigate to it and open _mingw64.exe_
3. In the MSYS command line install the necessary tools for the project:

```sh
pacman -S mingw-w64-x86_64-clang mingw-w64-x86_64-llvm mingw-w64-x86_64-lld mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-clang-tools-extra mingw-w64-x86_64-ninja
```

4. After the installation completed, check if everything has been isntalled and set properly:

```sh
clang --version
llvm-config --version
cmake --version
ninja --version
```

5. If so, navigate to the _dump-tool_ directory and create a directory for build and execute the following commands:

```sh
mkdir build
cd build
cmake -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" ..
ninja
```

6. You might need to adjust the following parts in your code according to your installation and Clang version

```cpp
  std::vector<std::string> Args = {
    "-IC:/msys64/mingw64/include",
    "-IC:/msys64/mingw64/lib/clang/18/include", // adjust version if needed
    "-IC:/msys64/mingw64/include/c++/14.2.0", // adjust version if needed
  };
```

## Building clang-dump-tool on **Linux**

The similar steps can be followed, installing the same tools from the command line, Ninja is not required in this case, the code can be built with a simple _make_ command

```sh
mkdir build
cmake ..
make
```

Also, modification in the code might be required, if everything is installed and set correctly, replace the _Args_ vector with the following:

```cpp
  std::vector<std::string> Args{
    "-I/usr/lib/clang/14/include", // adjust version if needed
    "-I/usr/local/include",
    "-I/usr/include"
  };
```

## How to build the project

1. Create a **build** directory inside the **comparer** directory (navigating from **libs**):

```sh
cd ..
mkdir build
cd build
```

2. Build the project

```sh
cmake ..
make
```
