.. _Data_Into_VisIt_BOV:

The BOV file format
===================

The BOV (Brick Of Values) format is a binary format used to represent a single variable defined on a regular mesh.
The BOV file consists of two files, a file with the binary data, and a small text file describing it.

.. _data_into_visit_bov_structure:

The structure of a BOV file
---------------------------

The BOV header file consists of a text file with keyword / value pairs, one per line.
The BOV header file may be interspersed with comment lines that begin with a `#`.
The supported keywords are:

BRICK_ORIGIN
~~~~~~~~~~~~

This is the origin of your grid.

Here is an example of specifying the origin of your grid.

.. literalinclude:: data_examples/density.bov
   :lines: 8-8

BRICK_SIZE
~~~~~~~~~~

This is the size of your grid.

Here is an example of specifying the size of your grid.

.. literalinclude:: data_examples/density.bov
   :lines: 9-9

The `i` component varies the fastest, then `j`, then `k`. 
This means that if `BRICK_SIZE` is `2. 2. 2.`, the first float in the data file corresponds to `[0,0,0]`, the second to `[1,0,0]`, the third to `[0,1,0]`, the fourth to `[1,1,0]`, the fifth to `[0,0,1]`, and so on.

BYTE_OFFSET
~~~~~~~~~~~

This is a byte offset into the file to start reading the data.
It lets you specify some number of bytes to skip at the front of the file.
This can be useful for skipping the 4-byte header that Fortran tends to write to files.
If your file does not have a header then DO NOT USE BYTE_OFFSET.

Here is an example of specifying the size of your grid. ::

    BYTE_OFFSET: 4

CENTERING
~~~~~~~~~

This is the centering of your variable.
Valid values are `ZONAL` and `NODAL`.

Here is an example of specifying the centering of your variable.

.. literalinclude:: data_examples/density.bov
   :lines: 7-7

DATA_BRICKLETS
~~~~~~~~~~~~~~

Specifies the size of the chunks when `DIVIDE_BRICK` is true.
The values chosen for `DATA_BRICKLETS` must be factors of the numbers used for `DATA_SIZE`.

Here is an example of specifying the data bricklets of your grid. ::

    DATA_BRICKLETS: 5 5 5

DATA_COMPONENTS
~~~~~~~~~~~~~~~

Specifies the number of components in your variable.
1 specifies a scalar, 2 specifies a complex number, 3 specifies a vector, and any value beyond 3 specifies an array variable.
You can use `COMPLEX` instead of `2` for complex numbers.
When your data consists of multiple components, all components for a cell or node are written sequentially to the file before going to the next cell or node.

Here is an example of specifying the data bricklets of your grid. ::

    DATA_COMPONENTS: 1

.. _data_into_visit_bov_example:

DATA_ENDIAN
~~~~~~~~~~~

This is the Endian representation of your data.
Valid values are `BIG` and `LITTLE`.
Data from Intel processors is `LITTLE`, while most other processors are `BIG`.

Here is an example of specifying the Endian representation of your variable.

.. literalinclude:: data_examples/density.bov
   :lines: 6-6

DATA_FILE
~~~~~~~~~

This is the name of the file with the binary data.

Here is an example of specifying the name of the data file.

.. literalinclude:: data_examples/density.bov
   :lines: 2-2

DATA_FORMAT
~~~~~~~~~~~

This is the type of your data.
Valid values are `BYTE`, `SHORT`, `INT`, `FLOAT` and `DOUBLE`.

Here is an example of specifying the type of your variable.

.. literalinclude:: data_examples/density.bov
   :lines: 4-4

DATA_SIZE
~~~~~~~~~

These are the dimensions of your variable.

Here is an example of specifying the dimensions of your variable.

.. literalinclude:: data_examples/density.bov
   :lines: 3-3

DIVIDE_BRICK
~~~~~~~~~~~~

A flag indicating if the array should be divided into chunks that can be processed in parallel.
Valid values are `TRUE` and `FALSE`.
When DIVIDE_BRICK is true, the values stored in DATA_BRICKLETS specifies the size of the chunks.

Here is an example of specifying divide brick. ::

    DIVIDE_BRICK: TRUE

TIME
~~~~

This is the time associated with the variable.

Here is an example of specifying the time.

.. literalinclude:: data_examples/density.bov
   :lines: 1-1

VARIABLE
~~~~~~~~

This is the name of your variable.

Here is an example of specifying the name of your variable.

.. literalinclude:: data_examples/density.bov
   :lines: 5-5

An example of a BOV file
------------------------

Here is an example header file representing a 10 by 10 by 10 array of density values.

.. literalinclude:: data_examples/density.bov

Here is a sample python script that creates the 10 by 10 by 10 array of density values.

.. literalinclude:: data_examples/density.py
