# Boids Sample

This sample is a more comprehensive example of using the engine compared to the `Triangle` sample.

It is composed of a central `Simulator` class and several components and systems.

**Components:**

* `Boid`, `Predator`, `Food`, `Acceleration`, `Movement`

**Systems:**

* `BoidSystem`, `PredatorSystem`, `FoodSystem`, `MovementSystem`

The sample begins with spawning 1500 boids (purple), 5 food (green), and 2 predators (orange). The boids flock together and search out food and consume it. When food is eaten, the simulator spawns more to replace it. Predators search for boids and dive towards them, consuming any that they come into contact with. Boids attempt to move away from predators but by design are slower.

Overall it is a sample showing a large number of entities spawning, being destroyed, moving, performing spatial queries, etc.

---

Below is a glorious low resolution 30 FPS capture of the sample.

[<video src="https://ptgc-public.s3.us-east-1.amazonaws.com/litl/20270729_BoidPredators.mp4" width="600"></video>](https://github.com/user-attachments/assets/3b0529ff-5cec-4270-8f71-e3b76b69646a)
