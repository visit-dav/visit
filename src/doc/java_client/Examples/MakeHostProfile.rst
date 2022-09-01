.. _MakeHostProfile:

MakeHostProfile
~~~~~~~~~~~~~~~
This program shows how to create a host profile, add it to the host profile list, and send it to the viewer.
The program then goes on to access data on the remote computer, making use of the host profile that was created.
Additional options such as how to launch the engine in parallel could be added to the host profile.
Also, more profiles could be added to the host profile list before sending it to the viewer. 

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . MakeHostProfile -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar MakeHostProfile -stay -dv -datapath /path/to/silodata

If the above command line does not work or if the viewer seems to stall when connecting to the remote computer, try adding the ``-nopty`` argument to make VisIt_ prompt for passwords in the console in which you ran Java.
This should rarely be necessary.

Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/MakeHostProfile.java
   :language: Java


