.. _Data_Into_VisIt_VTK:

The VTK file format
===================

There are primarily two types of VTK files, legacy and XML.
We are going to focus on the legacy files, since they are a little bit simpler.

* VTK files can represent a single timestep.

  * If you have multiple times you can group them with a `.visit` file.

* VTK files can be text or binary

  * Both text and binary files have text meta data.
  * Binary files have binary fields and coordinates imbedded in the text.

* VisIt_ has conventions for additional meta data that fits within the VTK specification.

The remainder of this VTK documentation consistes of a description of the file format, a simple example, and then more complex examples of the different mesh types.

The official VTK file format descriptions can be found at the `VTK website <https://kitware.github.io/vtk-examples/site/VTKFileFormats/>`_.

.. warning::

    Reading VTK files into VisIt_ requires *strict* file extension matching.
    For example, if you have a pvtu file, it must have the ``.pvtu`` extension or VisIt_ will not be able to open and read the file.
    This is true even when using VisIt_'s feature to explicitly specify the plugin to use to open the file.
    For example, if you know ``file.foo`` is a VTK file and try to open it using VisIt_'s VTK plugin, it will fail because the extension does not match a known extension for VTK files.
    For VisIt_ to read VTK files, the files *must have* the correct VTK extensions.

.. _data_into_visit_vtk_structure:

The visit_writer source and header files
----------------------------------------

The ``visit_writer.c`` and ``visit_writer.h`` files contain C source code with simple functions that write VTK files that can easily be added to your existing C or C++ data generating code without introducing any external dependencies.
The library can write either text or binary files.
The library can output point, unstructured, rectilinear, regular and curvilinear meshes.

The `header <https:github.com/visit-dav/visit/blob/develop/src/tools/data/writer/visit_writer.h>`_ and `source <https:github.com/visit-dav/visit/blob/develop/src/tools/data/writer/visit_writer.c>`_ files can be found in the VisIt_ repository on GitHub.

Binary files
------------

Binary files contain binary coordinates and fields.
This is any data where a length and data type are specified.
The binary data follows immediately after the newline character of the previous ASCII keyword and parameter sequence.
Binary data must be written in *big endian* format.
If you are on a *little endian* system, you will need to byte swap the data before writing it.
X86 and ARM processors store data in *little endian* format, so you will need to byte swap the data in that case.
If you want to write your code independent of the *endianness*, you can write code to detect it and do the byte swapping if appropriate.
The ``visit_writer`` mentioned above contains code to detect the *endianness* and do byte swapping if necessary.

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
If the Z-coordinates are all zero, VisIt_ will treat the mesh as 2D and display it in 2D.

5. The final part describes the fields.
The fields can be defined on either the cells or the points.
The cell or point fields can be listed in either order.
The fields can be either scalars, vectors or tensors. 

Mesh topology
~~~~~~~~~~~~~

The following mesh topologies are supported.

* Structured points
* Structured grid
* Rectilinear grid
* Polydata
* Unstructured grid

Structured points
"""""""""""""""""

The structured points section has the following structure. ::

    DATASET STRUCTURED_POINTS
    DIMENSIONS nx ny nz
    ORIGIN x y z
    SPACING sx sy sz

`nx`, `ny`, `nz` are the number of dimensions in the X- Y- and Z-directions.
`x`, `y`, `z` is the origin of the grid.
`sx`, `sy`, `sz` is the spacing in the X- Y- and Z-directions.

Structured grid
"""""""""""""""

The structured grid section has the following structure. ::

    DATASET STRUCTURED_GRID
    DIMENSIONS nx ny nz
    POINTS nPoints dataType

`nx`, `ny`, `nz` are the number of dimensions in the X- Y- and Z-directions.
`nPoints` is the number of points.
`nPoints` must be consistent with the dimensions.
Supported data types in VisIt_ are `float` and `double`.

