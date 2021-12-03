.. _Data_Into_VisIt_PT:

The PlainText file format
=========================

Plain text files are text files with tables of data.
These are standard CSV (Comma Separated Values) files.
The file can start with an arbitrary number of lines that may be skipped.
The files can be used to represent the following types of data:

* A collection of curves, all defined on the same domain.
* Points in 2D or 3D with variables defined on the points.
* A single variable defined on a 2D regular grid.

Defining curves with a PlainText file
-------------------------------------

The first line can be an optional list of variable names.
The remaining lines consist of rows, where each row represents one point in each of the curves.

Here are the first 10 lines of an example of a file representing curves.

.. literalinclude:: data_examples/curves.csv
   :lines: 1-10

Here is the Python script that created the file.

.. literalinclude:: data_examples/plain_text.py
   :lines: 1-14

Here are the Plain Text reader options used to read the data.

.. figure:: images/PlainTextOptions1.png

If you specify the column for the X coordinates, then that column will be used for the domain for all the curves.
If you don't specify an X coordinate, then it will use the row index for the domain for all the curves.

Defining 2D or 3D points with variables
---------------------------------------

The first line can be an optional list of variable names.
The remaining lines consist of rows, where each row represents the values for a single point.

Here are the first 10 lines of an example of a file representing 3D points.

.. literalinclude:: data_examples/points.csv
   :lines: 1-10

Here is the Python script that created the file.

.. literalinclude:: data_examples/plain_text.py
   :lines: 16-32

Here are the Plain Text reader options used to read the data.

.. figure:: images/PlainTextOptions2.png

If you specify the columns for the X and Y coordinates, the points will be defined in 2D space.
If you specify the columns for the X, Y and Z coordinates, the points will be defined in 3D space.

Defining a single variable on a 2D regular grid
-----------------------------------------------

The variable is interpreted as a node centered variable and the X and Y coordinates are the indexes into the rows and columns.
The rows represent values along the X direction and the rows get stacked in the Y direction.

The first line can be an optional list of variable names.
The first variable name will be used for the name of the variable.
Note that you will need to provide the same number of variable names as there are columns, even though only the first one is used.
The remaining lines consist of rows, where each row represents the values for a single Y coordinate.

Here is an example of a file representing 3D points.

.. literalinclude:: data_examples/regulargrid2d.csv

Here is the Python script that created the file.

.. literalinclude:: data_examples/plain_text.py
   :lines: 34-51

Here are the Plain Text reader options used to read the data.

.. figure:: images/PlainTextOptions3.png

The columns for the X, Y and Z coordinates are not used.
