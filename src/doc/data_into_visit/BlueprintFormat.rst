.. _Data_Into_VisIt_BP:

The Conduit/Blueprint file format
=================================

Conduit is a library for intuitively describing hierarchical scientific data in C++/C, Fortran, and Python.
It is used for coupling between packages in-core, serialization, and I/O tasks.
The Conduit `Node` is the basic abstraction for describing data.
The `Node` supports hierarchical construction.

Here is a simple example of using Conduit in Python.

.. literalinclude:: data_examples/node_example.py
      :lines: 4-12

Here is the output.

.. literalinclude:: data_examples/node_example.out

Blueprint is a set of higher-level conventions for describing meshes and fields defined on those meshes.  
The Conduit library can be used to describe in-memory arrays defining a mesh and fields conforming to the Blueprint data model and then writing them to disk.
Blueprint can be used to check if a Conduit Node conforms to the Blueprint specification.

Here is a simple example writing a Blueprint uniform mesh in Python.

.. literalinclude:: data_examples/blueprint_example.py
      :lines: 4-45

Here is the output.

.. literalinclude:: data_examples/blueprint_example.out

The complete documentation for Blueprint can be found  `here <https://llnl-conduit.readthedocs.io/en/latest/blueprint.html>`_.
