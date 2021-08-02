.. _Data_Into_VisIt_Intro:

Introduction
============

.. toctree::
    :maxdepth: 2

VisIt_ comes with over 100 database readers.
If you are using an existing format that we support, then it is just matter of opening the file.
VisIt_ uses file extensions to determine the type of reader to use. If your files don't have the correct extension you can force VisIt_ to use a specific reader.
If you don't already have an existing file format or VisIt_ doesn't support your format, there are two routes you can take.
One, is to use one of the existing formats and the other is to write your own database reader.

The database readers can be categorized into the following groups:

* Simple text formats
* Simple binary formats
* Library based formats
* Specialized formats

The simple formats are characterized by the fact that they can easily be written by a programs using text or binary based write statements.
The advantage is primarily that they are simple and that code can easily be added to a simulation to output the files without any external dependencies.
Typically they support a single type of mesh, although not necessarily.
Typically simple text formats must be read completely, even when only a portion of the data in the file may be necessary to perform the desired operation.

The library based formats are characterized by the fact that they require an external library to write.
These libraries typically have sophisticated data models and support many different types of meshes.
If you have a complex mesh type there is a better chance that it is supported.
Another advantage is that these types of file formats have metadata that allows reading only a portion of the file to determine the contents and support partial reads to only read the portion of the file that is necessary to perform the visualization.
Reading only a portion of the file can provide a significant performance improvement since reading the data is typically a large portion of the time required to get the initial image displayed.
The disadvantage is that it requires an external library, which adds build complexity to the simulation.
It also requires learning the API for the library, which also may be complex.

The specialized formats are typically asosociated with a specific simulation code.
In this case, the simulation code probably already has an output file that is part of some workflow and they don't want to write out another file format, possible writing out duplicate data.

Below is a list of recommended simple file formats and the mesh types they support.

+----------------------------------------+--------------+-------------------------------------------------------+
| Name                                   | Type         | Supported mesh types                                  |
+========================================+==============+=======================================================+
| :ref:`VTK <data_into_visit_vtk>`       | Binary, Text | Point, Regular, Rectilinear, Structured, Unstructured |
+----------------------------------------+--------------+-------------------------------------------------------+
| :ref:`BOV <data_into_visit_bov>`       | Binary       | Regular                                               |
+----------------------------------------+--------------+-------------------------------------------------------+
| :ref:`Curve <data_into_visit_curve>`   | Text         | 2D Curves                                             |
+----------------------------------------+--------------+-------------------------------------------------------+
| :ref:`PlainText <data_into_visit_pt>`  | Text         | Point, 2D Rectilinear                                 |
+----------------------------------------+--------------+-------------------------------------------------------+

Here is a list of recommended library based formats and the mesh types they support.

+-----------------------------------------------+--------------------------------------------------------+
| Name                                          | Supported mesh types                                   |
+===============================================+========================================================+
| :ref:`Silo <data_into_visit_silo>`            | Point, Regular, Rectilinear, Structured, Unstructured, |
|                                               | Multiblock, Patch based AMR                            |
+-----------------------------------------------+--------------------------------------------------------+
| :ref:`XDMF <data_into_visit_xdmf>`            | Point, Regular, Rectilinear, Structured, Unstructured, |
|                                               | Multiblock                                             |
+-----------------------------------------------+--------------------------------------------------------+
| :ref:`Conduit/Blueprint <data_into_visit_bp>` | Point, Regular, Rectilinear, Structured, Unstructured, |
|                                               | Multiblock, Patch based AMR                            |
+-----------------------------------------------+--------------------------------------------------------+

.. _data_into_visit_vtk:

The VTK file format
===================

There are primarily two types of VTK files, legacy and XML.
We are going to focus on the legacy files, since they are a little bit simpler.
In addition to the standard VTK content, VisIt_ also has some conventions for data specific to it.

The basic structure of a VTK file
---------------------------------

A VTK file consists of 5 sections.

1. The first section is the file version and identifier.
This section contains the single line: # vtk DataFile Version 3.0.

2. The second section is the title.
The title must be on a single line and can be at most 256 characters.

3. The next section is the file format.
The file format describes the type of the file, either ASCII or binary.
The type must be on a single line and contain either the word ASCII or BINARY.

4. The fourth part is the mesh structure.
It contains the points and the topology.
This part begins with a line containing the keyword DATASET followed by a keyword describing the type of dataset.
Then, depending upon the type of dataset, other keyword/data combinations define the actual data.
The topology can be 1D, 2D or 3D, although only 2D and 3D topologies are supported by VisIt_.

The following keywords for mesh topologies are supported.

+---------------------+
| Type                |
+=====================+
| STRUCTURED_POINTS   |
+---------------------+
| STRUCTURED_GRID     |
+---------------------+
| RECTILINEAR_GRID    |
+---------------------+
| POLYDATA            |
+---------------------+
| UNSTRUCTURED_GRID   |
+---------------------+

5. The final part describes the fields. The fields can be defined on either the cells or the points.
The cell or point fields can be listed in either order.
The fields can be either scalars, vectors or tensors. 

An example VTK file
-------------------

A basic VTK file is shown here.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 1-21

The line below contains the version and identifier.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: -1

The line below contains the title.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 2-2

The line below contains the data type, which in this case is ASCII.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 3-3

The line below identifies the type of the mesh, which in this case is a rectilinear grid.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 4-4

The following lines provide the coordinate information for the mesh.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 5-11

The information provides the dimensions of the coordinate arrays of the mesh along with the coordinates in each of the three directions.

The following lines represent one scalar field defined on the cells.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 13-16

The information tells us that there are 2 values for the cell data, that it is a scalar, that the name of the variable is `var1`, that it should be read in as float values, that we should use the default lookup table, and that the values consist of `1 2`.

