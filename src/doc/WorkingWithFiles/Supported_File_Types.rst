.. _Supported File Types:

Supported File Types
--------------------

VisIt can create visualizations from databases that are stored in many types 
of underlying file formats. VisIt has a database reader for each supported 
file format and the database reader is a plugin that reads the data from the 
input file and imports it into VisIt. If your data format is not listed in 
`File formats supported by VisIt 
<http://visitusers.org/index.php?title=Detailed_list_of_file_formats_VisIt_supports>`_
then you can first translate your data into a format that VisIt can read 
(e.g. Silo, VTK, etc.) or you can create a new database reader plugin for 
VisIt. For more information on developing a database reader plugin, refer to 
the `Getting Data Into VisIt  
<https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/visit/GettingDataIntoVisIt2.0.0.pdf>`_
manual or send an e-mail inquiry to visit-users@elist.ornl.gov.

File extensions
~~~~~~~~~~~~~~~

VisIt uses file extensions to decide which database reader plugin should be 
used to open a particular file format. Each database reader plugin has a set 
of file extensions that are used to match a filename to it. When a file's 
extension matches (case sensitive except on MS Windows) that of a certain 
plugin, VisIt attempts to load the file with that plugin. If the plugin cannot 
load the file then VisIt attempts to open the file with the next suitable 
plugin, before trying to open the file with the default database reader plugin.
If your files do not have file extensions then VisIt will attempt to use the 
default database reader plugin. You can provide the ``-default_format``
command line option with the name of the database reader plugin to use if 
you want to specify which reader VisIt should use when first trying to open a 
file. For example, if you want to load a PDB/Flash file, which usually has no 
file extension, you could provide: ``-default_format PDB`` on the command line.
