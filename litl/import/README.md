# LITL Importer

Utility library for importing various formats and then exporting them to an unified internal format. This internal format is then easily digested by the Engine's Asset subsystem.

Each subdirectory represents an object type (such as `mesh`) and itself has three subdirectories:

* `intermediate`: The unified intermediate representation of that object.
* `import`: Format-specific importers that convert to the intermediate representation.
* `export`: Single exporter that saves the intermediate object to its binary file.

The Engine itself does not work with the original file formats but only on the internal format. For example, it can not work with `.obj` or `.fbx` meshes, but it does make use of `.litlbmsh` files.