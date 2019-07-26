.. _Supported File Types:

Supported File Types
--------------------

VisIt_ can create visualizations from databases that are stored in many types 
of underlying file formats. VisIt_ has a database reader for each supported 
file format and the database reader is a plugin that reads the data from the 
input file and imports it into VisIt._ If your data format is not listed in 
`File formats supported by VisIt 
<http://visitusers.org/index.php?title=Detailed_list_of_file_formats_VisIt_supports>`_
then you can first translate your data into a format that VisIt_ can read 
(e.g. Silo, VTK, etc.) or you can create a new database reader plugin for 
VisIt_. For more information on developing a database reader plugin, refer to 
the `Getting Data Into VisIt  
<https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/visit/GettingDataIntoVisIt2.0.0.pdf>`_
manual or send an e-mail inquiry to visit-users@elist.ornl.gov.

File extensions
~~~~~~~~~~~~~~~

VisIt_ uses file extensions to decide which database reader plugin should be 
used to open a particular file format. Each database reader plugin has a set 
of file extensions that are used to match a filename to it. When a file's 
extension matches (case sensitive except on MS Windows) that of a certain 
plugin, VisIt_ attempts to load the file with that plugin. If the plugin cannot 
load the file then VisIt_ attempts to open the file with the next suitable 
plugin, before trying to open the file with the default database reader plugin.
If your files do not have file extensions then VisIt_ will attempt to use the 
default database reader plugin. You can provide the ``-default_format``
command line option with the name of the database reader plugin to use if 
you want to specify which reader VisIt_ should use when first trying to open a 
file. For example, if you want to load a PDB/Flash file, which usually has no 
file extension, you could provide: ``-default_format PDB`` on the command line.

Example Data Files
~~~~~~~~~~~~~~~~~~

As part of VisIt_'s regular testing, a number of example data files VisIt_ reads
can be found in VisIt_'s `data <https://github.com/visit-dav/visit/tree/develop/data>`_
subdirectory of the main code repository. In particular, if you are looking for examples
of various of the human readable ASCII formats VisIt_ reads so that you can produce
a compatible file, you may find examples there that help.

More Details of ASCII Formats
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here we describe more details specific to some of the ASCII formats VisIt_ reads.

.. _dotvisitfiles:

Creating ``.visit`` Files
"""""""""""""""""""""""""

To create a ``.visit`` file, simply make a new text file that contains the names
of the files that you want to visualize and save the file with a ``.visit`` extension.

* Visit will take the first entry in the ``.visit`` file and attempt to determine the
  appropriate plugin to read the file.
* Not all plugins can be used with ``.visit`` files. In general, **MD** or **MT** formats
  sometimes do not work.

  * An **MT** file is a file format that provides multiple time steps in a single file. Thus,
    grouping multiple **MT** files to produce a time series may not be supported.
  * An **MD** file is one that provides multiple domains in a single file. Thus, grouping 
    multiple **MD** files to produce a view of the whole may not be supported.

Here is an example ``.visit`` file that groups time steps together. These files should contain 
1 time step per file.

.. code-block:: none

   timestep0.silo
   timestep1.silo
   timestep2.silo
   timestep3.silo
   ...

Here is an example ``.visit`` file that groups various smaller domain files into a whole dataset 
that VisIt can visualize. Note the use of the ``!NBLOCKS`` directive and how it designates the 
number of files in a time step that constitute the whole domain. The ``!NBLOCKS`` directive must 
be on the first line of the file. In this example, we have 2 time steps each composed of 4 domain 
files.

.. code-block:: none
   :emphasize-lines: 1

   !NBLOCKS 4
   timestep0_domain0.silo
   timestep0_domain1.silo
   timestep0_domain2.silo
   timestep0_domain3.silo
   timestep1_domain0.silo
   timestep1_domain1.silo
   timestep1_domain2.silo
   timestep1_domain3.silo
   ...

You may also explicitly indicate the *time* associated with a file (or group of block files)
using the ``!TIME`` directive like so...

.. code-block:: none
   :emphasize-lines: 1,2,7

   !NBLOCKS 4
   !TIME 1.01
   timestep0_domain0.silo
   timestep0_domain1.silo
   timestep0_domain2.silo
   timestep0_domain3.silo
   !TIME 2.02
   timestep1_domain0.silo
   timestep1_domain1.silo
   timestep1_domain2.silo
   timestep1_domain3.silo
   ...


Point3D Files
"""""""""""""

Point3D files are four or fewer columns of ASCII values with some header text to indicate the
variable names associated with each column and a ``coordflag`` entry to indicate how to 
interpret the columns of data as coordinates. Point3D files can be used to define discrete
points in 1, 2 and 3 dimensions having a single scalar value associated with each point.
Some examples are below. The Point3D file...

.. code-block:: none

    x y z value
    0 0 0 0
    0 0 1 1
    0 1 0 2
    0 1 1 3
    1 0 0 4
    1 0 1 5
    1 1 0 6
    1 1 1 7

Defines a collection of 8 points in 3 dimensions have a scalar variable named *value*.
Below, the ``#coordflag`` directive is used to define the same collection of 8 points in
3 dimensions as the previous example except where the columns holding the z-coordinate
and the scalar variable are interleaved.

.. code-block:: none
   :emphasize-lines: 2

    x y value z
    #coordflag xyvz
    0 0 0 0
    0 0 1 1
    0 1 2 0
    0 1 3 1
    1 0 4 0
    1 0 5 1
    1 1 6 0
    1 1 7 1

In the example below, the ``#coordflag`` directive is used to a collection of 3 points in
*2 dimensions* where each point has a velocity magnitude value associated with it.

.. code-block:: none
   :emphasize-lines: 2

    x y velocity
    #coordflag xyv
    0 0 1
    0 1 1.01
    1 0 2.02

