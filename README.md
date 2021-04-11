<p align="center">
  <a href="https://github.com/ZeusYang/TinySoftRenderer">
    <img src="images/logo.jpg" alt="Logo" height="150">
  </a>
  <h3 align="center">TinySoftRenderer</h3>
<p align="center">
<img src="images/license.svg">
</p>
  <p align="center">
    A tiny soft-renderer built from scratch using C++ 11
    <br />
    <a href="https://github.com/ZeusYang/TinySoftRenderer/blob/master/images/demo.gif">View Demo</a>
    <a href="https://github.com/ZeusYang/TinySoftRenderer/issues">Report Bug</a>
  </p>




## About The Project

The original intention of building such a 3D rendering system from scratch without any help of graphics library is to get a deeper understanding of the three-dimensional rendering process. 



### Built With

This project was totally refactored based on previous naive version I built 2 years ago. Now, I utilize the following third-party libraries to build this renderer. Please note that SDL2 is just for displaying the rendered results as well as handling mouse and key events.
* [SDL2](https://www.libsdl.org/)
* [GLM](https://github.com/g-truc/glm)
* [stb_image](https://github.com/nothings/stb)
* [assimp](https://github.com/assimp/assimp)
* [oneTBB](https://github.com/oneapi-src/oneTBB/releases/tag/v2021.1.1)



## Getting Started

### Prerequisites

At present, I build it on Windows platform. Please make sure your system is equipped with the following softwares.  

- [cmake](https://cmake.org/)：at least version 3.5

* Microsoft visual studio 2017 or 2019
  

### Installation

Please compile the project for **x64 platform**.

1. Clone the repo
   ```sh
   git clone https://github.com/ZeusYang/TinySoftRenderer.git
   ```
   
2. Use cmake to build the project：
   
   ```
   cd build
   cmake ..
   make
   ```
   
   or using cmake-gui is ok.

Please note that copy **external/dlls/*.dll** (for example: SDL2.dll) to the corresponding example binary directory for execution (like `build/Release`). Release mode is much more efficient than debug mode.



## Usage

Please check out `examples/` for more details. 



## Features

- Affine and perspective correct per vertex parameter interpolation.
- Screen space back face culling (more robust compared to implementation in ndc space).
- Z-buffering (Reversed z) and depth testing for 3D rendering.
- Sutherland Hodgeman Homogeneous Cliping. Refs: [link1](https://fabiensanglard.net/polygon_codec/clippingdocument/Clipping.pdf), [link2](https://fabiensanglard.net/polygon_codec/)
- Accelerated edge function based triangle rasterization. Refs: [link](http://acta.uni-obuda.hu/Mileff_Nehez_Dudra_63.pdf)

<img src="images/raster.jpg" alt="Logo" width="100%">

- Texture mapping, Nearest texture sampling, Bilinear texture sampling.
- Tiling and morton curve memory layout for texture. (But it turns out that high-frequency address mapping is also time-consuming...) Refs: [link1](https://en.wikipedia.org/wiki/Z-order_curve), [link2](https://fgiesen.wordpress.com/2011/01/17/texture-tiling-and-swizzling/)
- Phong/Blinn-Phong illumination model
- Light source: point light, spot light, and direcitonal light

<img src="images/lighting.jpg" alt="Logo" width="100%">

- Mipmap texture, trilinear sampling. Refs: [link1](http://www.aclockworkberry.com/shader-derivative-functions/#footnote_3_1104), [link2](https://en.wikipedia.org/wiki/Mipmap)

<img src="images/mipmap.jpg" alt="Logo" width="100%">

- Tangent Space Normal Mapping.
- Reinhard tone mapping (from HDR -> LDR).

<img src="images/tonemapping.jpg" alt="Logo" width="100%">

- Multi sampling anti-aliasing (MSAA 4X)

<img src="images/MSAA4X.jpg" alt="Logo" width="100%">

- Alpha blending, and alpha to coverage (order independent transparency).

<img src="images/transparent.jpg" alt="Logo" width="100%">

- Multi-thread parallelization using [tbb](https://github.com/oneapi-src/oneTBB) as backend. The cpu usage could reach to 100%.



## ToDo

- Per drawable shader




## License

Distributed under the MIT License. See `LICENSE` for more information.



## Contact

yangwc3@mail2.sysu.edu.cn

