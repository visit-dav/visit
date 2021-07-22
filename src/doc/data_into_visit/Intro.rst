.. _Data_Into_VisIt_Intro

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
Typically they support a single type of grid, although not necessarily.
Typically simple text formats must be read completely, even when only a portion of the data in the file may be necessary to perform the desired operation.

The library based formats are characterized by the fact that they require an external library to write.
These libraries typically have sophisticated data models and support many different types of grids.
If you have a complex grid type there is a better chance that it is supported.
Another advantage is that these types of file formats have metadata that allows reading only a portion of the file to determine the contents and support partial reads to only read the portion of the file that is necessary to perform the visualization.
Reading only a portion of the file can provide a significant performance improvement since reading the data is typically a large portion of the time required to get the initial image displayed.
The disadvantage is that it requires an external library, which adds build complexity to the simulation.
It also requires learning the API for the library, which also may be complex.

The specialized formats are typically asosociated with a specific simulation code.
In this case, the simulation code probably already has an output file that is part of some workflow and they don't to write out another file format, possible writing out duplicate data.

Below is a list of simple file formats and the grid types they support.

VTK - Binary and Text
    Description - The Visualization Toolkit file format. There are both legacy
    and HTML formats. A single VTK file can support a single grid type, but
    they support many different mesh types and you can group VTK files.
    Mesh types - Point, Regular, Rectilinear, Structured, Unstructured
    Variable centering - Node or Cell
BOV - Text header file with binary data
    Description - Brick Of Values.
    Mesh types - Regular
    Variable centering - Node or Cell
Curve - Text
    Description - 2D curves.
PlainText - Text
    Description - Simple text format
    Mesh types - Point, 2D Structured
  
+------------------------------------------------------+--------------+------------------------------------------------+
| Name                                                 | Type         | Supported mesh types                           |
+======================================================+==============+================================================+
| :ref:`VTK <_data_into_visit_vtk_format>`             | Binary, Text | Regular, Rectilinear, Structured, Unstructured |
+------------------------------------------------------+--------------+------------------------------------------------+
| :ref:`BOV <_data_into_visit_bov_format>`             | Binary       | Regular                                        |
+------------------------------------------------------+--------------+------------------------------------------------+
| :ref:`Curve <_data_into_visit_curve_format>`         | Text         | 2D Curves                                      |
+------------------------------------------------------+--------------+------------------------------------------------+
| :ref:`PlainText <_data_into_visit_plaintext_format>` | Text         | Points, 2D Rectilinear                         |
+------------------------------------------------------+--------------+------------------------------------------------+

.. _data_into_visit_vtk_format

VTK file format
---------------
