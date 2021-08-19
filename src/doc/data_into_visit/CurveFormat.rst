.. _Data_Into_VisIt_Curve:

the Curve file format
=====================

The curve file is a text file for specifying 1D curves.
It consists of a curve name that is on a line starting with a `#` followed by the X-Y coordinates of the curve, one point per line.
The X coordinates must be monotonically increasing.
A curve file may contain multiple curves per file.

Here is a sample curve file.

.. literalinclude:: data_examples/curves.curve
