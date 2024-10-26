# ast-tree-comparer
This repository contains the comparing algorithm of two trees built from the ASTs of C++ projects written in C++
### Dependencies
This project has a dependency on [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp), which provides a wrapper for the SQLite database. Before building the project, follow these steps:
1. Clone the SQLiteCpp repository: git clone https://github.com/SRombauts/SQLiteCpp.git
2. If not already present, create a libs directory in the root of the project: mkdir libs
3. Move the cloned SQLiteCpp repository into the libs directory: mv SQLiteCpp libs/
4. Build SQLiteCpp:
```sh
cd libs/SQLiteCpp
mkdir build
cd build
cmake ..
cmake --build . --config Release
```