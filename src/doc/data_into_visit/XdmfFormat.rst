.. _Data_Into_VisIt_Xdmf:

The Xdmf file format
====================

Xdmf (eXtensible Data Model and Format) files can represent a variety of meshes including the basic simple mesh types, such as, point, regular, rectilinear, curvilinear and unstructured.
They also support multi-block meshes.
Xdmf files consist of an XML (eXensible Markup Language) file containing meta data that references binary data in separate files.
The binary files can either be raw binary files or HDF5 files.
Xdmf also supports embedding the bulk data in the XML file, but this is usually for illustrative uses or small files.

There is a high-level Xdmf library that can be used to read and write Xdmf files.
It's also possible to directly write out Xdmf files as well as the files containing bulk data.
The advantage to using the library is that it is easier to use.
The advantage to writing out the files directly is that there are fewer external dependencies for your simulation code.
In fact, if you write out raw binary files, you will have zero external dependencies.

The remainder of this Xdmf documentation consists of a description of the file format, a simple example, and then more complex examples of the different mesh types.

The official Xdmf file format specification and description can be found `here <https://xdmf.org/index.php/Main_Page>`__.

Basic structure of an Xdmf file
-------------------------------

The overarching structure in an Xdmf file is a `Domain`.
A `Domain` consists of one or more `Grids`.
A `Grid` can be either `Uniform` or a `Collection`.
A Collection contains one or more `Uniform Grids`. 

A `Uniform` grid is the basic unit of grid and consists of a `Topology`, `Geometry` and zero or more `Attributes`.
The `Geometry` defines the coordinates of the mesh.
The `Topology` defines how the coordinates are connected.
The `Attributes` are the fields on the mesh.

The data values for a `Topology` and  `Geometry` are stored as a `DataItem`.
A `DataItem` can be used to store numeric values directly within the Xdmf file or in an external file.

Here is the structure of a basic Xdmf file. ::

    <?xml version="1.0" ?>
    <!DOCTYPE Xdmf SYSTEM "Xdmf.dtd" []>
    <Xdmf Version="3.0">
     <Domain>
       <Grid ... >
         <Topology ... />
         <Geometry ... >
           <DataItem ... >
             ...
           </DataItem>
         </Geometry>
         <Attribute ... >
           <DataItem ... >
             ...
           </DataItem>
         </Attribute>
       </Grid>
     </Domain>
    </Xdmf>

DataItem
~~~~~~~~

The `DataItem` is used to store embedded data or external binary data.

The following is an example of a `DataItem` that embeds the data directly in the Xdmf file. ::

    <DataItem Format="XML" NumberType="Float" Precision="4" Dimensions="3">
        1.0 2.0 3.0
    </DataItem>

The following is an example of a `DataItem` that references an array in an HDF5 file. ::

    <DataItem Format="HDF" NumberType="Float" Precision="4" Dimensions="3">
        output.h5:/values
    </DataItem>

The following is an example of a `DataItem` that references an array in a binary file. ::

    <DataItem Format="Binary" NumberType="Float" Precision="4" Endian="Little" Seek="0" Dimensions="3">
        output.bin
    </DataItem>

The valid values for `Format` are:

* XML
* HDF
* Binary

The valid values for `NumberType` are:

* Float
* Int
* UInt
* Char
* UChar

The valid values for `Precision` are:

* 1 - Char or UChar
* 2 - Int or UInt
* 4
* 8

`Dimensions` consists of one to three values representing a 1D, 2D or 3D array.

The valid values for `Endian` are:

* Native
* Big
* Little

`Seek` is an byte offset into a binary file.

Topology
~~~~~~~~

The following mesh topologies are supported.

* Structured

  * 2DSMesh - Curvilinear
  * 2DRectMesh - Rectilinear
  * 2DCoRectMesh - Regular
  * 3DSMesh - Curvilinear
  * 3DRectMesh - Rectilinear
  * 3DCoRectMesh - Regular

