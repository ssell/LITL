# Boids Sample

## Overview

This sample is a more comprehensive example of using the engine compared to the `Triangle` sample.

It is composed of a central `Simulator` class and several components and systems.

**Components:**

* `Boid`, `Predator`, `Food`, `Acceleration`, `Movement`

**Systems:**

* `BoidSystem`, `PredatorSystem`, `FoodSystem`, `MovementSystem`

The sample begins with spawning 1500 boids (purple), 5 food (green), and 2 predators (orange). The boids flock together and search out food and consume it. When food is eaten, the simulator spawns more to replace it. Predators search for boids and dive towards them, consuming any that they come into contact with. Boids attempt to move away from predators but by design are slower.

Overall it is a sample showing a large number of entities spawning, being destroyed, moving, performing spatial queries, etc.

## Controls

| Input        | Action                                         |
| ------------ | ---------------------------------------------- |
| Left Click   | Spawn predator at the click position.          |
| Right Click  | Remove predator nearest to the click position. |

## Screenshots

Below is a glorious low resolution 30 FPS capture of the sample.

[<video src="https://ptgc-public.s3.us-east-1.amazonaws.com/litl/20270729_BoidPredators.mp4" width="600"></video>](https://github.com/user-attachments/assets/3b0529ff-5cec-4270-8f71-e3b76b69646a)

## Changes

* LITL v0.3:
    * Added raw input callback to spawn predator at left-click position.
    * Added raw input callback to remove predator at right-click position.
* LITL v0.2: 
    * Updated to use Assets to load spheres for boid representation instead of manual mesh creation.
    * Updated to use Assets to load shared material instead of direct shader use.
* LITL v0.1: Sample created.