Rectilinear grid
""""""""""""""""

The rectilinear grid section has the following structure. ::

    DATASET RECTILINEAR_GRID
    DIMENSIONS nx ny nz
    X_COORDINATES nx dataType
    x1 x2 ... xn
    Y_COORDINATES ny dataType
    y1 y2 ... yn
    Z_COORDINATES nz dataType
    z1 z2 ... zn

`nx`, `ny`, `nz` are the number of dimensions in the X- Y- and Z-directions.
`nx`, `ny`, `nz` in the dimensions statement must be constient with the ones in the coordinates statement.
Supported data types in VisIt_ are `float` and `double`.

Polydata
""""""""

The polydata grid section has the following structure. ::

    DATASET POLYDATA
    POINTS nPoints dataType
    x1 y1 z1
    x2 y2 z2
    ...
    xn yn zn

    VERTICES nVertices size
    1 p1
    1 p2
    ...
    1 pn

    LINES nLines size
    2 l11 l12
    2 l21 l22
    ...
    2 ln1 ln2

    POLYGONS nPolygons size
    n1 i11 i12 ... i1n1
    n2 i21 i22 ... i2n2
    ...
    nn in1 in2 ... innn

    TRIANGLE_STRIPS nStrips size
    n1 i11 i12 ... i1n1
    n2 i21 i22 ... i2n2
    ...
    nn in1 in2 ... innn

Supported data types in VisIt_ are `float` and `double`.
The vertices, lines, polygons and triangle_strips sections may or may not be present.
The vertices, lines, polygons and triangle_strips sections may be in any order.
`xn`, `yn` and `zn` are the coordinates of the nth point.
`n1`, `n2` and `nm` are the number of indices for each cell.
`in1`, `in2` and `innn` are the zero origin indices for the nth cell.

Unstructured grid
"""""""""""""""""

The unstructured grid section has the following structure. ::

    DATASET UNSTRUCTURED_GRID
    POINTS nPoints dataType
    x1 y1 z1
    x2 y2 z2
    ...
    xn yn zn

    CELLS nCells size
    n1 i11 i12 ... i1n1
    n2 i21 i22 ... i2n2
    ...
    nn in1 in2 ... innn

    CELL_TYPES nCells
    t1
    t2
    ...
    tn

Supported data types in VisIt_ are `float` and `double`.
The cells and cell_types sections may be in any order.
`xn`, `yn` and `zn` are the coordinates of the nth point.
`n1`, `n2` and `nm` are the number of indices for each cell.
`in1`, `in2` and `innn` are the zero origin indices for the nth cell.
`t1`, `t2` and `tn` are the cell types for each cell.

The following cell types are supported.

+----------------------------+-------+----------------------------+-------+
| Type                       | Value | Type                       | Value |
+============================+=======+============================+=======+
| VTK_VERTEX                 | 1     | VTK_QUADRATIC_EDGE         | 21    |
+----------------------------+-------+----------------------------+-------+
| VTK_POLY_VERTEX            | 2     | VTK_QUADRATIC_TRIANGLE     | 22    |
+----------------------------+-------+----------------------------+-------+
| VTK_LINE                   | 3     | VTK_QUADRATIC_QUAD         | 23    |
+----------------------------+-------+----------------------------+-------+
| VTK_POLY_LINE              | 4     | VTK_QUADRATIC_TETRA        | 24    |
+----------------------------+-------+----------------------------+-------+
| VTK_TRIANGLE               | 5     | VTK_QUADRATIC_HEXAHEDRON   | 25    |
+----------------------------+-------+----------------------------+-------+
| VTK_TRIANGLE_STRIP         | 6     | VTK_BIQUADRATIC_QUAD       | 28    |
+----------------------------+-------+----------------------------+-------+
| VTK_POLYGON                | 7     | VTK_BIQUADRATIC_TRIANGLE   | 34    |
+----------------------------+-------+----------------------------+-------+
| VTK_PIXEL                  | 8     | VTK_CUBIC_LINE             | 35    |
+----------------------------+-------+----------------------------+-------+
| VTK_QUAD                   | 9     | VTK_LAGRANGE_TRIANGLE      | 69    |
+----------------------------+-------+----------------------------+-------+
| VTK_TETRA                  | 10    | VTK_LAGRANGE_QUADRILATERAL | 70    |
+----------------------------+-------+----------------------------+-------+
| VTK_VOXEL                  | 11    | VTK_LAGRANGE_TETRAHEDRON   | 71    |
+----------------------------+-------+----------------------------+-------+
| VTK_HEXAHEDRON             | 12    | VTK_LAGRANGE_HEXAHEDRON    | 72    |
+----------------------------+-------+----------------------------+-------+
| VTK_WEDGE                  | 13    |                            |       |
+----------------------------+-------+----------------------------+-------+
| VTK_PYRAMID                | 14    |                            |       |
+----------------------------+-------+----------------------------+-------+

