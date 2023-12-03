Real-time Volumetric Cloud Rendering in Vulkan
==================================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Final Project**

## Project review

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

## Algorithms and approaches
### Cloud modeling

### Cloud raymarching
