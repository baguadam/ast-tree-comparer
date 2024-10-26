# ast-tree-comparer

This repository contains the comparing algorithm of two trees built from the ASTs of C++ projects written in C++

## Dependencies

This project has a dependency on [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp), which provides a wrapper for the SQLite database. Before building the project, follow these steps:

1. If not already present, create a libs directory inside the **comparer** directory:

```sh
cd comparer
mkdrir libs
```

2. Navigate to the libs directory and clone the SQLiteCpp repository:

```sh
cd libs
git clone https://github.com/SRombauts/SQLiteCpp.git
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
