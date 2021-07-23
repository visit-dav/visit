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
In this case, the simulation code probably already has an output file that is part of some workflow and they don't to write out another file format, possible writing out duplicate data.

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

5. The final part describes the fields. The fields can be defined on either the cells or the points.
The cell or point fields can be listed in either order.

An example VTK file
-------------------

A basic VTK file is shown here.

.. literalinclude:: data_examples/lines.vtk

The line below contains the version and identifier.

.. literalinclude:: data_examples/lines.vtk
   :lines: -1

The line below contains the title.

.. literalinclude:: data_examples/lines.vtk
   :lines: 2-2

The line below contains the data type, which in this case is ASCII.

.. literalinclude:: data_examples/lines.vtk
   :lines: 3-3

The line below identifies the type of the mesh, which in this case is polydata.

.. literalinclude:: data_examples/lines.vtk
   :lines: 4-4

The following lines provide the coordinate information for the mesh.

.. literalinclude:: data_examples/lines.vtk
   :lines: 5-11

The `6` represents the number of points and `float` indicates that the values should be read in as floats.
After that are the 6 points, one point per line for readability.
There is no requirement that you have one point per line.

The following lines represent the polydata.

.. literalinclude:: data_examples/lines.vtk
   :lines: 13-16

The `3` indicates that there are three lines and the `9` indicates that there are 9 integers defining the lines.
The three lines are then defined, one line per line for readability.
There is no requirement that you have one line per line.
The first line of integers indicates that there are `2` points in the first line, made up of the 1st and 2nd points.
The indexes are 0-origin indexes into the point list.
The second line of integers indicates that there are `2` points in the second line, made up of the 3rd and 4th points.
The third line of integers indicates that there are `2` points in the third line, made up of the 5th and 6th points.

The following lines represent one field defined on the cells.

.. literalinclude:: data_examples/lines.vtk
   :lines: 18-21

The first line indicates that we have field values defined on the cells, where the number of cells is `3`.
The second line indicates we have `1` field.
The third line indicates that the name of the field is `var1`, the `1` indicates the field has 1 compenent, the `3` means we have three values and the `float` indicates the values should be read in as 32 bit floating point values.
The fourth line contains the three field values.

The following lines represent two fields defined at the points.

.. literalinclude:: data_examples/lines.vtk
   :lines: 23-

The first line indicates that we have field values defined at the points, where the number of points is `6`.
The second line indicates we have `2` fields.
The third line indicates that the name of the field is `var2`, the `1` indicates the field has 1 compenent, the `6` means we have six values and the `float` indicates the values should be read in as 32 bit floating point values.
The fourth line contains the six field values.
The fifth line indicates that the name of the field is `var3`, the `1` indicates the field has 1 compenent, the `6` means we have six values and the `float` indicates the values should be read in as 32 bit floating point values.
The sixth line contains the six field values.

.. _data_into_visit_bov:

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
