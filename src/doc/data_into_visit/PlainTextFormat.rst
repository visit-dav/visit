.. _Data_Into_VisIt_PT:

The ``PlainText`` file format
=============================

Standard CSV (Comma Separated Values) files are read using the ``PlainText`` reader.

Plain text files are text files with columns of data.
A single space, comma or tab character separates (e.g. *delimits*) individual values on each line of the file.
The ``PlainText`` reader automatically detects the separator character being used.
The file can include an arbitrary number of lines at the *beginning* of the file to be skipped.
Following any skipped lines, the file may include an optional *header* line holding the names associated with each column.
Plain text files can be used to represent the following types of data:

* A collection of curves, all defined on the same (explicit or implicit) domain.
* Points in 2D or 3D with variables defined on the points.
* A single variable defined on a 2D, uniform grid.

Defining curves with a ``PlainText`` file
-----------------------------------------

The first line can be an optional list of variable names.
The remaining lines consist of rows, where each row represents one point in each of the curves.
In this example, the values on each row are separated by commas.

Here are the first 10 lines of an example of a file representing curves.

.. literalinclude:: data_examples/curves.csv
   :lines: 1-10

Here is the Python script that created the file.

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: # Begin curve data gen logic
   :end-before: # End curve data gen logic

Here are the ``PlainText`` reader options used to read the data.

.. figure:: images/PlainTextOptions1.png

If you specify the column for the X coordinates, then that column will be used for the domain for all the curves.
If you don't specify an X coordinate, then it will use the row index for the domain for all the curves.

Here is the Python code to plot this data in VisIt_

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: SetOpenOptionsCurveTest()
   :end-before: os.unlink(filename)

and the resulting data plotted in VisIt_

.. figure:: ../../../test/baseline/databases/plaintext/PlainText_Curves.png

Defining curves using row index for X coordinate
------------------------------------------------

Here are the first 10 lines of an example of a file representing curves.

.. literalinclude:: data_examples/curves.csv
   :lines: 1-10

Here is the Python script that created the file.

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: # Begin curve noX data gen logic
   :end-before: # End curve noX data gen logic

Here is the Python code to plot this data in VisIt_

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: SetOpenOptionsCurveTestNoX()
   :end-before: os.unlink(filename)

and the resulting data plotted in VisIt_

.. figure:: ../../../test/baseline/databases/plaintext/PlainText_Curves_noX.png

Defining 2D or 3D points with variables
---------------------------------------

The first line can be an optional list of variable names.
The remaining lines consist of rows, where each row represents the coordinates and variable values for a single point.

Here are the first 10 lines of an example of a file representing 3D points.

.. literalinclude:: data_examples/points.csv
   :lines: 1-10

Here is the Python script that created the file.

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: # Begin 3D points with velocity & temp variable data gen logic
   :end-before: # End 3D points with velocity & temp variable data gen logic

Here are the ``PlainText`` reader options used to read the data.

.. figure:: images/PlainTextOptions2.png

If you specify the columns for the X and Y coordinates, the points will be defined in 2D space.
If you specify the columns for the X, Y and Z coordinates, the points will be defined in 3D space.

Here is the Python code to plot this data in VisIt_

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: SetOpenOptionsPointsTest()
   :end-before: os.unlink(filename)

and the resulting data plotted in VisIt_

.. figure:: ../../../test/baseline/databases/plaintext/PlainText_Points.png

Defining a single variable on a 2D uniform grid
-----------------------------------------------

The data is interpreted as a node centered variable on a uniform mesh where the row and column indices define the X and Y coordinates.
The rows represent values along the X direction and the rows get stacked in the Y direction.
Each row further *down* in the file gets stacked *up* one upon the other in the visualized result in VisIt_.
This means that the row-by-row *downward* direction in the file is the same as the *upward* (positive Y) direction in the visualized result in VisIt_.

The first line can be an optional list of variable names.
The first column name will be used for the name of the variable.
Other column names are ignored but nonetheless required to proper reading. 
The remaining lines consist of rows, where each row represents the values for a single Y coordinate.

Here is an example of a file representing 3D points.

.. literalinclude:: data_examples/array.csv

Here is the Python script that created the file.

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: # Begin 2D array gen logic
   :end-before: # End 2D array gen logic

Here are the ``PlainText`` reader options used to read the data.

.. figure:: images/PlainTextOptions3.png

The columns for the X, Y and Z coordinates are not used.

Here is the Python code to plot this data in VisIt_

.. literalinclude:: ../../test/tests/databases/plaintext.py
   :language: Python 
   :start-after: SetOpenOptionsForArrayTest()
   :end-before: os.unlink(filename)

and the resulting data plotted in VisIt_

.. figure:: ../../../test/baseline/databases/plaintext/PlainText_2DArray.png
