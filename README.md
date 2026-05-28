# homemade-vector

A small C++ learning project that implements a custom dynamic array (`Vector<T>`) with manual memory management.

The goal is to understand low-level container behavior: allocation, element lifetime, copy/move semantics, and growth strategy.

## Current Features

- Template container: `Vector<T>`
- Dynamic growth in `push_back` (capacity doubling)
- `pop_back`
- Copy constructor
- Copy assignment operator
- Move constructor
- Move assignment operator
- Element access:
  - `operator[]`
  - `at()`
  - `front()`
  - `back()`
- Utility methods:
  - `data()`
  - `size()`
  - `capacity()`
  - `empty()`
  - `clear()`
  - `begin()` / `end()`

## Build

This project uses CMake.

```powershell

cmake -S . -B .\cmake-build-debug
cmake --build .\cmake-build-debug --config Debug
```

## Run

```powershell
& .\cmake-build-debug\homemade_vector.exe
```

## Project Layout

- `main.cpp` - container implementation and a small usage example
- `CMakeLists.txt` - build configuration

## Learning Notes

This is a study project, not a production-ready STL replacement.

Next:
- Add other methods similar to STL Vector
- Add tests to show the functionality of the Vector