Cell data
~~~~~~~~~

The cell data section starts with a single line with the keyword `CELL_DATA` followed by the number of cells.
The number of cells must match the number of cells defined by the topology.
Next comes a list of fields.
Fields can be either scalars, vectors or tensors.

Scalar fields
"""""""""""""

The scalar field section has the following structure. ::

    SCALARS fieldName dataType 1
    LOOKUP_TABLE default
    s1 s2 ... sN

The scalar field section starts with a single line with the keyword `SCALARS` followed by the name of the field followed by the data type followed by the number of values per scalar.
The number of values per scalar is optional, and if present, must be `1`.
Next comes the lookup table information.
The lookup table information consists of a single line with the keyword `LOOKUP_TABLE` followed by the keyword `default`.
The lookup table specifies the lookup table name when mapping the scalar to colors.
The lookup table is not used by VisIt_.
Next come the scalar values.
The scalar values can be split up into lines in an arbitrary manner.

Vector fields
"""""""""""""

The vector field section has the following structure. ::

    VECTORS fieldName dataType
    v11 v12 v13
    v21 v22 v23
    ...
    vn1 vn2 vn3

The vector field section starts with a single line with the keyword `VECTORS` followed by the name of the field followed by the data type.
Next come the vector values.
The vector values consist of three values per point, regardless of whether it is for a 2D or 3D mesh.
The vector values can be split up into lines in an arbitrary manner.

Tensor fields
"""""""""""""

The tensor field section has the following structure. ::

    TENSORS fieldName dataType
    t111 t112 t113
    t121 t122 t123
    t131 t132 t133
    t211 t212 t213
    t221 t222 t223
    t231 t232 t233
    ...
    tn11 tn12 tn13
    tn21 tn22 tn23
    tn31 tn32 tn33

The tensor field section starts with a single line with the keyword `TENSORS` followed by the name of the field followed by the data type.
Next come the tensor values.
The tensor values consist of nine values per point, regardless of whether it is for a 2D or 3D mesh.
The tensor values can be split up into lines in an arbitrary manner.

Point data
~~~~~~~~~~

The point data section starts with a single line with the keyword `POINT_DATA` followed by the number of points.
The number of points must match the number of points defined by the topology.
The rest of the point data section is the same as the cell data section.

Field data
~~~~~~~~~~

Field data is an array of data arrays that can be associated with a `DATASET`, `CELL_DATA` or `POINT_DATA`.

Field data has the following general format. ::

    FIELD FieldData numberOfFields
    FieldName numberOfComponent numberOfTuples dataType
    t11 t12 ... t1cn
    t21 t22 ... t2cn
    ...
    tn1 tn2 ... tncn

The field data section begines with a single line that starts with the keywords `FIELD` and `FieldData` followed by the number of fields.
Next come the fields.
Each field starts with a single line with the field name, the number of components, the number of tuples and the data type.

.. _data_into_visit_vtk_conventions:

VisIt_ meta data conventions for VTK files
------------------------------------------

VisIt_ supports a number of conventions for storing additional data. This data is stored as FIELD data as additional information in the DATASET, CELL_DATA or POINT_DATA.

The Following meta data is stored as DATASET FIELD data.

MeshCoordType
~~~~~~~~~~~~~

This specifies if the mesh coordinates are 2D cylindrical coordinates.
If the value is a `1`, it is an R-Z mesh.
If the value is a `2`, it is a Z-R mesh.

Here is an example of specifying the mesh coordinate type.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 6-7

MeshName
~~~~~~~~

The mesh name is represented as a string.

Here is an example of specifying the mesh name.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 8-9

CYCLE
~~~~~

The cycle is specified as a single integer value.

Here is an example of specifying the cycle.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 10-11

TIME
~~~~

The time is specified as a single double precision value.

Here is an example of specifying the time.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 12-13

VisItExpressions
~~~~~~~~~~~~~~~~

Each string represents a single expression.
The string contains the expression name, the expression type and the expression.
The three properties are seperated by semicolons.
The expression type consists of one of `curve`, `scalar`, `vector`, `tensor`, `array`, `material` or `species`.

