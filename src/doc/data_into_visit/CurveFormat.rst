.. _Data_Into_VisIt_Curve:

the Curve file format
=====================

A curve file is a text file for specifying 1D curves.
It consists of one or more comment lines with meta data about the curve followed by the coordinates.
The comment lines start with either a `#` or `%` character.
Comment lines consist of keyword / value pairs.
Supported keywords are `CYCLE` and `TIME`.
The last comment line contains the name of the curve.
The points consists of the X-Y coordinates for each point, one per line.
The X coordinates must be monotonically increasing.
A curve file may contain multiple curves per file.

Here is a sample curve file.

.. literalinclude:: data_examples/curves.curve
