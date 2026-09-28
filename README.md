# Marching Squares (OpenCL/OpenGL Interop)

A C++ demo that computes [marching squares](https://en.wikipedia.org/wiki/Marching_squares) isolines from elevation data on the GPU with OpenCL, and renders the result directly into an OpenGL texture using CL/GL interop.

Originally built as an assignment for a university GPU programming course, cleaned up here as a standalone, self-contained project.

## How it works

- Elevation data is loaded from [`marchingsquares/hdata2.txt`](marchingsquares/hdata2.txt).
- An OpenCL kernel ([`marchingsquares/marchingsquare.cl`](marchingsquares/marchingsquare.cl)) evaluates the marching squares case for every grid cell at a given threshold, entirely on the GPU.
- A second kernel ([`marchingsquares/GLinterop.cl`](marchingsquares/GLinterop.cl)) writes the resulting isoline layers into an OpenGL texture shared between OpenCL and OpenGL, so the image never has to round-trip through the CPU.
- OpenCV is used on the CPU side to compose the color lookup layers ([`lookup*.png`](marchingsquares)) into the elevation map, and SDL2 + GLEW handle windowing and OpenGL context/extension setup.

## Requirements

- Windows with Visual Studio 2022 (Desktop development with C++ workload) and the v142 (VS2019) toolset
- A GPU/driver with OpenGL and OpenCL support
- [vcpkg](https://github.com/microsoft/vcpkg), integrated with Visual Studio (`vcpkg integrate install`)

Dependencies (SDL2, SDL2_image, GLEW, GLM, OpenCL headers/loader, OpenCV 4) are declared in [`vcpkg.json`](vcpkg.json) and resolve automatically once vcpkg is integrated.

## Building

```bash
git clone <this-repo>
cd marchingsquares
vcpkg integrate install   # once per machine, if not already done
```

Then open [`marchingsquares.sln`](marchingsquares.sln) in Visual Studio, select a configuration, and build. On first build, vcpkg will fetch and compile the dependencies declared in `vcpkg.json`, which can take a while.

Run with **F5** (Local Windows Debugger) rather than launching the `.exe` directly, the working directory needs to be the `marchingsquares/` project folder so the app can find `hdata2.txt` and the `lookup*.png` lookup textures at their relative paths.

## Controls

Mouse and keyboard input are wired up in [`main.cpp`](marchingsquares/main.cpp) / [`MyApp.cpp`](marchingsquares/MyApp.cpp) for camera control and interaction; see `CMyApp::KeyboardDown`/`MouseMove`/etc. for specifics.
