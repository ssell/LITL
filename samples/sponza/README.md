# Sponza Sample

**NOTE: The Sponza GLB file is not provided in the repository due to its size (115MB). In order to run the sample you will need to [download](https://github.com/Ludicon/sponza-gltf/blob/main/sponza-png.glb) the model file and place it under the sample local `assets/models/ignore` directory.**

This is a basic sample demonstrating how to load a model using the Assets system. It demonstrates that several new features are operational:

* Tasks implemented as async coroutines that allow seamless thread context switching.
    * These offer a longer-lived alternative to the preexisting intraframe async Jobs.
* Import library (`litl-import`) which transforms external formats to their internal binary file formats. For example, `.obj` → `.litlbmsh`.
* Assets system which performs async loading of on-disk files.
* Extraction of meshes, materials, etc. from complex model files along with inter-asset dependencies.
* Depth-stencil buffer.

In addition to basic byte loading and transforming to internal formats, the import pipeline also performs many preprocessing operations such as mesh winding correction, normal generation, texture compression, mipmap generation, shader reflection, etc.

It is intended that this sample will be updated extensively in future versions as additional capabilities arrives. It will serve as a showcase for features such lighting, shadows, transparency, post-processing, etc.

---

![Screenshot of the Sponza sample application.](media/sample.png)