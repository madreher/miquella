# Presentation and Objectives of Miquella

Miquella is a path tracer project developped as a learning tool to experiment with different execution mediums 
and parallelisation methods/libraries. Its main focus is to provide a somewhat simple workload requiring heavy 
computation which can be parallelize either locally or distributed on multiple nodes. 

The early development of the project are focusing on creating a path tracer up and running. The book series [Ray Tracing in One Week End](https://github.com/RayTracing/raytracing.github.io)
provides an excellent base to create a simple path tracer which can be improved later on.

This projects aims at covering a wide array of parallization methods. These may include:
* SYCL, an open source standard to implement heterogeneous compute kernels
* CUDA, the goto library for GPU computation
* C++ threads as provided by the C++20 standard
* Kubernetes, an open source system for automating deployment, scaling, and management of containerized applications
* hpx, an open source library for concurrency and parallelism with support for both parallel and distributed applications
* whatever peaks interest 


# Compilation

## Linux

TODO

## Windows 

TODO

# Tag release

## 0.0.1

Base version of the project setting up the environment, conan, and building a simple GUI based on ImGUI and OpenGL. 
The GUI allows to display a texture for now automatically generated by the program but will become the host of the image 
generated by the rayrtracer.

## 0.0.2

Implement a naive path tracer following closely the implementation proposed in Raytracing in One Week End and a bit of 
Raytracing The Next Week End. The programs implement several keys concept classically found in any ray tracer: primitive shapes,
materials, scenes, camera types and a renderer. 