* Unstructured

  * Linear

    * Polyvertex - a group of unconnected points
    * Polyline - a group of line segments
    * Polygon
    * Triangle
    * Quadrilateral
    * Tetrahedron
    * Pyramid
    * Wedge
    * Hexahedron

  * Quadratic

    * Edge_3 - Quadratic line with 3 nodes
    * Tri_6
    * Quad_8
    * Tet_10
    * Pyramid_13
    * Wedge_15
    * Hex_20

  * Arbitrary

    * 1 - POLYVERTEX
    * 2 - POLYLINE
    * 3 - POLYGON
    * 4 - TRIANGLE
    * 5 - QUADRILATERAL
    * 6 - TETRAHEDRON
    * 7 - PYRAMID
    * 8 - WEDGE
    * 9 - HEXAHEDRON
    * 16 - POLYHEDRON
    * 34 - EDGE_3
    * 35 - QUADRILATERAL_9
    * 36 - TRIANGLE_6
    * 37 - QUADRILATERAL_8
    * 38 - TETRAHEDRON_10
    * 39 - PYRAMID_13
    * 40 - WEDGE_15
    * 41 - WEDGE_18
    * 48 - HEXAHEDRON_20
    * 49 - HEXAHEDRON_24
    * 50 - HEXAHEDRON_27

Geometry
~~~~~~~~

The following mesh geometries are supported.

* XYZ - Interlaced locations
* XY - Z is set to 0.0
* X_Y_Z - X, Y and Z are seperate arrays
* VXVYVZ - Three arrays, one for each axis
* ORIGIN_DXDYDZ - Six values, Ox, Oy, Oz + Dx, Dy, Dz
* ORIGIN_DXDY - Four values, Ox, Oy + Dx, Dy

Attribute
~~~~~~~~~

The following `AttributeType` are supported.

* Scalar
* Vector
* Tensor - 9 values expected
* Tensor6 - 6 values expected

The following `Centering` are supported.

* Node
* Cell

The C++ examples
----------------

The examples consist of C++ code fragments that write out Xdmf files directly.
The code fragments that write out the corresponding HDF5 data are not shown.
The full C++ source code that contains all the example XDMF code shown is found `here <https://github.com/visit-dav/visit/blob/develop/src/tools/data/datagen/xdmf.C>`__. This includes the code that generates the example mesh data and the code that writes out the binary mesh data to the HDF5 file.

An example of a regular mesh file
---------------------------------

A regular mesh consists of a structured mesh with constant spacing in each direction.
The mesh can be 2D or 3D.
It is defined by a `2DCoRectMesh` or `3DCoRectMesh` topology.

Here is the code that writes a 3D regular mesh.

.. literalinclude:: data_examples/xdmf.C
      :lines: 1476-1523

Here is the resultant Xdmf file.

.. literalinclude:: data_examples/corect3d.xmf

Here is the code that writes a 2D regular mesh.

.. literalinclude:: data_examples/xdmf.C
      :lines: 1427-1474

Here is the resultant Xdmf file.

.. literalinclude:: data_examples/corect2d.xmf

An example of a rectilinear mesh
--------------------------------

A rectilinear mesh consists of a structured mesh where the coordinates along each axis are specified as a 1-D array of values.
The mesh can be 2D or 3D.
It is defined by a `2DRectMesh` or `3DRectMesh` topology.

Here is the code that writes a 3D rectilinear mesh.

.. literalinclude:: data_examples/xdmf.C
      :lines: 1574-1624

Here is the resultant Xdmf file.

.. literalinclude:: data_examples/rect3d.xmf

Here is the code that writes a 2D rectilinear mesh.

.. literalinclude:: data_examples/xdmf.C
      :lines: 1525-1572

Here is the resultant Xdmf file.

.. literalinclude:: data_examples/rect2d.xmf

An example of a point mesh
--------------------------

A point mesh consists of an unstructured mesh made up of a collection of points.
The mesh can be 2D or 3D.
It is defined by a `Polyvertex` topology.

Here is the code that writes a 3D point mesh.

.. literalinclude:: data_examples/xdmf.C
      :lines: 1626-1669

Here is the resultant Xdmf file.

.. literalinclude:: data_examples/point3d.xmf

