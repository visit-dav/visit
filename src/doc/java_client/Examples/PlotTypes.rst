.. _PlotTypes:

PlotTypes
~~~~~~~~~

This program shows how to open a file and determine which plots can be used with the data from the file.

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . PlotTypes -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar PlotTypes -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

Each plot in VisIt_ responds to a certain set of variable types (scalar,vector, and so on).
When you open a file, you get a list of variables in the metadata object.
You must match up the variable types supported by a plot and the variables from the metadata in order to determine which plots can accept which variables from the database.
This example program demonstrates a method for doing this comparison.

*Note: the Java implementation does not offer a GetVariableTypes method in the Plugin interface as it should.*
*This is an oversight that may be corrected in a future version of VisIt.*
*In the meantime, this program's GetVariableTypes method can be used to fulfill the same purpose.*


.. literalinclude:: ../../../java/PlotTypes.java
   :language: Java


