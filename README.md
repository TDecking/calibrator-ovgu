# calibrator-ovgu

Introduction
---

This program is a simple tool for manipulating point clouds.
Specifically, it allows transformation of several distinct clouds
containing individual perspectives of the same scene, until the original scene is reconstructed.
Corresponding transformation matrices may then be extracted.


Building
---

On Windows, extract release of [Opene3D](https://github.com/isl-org/Open3D),
rename it `open3d-windows-Debug` or `open3d-windows-Release` depending
on the build type.
Put it into the same folder in which the folder for this repository resides.
Run `cmake` with the matching build type, followed by running the desired build type for creation of the application.
