.. _GetMetaData:

GetMetaData
~~~~~~~~~~~

This program shows how to query metadata for a database and print it to the console.
In real applications, of course, you'd do something more constructive with the metadata object such as populate variable menus in a GUI. 

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . GetMetaData -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar GetMetaData -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/GetMetaData.java
   :language: Java


