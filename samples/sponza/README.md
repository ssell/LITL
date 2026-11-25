# Sponza Sample

**NOTE: The Sponza GLB file is not provided in the repository due to its size (115MB). In order to run the sample you will need to [download](https://github.com/Ludicon/sponza-gltf/blob/main/sponza-png.glb) the model file and place it under the sample local `assets/models/ignore` directory.**

It is intended that this sample will be updated extensively in future versions as additional capabilities arrives. It will serve as a showcase for features such lighting, shadows, transparency, post-processing, etc.

## Controls

Note that free-fly controls are not activated until the user clicks into the window.

### Mouse + Keyboard

| Input        | Action                                        |
| ------------ | --------------------------------------------- |
| Mouse delta  | Look                                          |
| `W`, `Up`    | Move forward                                  |
| `S`, `Down`  | Move backward                                 |
| `A`, `Left`  | Move left                                     |
| `D`, `Right` | Move right                                    |
| `Space`, `E` | Move up                                       |
| `Q`          | Move down                                     |
| `Shift`      | Sprint                                        |
| `Escape`     | Release mouse                                 |
| `~`          | Toggle tonemaps (None, ACES, Khronos Neutral) |
| `1`          | Decrease tonemap exposure by 0.1              |
| `2`          | Increase tonemap exposure by 0.1              |

### Gamepad

_Not yet implemented._

## Screenshots

**Tonemap = None, Exposure = 1**

![Screenshot of the Sponza sample application with no tonemap and exposure of 1.0](media/sponza_v0p4_notonemap_1exp.png)

**Tonemap = ACES, Exposure = 2.5**

![Screenshot of the Sponza sample application with ACES tonemap and exposure of 2.5](media/sponza_v0p4_aces_2p5exp.png)

**Tonemap = Khronos Neutral, Exposure = 2.0**

![Screenshot of the Sponza sample application with Khronos Neutral tonemap and exposure of 2.0](media/sponza_v0p4_khronos_2exp.png)

## Changes

* LITL v0.4:
    * Added controls for tonemapping.
    * Added directional light source.
* LITL v0.3:
    * Added input action callbacks to control new free-fly camera.
* LITL v0.2: Sample created.