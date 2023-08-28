.. _Supported File Types:

Supported File Types
--------------------

VisIt_ can create visualizations from databases that are stored in many types 
of underlying file formats. VisIt_ has a database reader for each supported 
file format and the database reader is a plugin that reads the data from the 
input file and imports it into VisIt_. If your data format is not listed in 
`File formats supported by VisIt 
<http://visitusers.org/index.php?title=Detailed_list_of_file_formats_VisIt_supports>`_
then you can first translate your data into a format that VisIt_ can read 
(e.g. Silo_, VTK, etc.) or you can create a new database reader plugin for 
VisIt_. For more information on developing a database reader plugin, refer to 
the `Getting Data Into VisIt  
<https://visit-dav.github.io/visit-website/pdfs/GettingDataIntoVisIt2.0.0.pdf?#page=97>`_
manual or contact us via `Getting help <https://visit-dav.github.io/visit-website/support/>`_.

File extensions
~~~~~~~~~~~~~~~

VisIt_ uses file extension matching to decide which database reader plugin should be used to open a particular file.
Each database reader plugin has a set of file extensions that are used to match a filename to it.
When a file's extension matches (case sensitive except on MS Windows) that of a certain plugin, VisIt_ attempts to load the file with that plugin.
If the plugin cannot load the file, then VisIt_ attempts to open the file with the next plugin that matches the extension.

If you have a file with a common extension like ``.hdf5`` or ``.h5``, there can be *many* VisIt plugins that match those extensions.
VisIt_ will use the *first* plugin it tries that appears to successfully open the file.
Sometimes, the first plugin that can read the file isn't the one you really wanted.
In that case, your options are to explicitly select the plugin or to add it to the list of *preferred* plugins.

To explicitly select the plugin, use **File --> Open file...** and select the plugin you want from the **Open file as type** pull down list.
To add a plugin to the list of *Preferred Database Plugins*, go to **Options --> Plugin Manager...** and then the **Databases** tab.
Select the plugin from the list on the left and then hit the **Add to preferred list** button.
Be sure to go to **Options --> Save settings...** if you want your selections to persist across VisIt_ sessions.
If VisIt_ finds it is unable to open a file either because there are no plugins matching the extension or all the matching plugins failed to open the file, it will begin trying preferred plugins in the order from *top* to *bottom* of the list.

VisIt_ also supports the ``-fallback_format`` command-line option.
This option adds the specified plugin to the list of preferred plugins.
For example, ``-fallback_format VTK`` adds the VTK plugin to the list of preferred plugins.
More than one ``-fallback_format`` option can be specified on the command-line and earlier encountered options take precedence over later ones.

Finally, you can also specify the plugin to use to open a file as part of the ``-o`` command-line option.
For example, to open the file ``foobar.gorfo`` as a Silo file, you can specify ``-o foobar.gorfo,Silo_1.0`` on the command-line when starting VisIt_.
Note this feature of the ``-o`` option *requires* the plugin name (in correct case) followed by an underscore (``_``) and then its version number which is almost always ``1.0``. 
If you want to see the plugin name options as well as their version numbers, go to **Options --> Plugin Manager...** and then the **Databases** tab. 
Adding plugins to the list of *Preferred Database Plugins* will display their names and version numbers.

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

In the example below, the ``#coordflag`` directive is used to define a collection of
points in *two dimensions* where each point has a velocity magnitude value associated
with it.

.. code-block:: none

    x y velocity
    #coordflag xyv
    0 0 1
    0 1 1.01
    1 0 2.02

Likewise, for a collection of points in just *one dimension*, we would have

.. code-block:: none

    x y velocity
    #coordflag xv
    0 1
    1 1.01
    2 2.02

There are some
`additional examples <https://www.visitusers.org/index.php?title=Reading_point_data#Using_Point3D_files>`_
of Point3D files on the VisIt_ wiki pages.