Here is an example of specifying the expressions.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 14-16

avtGhostZones
~~~~~~~~~~~~~

The ghost zones specify a flag indicating if the zone is a ghost zone or a real zone.
A one indicates a ghost zone.
A zero indicates a real zone.
The ghost zone meta data is stored as CELL_DATA FIELD data.

Here is an example of specifying ghost zones.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 33-37

.. _data_into_visit_vtk_example:

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

The information tells us that there are 2 values for the cell data, that it is a scalar, that the name of the variable is `density`, that it should be read in as float values, that we should use the default lookup table, and that the values consist of `1 2`.

The following lines represent one scalar field defined at the points.

.. literalinclude:: data_examples/rectilineargrid.vtk
   :lines: 18-21

The information tells us that there are 12 values for the point data, that it is a scalar, that the name of the variable is `u`, that it should be read in as float values, that we should use the default lookup table, and that the values consist of `1 2 3 1 2 3 1 2 3 1 2 3`.

.. _data_into_visit_vtk_metadata_example:

An example of a VTK file with extra metadata
--------------------------------------------

A VTK file with extra meta data is shown here.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 1-45

The following lines represent the mesh name associated with this file.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 8-9

The following lines represent cycle associated with this file.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 10-11

The following lines represent the time associated with this file.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 12-13

The following lines represent the expressions associated with this file.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 14-16

The following lines represent the ghost zones associated with this file.

.. literalinclude:: data_examples/extra_metadata.vtk
   :lines: 33-37

.. _data_into_visit_vtk_struct_points:

An example of a structured points file
--------------------------------------

A structured points file results in a regular mesh with constant spacing in each direction.
The mesh can be 2D or 3D.
It is defined by specifying `STRUCTURED_POINTS` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with 3D structured points.

.. literalinclude:: data_examples/structuredpoints.vtk
 
This defines a regular mesh consiting of 12 points and 2 cells.
For the cells, it defines a single scalar variable named `density`.
For the points, it defines 2 scalar variables named `u` and `v`, a vector variable named `velocity`, and a tensor variable named `stress`.

Here is an example of a VTK file with 2D structured points.

.. literalinclude:: data_examples/structuredpoints2d.vtk
 
This defines a mesh similar to the previous one except that it has one row of points in the Z-direction.
Note that the number of dimensions in the Z-direction is 1 and that Z origin is zero.
The spacing in the Z-direction is one, but it could be any value.

.. _data_into_visit_vtk_struct_grid:

An example of a structured grid file
------------------------------------

A structured grid file results in a structured mesh where the position of each node in the mesh is specified.
The mesh can be 2D or 3D.
It is defined by specifying `STRUCTURED_GRID` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with a 3D structured grid.

.. literalinclude:: data_examples/structuredgrid.vtk
 
It defines a structured mesh that is the same as the structured point example except that the mesh points are no longer regular.
This defines a structured mesh consiting of 12 points and 2 cells.
For the cells, it defines a single scalar variable named `density`.
For the points, it defines 2 scalar variables named `u` and `v`, a vector variable named `velocity`, and a tensor variable named `stress`.

Here is an example of a VTK file with a 2D structured grid.

.. literalinclude:: data_examples/structuredgrid2d.vtk
 
This defines a mesh similar to the previous one except that it has one row of points in the Z-direction.
Note that the number of dimensions in the Z-direction is 1 and that the Z values are all zero.

.. _data_into_visit_vtk_rect_grid:

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

.. _data_into_visit_vtk_polydata:

An example of a polydata file
-----------------------------

A polydata file results in a points, lines and surface cells.
The mesh can be 2D or 3D.
It is defined by specifying `POLYDATA` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with polydata.

.. literalinclude:: data_examples/polydata.vtk

It supportes points, lines, polygons and triangle strips.

.. _data_into_visit_vtk_unstruct_grid:

An example of a unstructured grid file
--------------------------------------

An unstructured grid file results in an arbitrary combination of cell types.
The points are explicitly specified.
The mesh can be 2D or 3D.
It is defined by specifying `UNSTRUCTURED_GRID` as the `DATASET` type in the mesh structure portion of the file.

Here is an example of a VTK file with a 3D unstructured grid.

.. literalinclude:: data_examples/unstructuredgrid.vtk

It contains all the linear element types.