The following lines represent one scalar field defined at the points.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 18-21

The information tells us that there are 12 values for the point data, that it is a scalar, that the name of the variable is `var2`, that it should be read in as float values, that we should use the default lookup table, and that the values consist of `1 2 3 1 2 3 1 2 3 1 2 3`.

An example of a structured points file
--------------------------------------

A structured points file results in a regular mesh with constant spacing in each direction.
The mesh can be 2D or 3D.
It is defined by specifying `STRUCTURED_POINTS` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with 3D structured points.

.. literalinclude:: data_examples/structuredpoints.vtk
 
This defines a regular mesh consiting of 12 points and 2 cells.
For the cells, it defines a single scalar variable named `var1`.
For the points, it defines 2 scalar variables named `var2` and `var3`, a vector variable named `vec1`, and a tensor variable named `ten1`.

Here is an example of a VTK file with 2D structured points.

.. literalinclude:: data_examples/structuredpoints2d.vtk
 
This defines a mesh similar to the previous one except that it has one row of points in the Z-direction.
Note that the number of dimensions in the Z-direction is 1 and that Z origin is zero.
The spacing in the Z-direction is one, but it could be any value.

An example of a structured grid file
------------------------------------

A structured grid file results in a structured mesh where the position of each node in the mesh is specified.
The mesh can be 2D or 3D.
It is defined by specifying `STRUCTURED_GRID` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with a 3D structured grid.

.. literalinclude:: data_examples/structuredgrid.vtk
 
It defines a structured mesh that is the same as the structured point example except that the mesh points are no longer regular.
This defines a structure mesh consiting of 12 points and 2 cells.
For the cells, it defines a single scalar variable named `var1`.
For the points, it defines 2 scalar variables named `var2` and `var3`, a vector variable named `vec1`, and a tensor variable named `ten1`.

Here is an example of a VTK file with a 2D structured grid.

.. literalinclude:: data_examples/structuredgrid2d.vtk
 
This defines a mesh similar to the previous one except that it has one row of points in the Z-direction.
Note that the number of dimensions in the Z-direction is 1 and that the Z values are all zero.

An example of a rectilinear grid file
-------------------------------------

A rectilinear grid file results in a structured mesh where the coordinates along each axis are specified as a 1-D array of values.
The mesh can be 2D or 3D.
It is defined by specifying `RECTILINEAR_GRID` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with a 3D rectilinear grid.

.. literalinclude:: data_examples/rectilineargrid.vtk
 
It defines a rectilinear mesh that is the same as the structured point example.

Here is an example of a VTK file with a 2D rectilinear grid.

.. literalinclude:: data_examples/rectilineargrid2d.vtk
 
This defines a mesh similar to the previous one except that it has one row of points in the Z-direction.
Note that in the Z-direction there is 1 value and that it is zero.

An example of a polydata file
-----------------------------

A polydata file results in a points, lines and surface cells.
The mesh can be 2D or 3D.
It is defined by specifying `POLYDATA` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with polydata.

.. literalinclude:: data_examples/polydata.vtk

It supportes points, lines, polygons and triangle strips.

.. _data_into_visit_bov:

An example of a unstructured grid file
--------------------------------------

An unstructured grid file results in an arbitrary combination of cell types.
The points are explicitely specified.
The mesh can be 2D or 3D.
It is defined by specifying `UNSTRUCTURED_GRID` as the `DATASET` type in the mesh structure portion of the file.

The following cell types are supported.

+--------------------------------+-------+
| Type                           | Value |
+================================+=======+
| VTK_VERTEX                     | 1     |
+--------------------------------+-------+
| VTK_POLY_VERTEX                | 2     |
+--------------------------------+-------+
| VTK_LINE                       | 3     |
+--------------------------------+-------+
| VTK_POLY_LINE                  | 4     |
+--------------------------------+-------+
| VTK_TRIANGLE                   | 5     |
+--------------------------------+-------+
| VTK_TRIANGLE_STRIP             | 6     |
+--------------------------------+-------+
| VTK_POLYGON                    | 7     |
+--------------------------------+-------+
| VTK_PIXEL                      | 8     |
+--------------------------------+-------+
| VTK_QUAD                       | 9     |
+--------------------------------+-------+
| VTK_TETRA                      | 10    |
+--------------------------------+-------+
| VTK_VOXEL                      | 11    |
+--------------------------------+-------+
| VTK_HEXAHEDRON                 | 12    |
+--------------------------------+-------+
| VTK_WEDGE                      | 13    |
+--------------------------------+-------+
| VTK_PYRAMID                    | 14    |
+--------------------------------+-------+
| VTK_QUADRATIC_EDGE             | 21    |
+--------------------------------+-------+
| VTK_QUADRATIC_TRIANGLE         | 22    |
+--------------------------------+-------+
| VTK_QUADRATIC_QUAD             | 23    |
+--------------------------------+-------+
| VTK_QUADRATIC_TETRA            | 24    |
+--------------------------------+-------+
| VTK_QUADRATIC_HEXAHEDRON       | 25    |
+--------------------------------+-------+

Here is an example of a VTK file with a 3D unstructured grid.

The BOV file format
===================

This is a work in progress.

.. _data_into_visit_curve:

the Curve file format
=====================

This is a work in progress.

.. _data_into_visit_pt:

The PlainText file format
=========================

This is a work in progress.

.. _data_into_visit_silo:

The Silo file format
====================

This is a work in progress.

.. _data_into_visit_xdmf:

The Xdmf file format
====================

This is a work in progress.

.. _data_into_visit_bp:

The Conduit/Blueprint file format
=================================

This is a work in progress.
