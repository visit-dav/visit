.. _Controlling Lighting:

Controlling lighting
~~~~~~~~~~~~~~~~~~~~

This example program is based on the ``RunViewer`` example program and it shows how to modify lighting in VisIt_.
The basic procedure is to obtain a handle to the state object that you want to modify, in this case, ``LightList`` and then modify the state object and call its Notify() method to send the changed object back to VisIt_'s viewer.
Once the changed object has been sent back to VisIt_'s viewer, you call a method from ViewerMethods that tells VisIt to apply the sent object to its internal state. 


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . TryLighting -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar TryLighting -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/TryLighting.java
   :language: Java


