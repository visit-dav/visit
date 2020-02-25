.. _Read_Write_Options:

Database Read and Write Options
-------------------------------
Several database plugins have options that affect reading and/or writing with that
format. These are described in this section. Alternatively, in some cases, the
behavior of a database plugin may be affected by enviornment variables.

Chombo
~~~~~~

This section incomplete.

Exodus
~~~~~~
   

Detect Compound Variables
"""""""""""""""""""""""""

Checking this option will cause the plugin to try to guess that similarly named
variables are the scalar components of an aggregate type such as a vector,
tensor or array variable. The plugin will then automatically define expressions
for these aggregate typed variables. For example, it will cause the plugin to
combine three scalar variables with names such as ``velx``, ``vely`` and ``velz``
into a *vector* expression ``vel`` defined as ``{velx, vely, velz}``. Note that
this is just a convenience to free users from having to define expressions
manally within their VisIt_ session.


Use Material Convention
"""""""""""""""""""""""

With this option, the user can cause the plugin to recognize standard or custom
*material conventions*. The Exodus file format does not define any specific
standards for handling advecting and *mixing* materials. Different data
producers have defined different conventions. A few pre-defined conventions for
handling mixed materials from Exodus files are supported. In addition, users can
define their own custom conventions as well. For a custom convention, the user
must define the *namescheme* that will produce the names of the scalar variables
holding material volume fractions. Optionally, users can specify a namescheme to
produce the names of the scalar variables holding material-specific values for an
associated non-material-specific variable.

The
`nameschemes <https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/LLNL-SM-654357.pdf?#page=226>`_
used here are identical to those described in the
`Silo user's manual <https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/LLNL-SM-654357.pdf>`_
with one extension. The conversion specifier ``%V`` is used to denote the basename
(non-material-specific) name of a set of scalar variables holding material
specific values.

The ALEGRA nameschemes for volume fraction and material specific variables are
``"@%s@n?'&VOLFRC_%d&n&':'VOID_FRC':@"`` and ``"@%V_%d@n"``.

The CTH nameschemes are ``"@%s@n?'&VOLM_%d&n&':'VOID_FRC':@"`` and ``"@%V_%d@n"``.

Finally, in all cases it is assumed materials are identified starting from index
one (1).  The special material id of zero (0) is used to denote void.

Material Count
++++++++++++++

Ordinarily, the plugin will determine the material count from the material
convention nameschemes. However, if it is having trouble getting the correct
count, users can specify it manually with this option. 

ffp
~~~
The ffp plugin can optionally use the
`STRIPACK library <https://people.sc.fsu.edu/~jburkardt/f_src/stripack/stripack.html>`_
to improve its behavior and performance. It will do so by loading the STRIPACK library as
a dynamicaly loaded shared library *when* VisIt_ can find it. VisIt_ will find the
STRIPACK library if it is available in the VisIt_ installation's top-level ``lib``
directory (typically something like) or if the enviornment variable 
``VISIT_FFP_STRIPACK_PATH`` is set specifying a path to the shared library as in

.. code-block:: shell

    setenv VISIT_FFP_STRIPACK_PATH /foo/bar/lib/libstripack.so

for csh (and friends) or for sh (and friends)...


.. code-block:: shell

    export VISIT_FFP_STRIPACK_PATH=/foo/bar/lib/libstripack.so

In client/server mode, the STRIPACK library must be installed on both the client
and the server.

The :ref:`build_visit <Building>` tool can be used to download, build and install the
STRIPACK library. Here is an example bash shell ``build_visit`` command-line...

.. code-block:: shell

    env FCFLAGS="-fdefault-real-8 -fdefault-double-8 -shared -fPIC" \
    STRIPACK_INSTALL_DIR=/usr/local/visit/lib ./build_visit --fortran \
    --no-visit --no-thirdparty --thirdparty-path /dev/null --no-zlib --stripack

Because STRIPACK is non-BSD licensed software, part of the ``build_visit``
process for installing it is to accept the STRIPACK license terms.

H5Part
~~~~~~

This section incomplete.

IDX
~~~

This section incomplete.

M3DC1
~~~~~

This section incomplete.

MDSplus
~~~~~~~

This section incomplete.

MFIX
~~~~

This section incomplete.

MFIXCDF
~~~~~~~

This section incomplete.

NASTRAN
~~~~~~~

The ``Num Materials`` option allows the user to indicate that the NASTRAN plugin
should look for and try to define a material object. If the user knows the
*number* of materials in the input database, it is best to specify it here
because that will avert the plugin having to read all lines of the input before
understanding the material configuration. However, if the user does not know
the number of materials, enter ``-1`` here and the plugin will search for
all information related to the material configuration during the *open*. This will
lead to longer open times. A value of ``0`` here means to ignore any material
information if present.

Nektar++
~~~~~~~~

This section incomplete.

Pixie
~~~~~

This section incomplete.

Silo
~~~~
   
Ignore Extents
""""""""""""""

The Silo database plugin has the ability to load spatial and data extents for
Silo multi-block (e.g. multiple domain) objects. This feature is an optional
*acceleration* feature that enables VisIt to cull domains based on knowledge
of downstream operations. For example, it can avoid reading domains known not
to intersect a slice plane. However, if the data producer creates buggy extents
data, this can lead to problems during visualization. So, the Silo plugin has
read options to disable spatial and data extents. The options for each are
``Always``, ``Auto``, ``Never`` and ``Undef``\ (ined) where ``Always`` and
``Never`` mean to always *ignore* or never *ignore* the extents data and
``Auto`` means to ignore extents data for files written by data producers known
to have issues with extents data in the past. The ``Undef`` setting is to deal
with cases where users may have :ref:`saved settings <How to Save Settings>` with
very old versions of these options.

Force Single
""""""""""""
The ``Force Single`` check box enables the Silo library's
`DBForceSingle() <https://wci.llnl.gov/codes/silo/media/pdf/LLNL-SM-453191.pdf?#page=41>`_
method. This can potentially be useful when reading double precision data and
running out of memory.

Search for ANNOTATION_INT (and friends)
"""""""""""""""""""""""""""""""""""""""
The ``ANNOTATION_INT`` (and friends) objects are generic containers sometimes
used to store mesh-specific data using Silo's
`compound array <https://wci.llnl.gov/codes/silo/media/pdf/LLNL-SM-453191.pdf?#page=260>`_.
However, because there is no multi-block analog for Silo compound arrays, in
order to handle them VisIt_ needs to be forced to go searching for their
existence in all the files comprising a multi-block database. Thus, enabling
this option can result in much slower database *open* times.

Shapefile
~~~~~~~~~

This section incomplete.

Uintah
~~~~~~

This section incomplete.

Vs
~~

Database Write Options
----------------------

This section incomplete.

