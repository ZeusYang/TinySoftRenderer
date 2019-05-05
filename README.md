# Soft-Renderer
&emsp;&emsp;The original intention of building such a rendering pipeline from scratch without any graphics library is to get a deeper understanding of the current three-dimensional rendering process. Many theoretical things need practice to fully understand. These days, I have gained a lot from the tortuosity of building a soft renderer, which laid a solid foundation for the future of graphics. At present, the soft rendering pipeline I have implemented already contains the basic functions of a traditional fixed pipeline, and I no longer plan to add more features such as transparent fusion, shadow and so on, because it is unnecessary.By the way, I just use Qt to display the pixel and use stb_image to load the images. The core code is not related to Qt at all.

&emsp;&emsp;Here are my personal blogs(in Chinese) if you would like get some more details about how I build it from scratch.

&emsp;&emsp;[软渲染器Soft Renderer：3D数学篇](https://yangwc.com/2019/05/01/SoftRenderer-Math/)

&emsp;&emsp;[软渲染器Soft Renderer：光栅化篇](https://yangwc.com/2019/05/01/SoftRenderer-Rasterization/)

&emsp;&emsp;[软渲染器Soft Renderer：进击三维篇](https://yangwc.com/2019/05/02/SoftRenderer-3DPipeline/)

&emsp;&emsp;[软渲染器Soft Renderer：光照着色篇（完结）](https://yangwc.com/2019/05/05/SoftRenderer-Shading/)

![directionalLight](https://github.com/ZeusYang/CDN-for-yangwc.com/blob/master/blog/SoftRenderer-Shading/directionalLight.gif)

![pointLight](https://cdn.jsdelivr.net/gh/ZeusYang/CDN-for-yangwc.com@1.1.8/blog/SoftRenderer-Shading/pointLight.gif)

![spotlight](https://cdn.jsdelivr.net/gh/ZeusYang/CDN-for-yangwc.com@1.1.8/blog/SoftRenderer-Shading/spotlight.gif)

![ret3](https://cdn.jsdelivr.net/gh/ZeusYang/CDN-for-yangwc.com@1.1.8/blog/SoftRenderer-Shading/ret3.gif)

