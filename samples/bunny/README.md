# Bunny Sample

_**Note:** The sample is currently in progress and is slated for the v0.2 release._

This is a basic sample demonstrating how to load a mesh using the Assets system. It demonstrates that several new features are operational:

* Tasks implemented as async coroutines that allow seamless thread context switching.
    * These offer a longer-lived alternative to the preexisting intraframe async Jobs.
* Import library (`litl-import`) which transforms external formats to their internal binary file formats. For example, `.obj`/`.fbx`/`.glTF`/`.glb` → `.litlbmsh`.
* Assets system which performs async loading of on-disk files: `var bunnyMesh = assets.getMesh("mesh/bunny");`
* Depth-stencil buffer

The Stanford Bunny is used for a couple of reasons (aside from being free and widely used):

* It was authored in a right-handed coordinate system and uses counter-clockwise index winding (like most externally authored meshes). Using it demonstrates the working conversion to our internal left-handed system that uses clockwise winding.
* It does not have any normals and so those are generated for it during the conversion to a `.litlbmsh`, along with other not-quite-as-visible mesh operations.


---

![Screenshot of the Bunny sample application.](media/sample.png)