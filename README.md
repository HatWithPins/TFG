# TFG
Repository for my physics degree final project.

I will expand this later, but for now, let's just say that this code is meant for simulating magnetorheological fluids using OpenCL. Code still needs some cleaning for making it more easy to use. At this point, I have to compile the code every time I want to change some parameter in the simulation.

I coded this in Visual Studio, using Windows and CMake. Don't know exactly if code needs something else for running in Linux or Mac systems.

And one quick note about OpenCL, if you have a NVidia card, just install CUDA and you are fine to go. If using an AMD card, like I did, you need [this library](https://github.com/GPUOpen-LibrariesAndSDKs/OCL-SDK/releases). I'm writing this on July 2021, if you found this much later in the future, it's possible that a different library is needed for compile and run OpenCL programs in AMD cards.
