# ast-tree-comparer

The repository contains the two C++ tools that serve as the main programs of my project. The **Dump Tool** is responsible for AST traversal and creating the text based AST (_Abstract Syntax Tree_) structure of a given translation unit, printing the results to a specific file. The **Comparer Tool** implements the comparison logic to investigate and find differences between two given ASTs. It also interracts with the _Neo4j_ database and writes the differences maintaining the tree structure. 

## Dump Tool
Dump Tool serves as a separate tool that takes a compilation unit, the compile commands the the output path as arguments, utilizes Clang API to traverse the syntax tree - alongside with the included libraries - and create the expected format of the nodes containing information about their type, path, location, with a USR. The tool uses several Clang and LLVM libraries that are part of the LLVM compiler infrastructure project. They are essential for analyzing and manipulating the C++ code. The following includes are used in the project:
-  **<clang/AST/RecursiveASTVisitor.h>**: provides a RecursiveASTVisitor template class that is a utility for traversing the Clang Abstract Syntax Tree. Used in the code to visit Declaration and Statement nodes, during the traversal, the actions and node processing are specified
- **<clang/Frontent/CompilerInstance.h>**:  represents an instance of the Clang compiler, includes the state, options, preprocessor. In our project, the most important aspect is the AST context that is provided by the library, used in the RecursiveASTVisitor implementation. Is is also responsible for configuring and running the Clang programmatically.
- **<clang/Frontent/FrontendAction.h>**: provides a FrontendAction class, it represents the actions to be performed during the compilation process. In our case, it is used for AST generation.
- **<clang/Index/USRGeneration.h>**: provides tool to generate Unified Symbol Resolution (USR). They are unique identifiers for entities such as classes, functions in a C++ program. In the program the provided USR is used to index Declaration nodes. For Statement nodes, to replace the USR in the output, "N/A" value is used.
- **<clang/Tooling/CommonOptionsParser.h>**: includes a class to simply parsing command-line options for Clang tools. With its help the input file arguments can be easily managed.
- **<clang/Tooling/Tooling.h>**: provides utilities to build custom Clang tools. With the include, it is possible to run tools over code, managing compilation databases that is a critical part of the project.
- **<llvm/Support/CommanLine.h>**: : provides a framework for defining and parsing command-line arguments.

### Build Dump Tool on **Windows**

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

### Build Dump Tool on **Linux**

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
## Comparer Tool
The Comparer Tool itself is a more complex tool responsible for multiple parts of the application. It not just reads the output files of the Dump Tool, builds the trees and compares them by different aspects, but also connects to the Neo4j database. The tool maintains the tree-like structure of the nodes during the comparison process, notes the relationships between them and writes the nodes into the database creating the Node and the Relationship.

### About the comparison
ASTs are more likely graphs as there are nodes with a given USR that appear multiple times in the tree structure. For example, consider a scenario where a specific function is referenced multiple times. In this case, the function retains the same USR across all occurences. It raises a critical design challenge: whether to create a graph from the nodes or maintain the tree structure while categorizing, and handling these situations.

The comparer tool reconstructs the tree from the input file that contains the dumped AST by the Dump Tool. However, simply storing the nodes in a tree representation is insufficient, since determining whether a node from one AST exists in another is an essential part of the application. To address this, nodes are also stored in maps during the tree-building process.

The program uses the breadth-first search algorithm to start the comparison, as it is beneficial to compare the two trees by levels to cut subtrees if possible to minimize node comparisons. With this implementation, four kinds of differences can be detected in the trees:
- ONLY_IN_FIRST_AST
- ONLY_IN_SECOND_AST
- DIFFERENT_SOURCELOCATIONS
- DIFFERENT_PARENTS

### Build Comparer Tool

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

### Tests
Tests are implemented using [gtest](https://github.com/google/googletest), they can be found inside the tests direcdory. Similarly, create a **build** folder and compile the project inside using the same commands. To execute tests:

```sh
ctest -V
```
