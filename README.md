Real-time Volumetric Cloud Rendering in Vulkan
==================================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Final Project**

## Project Overview

A realtime vulkan implementation of [*Nubis3: Methods (and madness) to model and render immersive real-time voxel-based clouds*](https://advances.realtimerendering.com/s2023/Nubis%20Cubed%20(Advances%202023).pdf), a highly detailed and immersive voxel-based cloud renderer and modeling approach in SIGGRAPH 2023, Advances in Real-Time Rendering in Games course. 

![](img/cloud_short.gif)

![](img/cloud.png)

## Feature checklist and progress
- Vulkan pipeline(half from scrach)
    - [x] Compute pipeline (compute the shape of cloud and light raymarching)
    - [x] Graphics pipeline (for post processing and tone mapping, adjusting image storage in compute shader)
    - [ ] Imgui (trouble here! Building it from scratch and didn't show)
- Algorithm implementation
    - [x] Nubis1 and Nubis2 raymarching algorithm (project cloud on a faraway atmosphere)
    - [ ] Nubis3
- Cloud modeling and noise texture generation
    - [ ] Generate profile data
- Post processing
- Interaction with scene
    - [ ] Going through the cloud
    - [ ] Day night cycle

## Installation Instructions

The project is using OpenVDB for modeling data loading. To build the project, here are a few steps to do as prerequisites:

### 1. Install vcpkg

The project uses vcpkg for the installation of OpenVDB and its dependencies. The following steps are specifically for Windows users, if you are using another operating system, please refer to the  [official GitHub page for vcpkg](https://github.com/microsoft/vcpkg).

```
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./vcpkg/bootstrap-vcpkg.bat
```

### 2. Install OpenVDB Dependencies

```
./vcpkg install zlib --triplet=x64-windows
./vcpkg install blosc --triplet=x64-windows
./vcpkg install tbb --triplet=x64-windows
./vcpkg install boost-iostreams --triplet=x64-windows
./vcpkg install boost-any --triplet=x64-windows
./vcpkg install boost-algorithm --triplet=x64-windows
./vcpkg install boost-interprocess --triplet=x64-windows
./vcpkg integrate install
```

### 3. Install OpenVDB

For operating systems other than Windows, refer to the [official GitHub page for OpenVDB](https://github.com/AcademySoftwareFoundation/openvdb).

```
./vcpkg install openvdb --triplet=x64-windows
./vcpkg integrate install
```

## Algorithms and Approaches
### Cloud Modeling
There are two major inputs containing the information to render a cloud. `Modeling Data` stores information defining the overall shape of the cloud, which can be loaded from `.vdb` files and `Cloud 3D Noise` stores the noises that will be used to calculate the details on the cloud, which can be loaded from sequences of `.tga` files. Both of the input data files can be generated from a noise generator as a Houdini Asset provided by Nubis3 team.

#### VDB

VDB is a data structure based on hierarchical voxel grids, which is especially efficient on storing data for model of clouds, smoke, and fire. In this project, we are using `.vdb` generated from the generator mentioned above, which specifically defines 3 channels for purpose of rendering clouds. 

1. `Dimensional Profile`: Construct overall shape and provide gradiant information. 

2. `Detail type`: Describing the distribution of two detail forms, Billow and Wispy, on cloud structures. 

3. `Density Scale`: Providing density modulation.

![](img/readmeref1.png)

We use `OpenVDB` to load the `.vdb` file and stores the values of the 3 channels into 3D textures that will be sent to shaders.

#### TGA

As mentioned above, there are two detail forms, `Billow` and `Wispy`. In order to simulate the behavior of the two forms, we use the 3D noises stored in the sequences of `.tga` files. The `.tga` file will be loaded as 3D textures with 4 channels, where each channel stores a specific type of noise. RG solve for `Billow` and BA solve for `Wispy`.

```
R：Low Freq "Curl-Alligator", G:High Freq "Curl-Alligator", B:Low Freq "Alligator", A: High Freq "Alligator"
```


### Cloud Raymarching
