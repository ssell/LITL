# Renderer Sample

While using the renderer directly should be rare, there may still be use-cases where direct interaction is needed. This sample is an example of using the `litl-renderer` library directly to draw a single textured triangle.

In the sample a camera moves back-and-forth while looking at a triangle that alternates between its vertex colors and the point/nearest-sampled texture.

It should be noted that some quirks of the sample design are intentional for the sake of demonstration purposes. For instance, BDA is used to only provide per-frame data and per-pass data is supplied in a separate bound UBO. In practice there is no reason for the separate UBO other than to show how it is used.

As such, the following topics are shown in the sample:

* Window creation
* Renderer creation
* Resource creation: pipeline, texture, sampler, shader module, vertex buffer, index buffer, data buffer, and push constants.
* Command buffer retrieval - both frame synced and scoped single-shot.
* Resource binding
* Draw command issuing
* Shutdown

---

![Screenshot of the Renderer sample application.](media/sample.png)