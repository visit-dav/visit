.. _OpenGUI:

OpenGUI
~~~~~~~

This program shows how to start the VisIt_ **GUI** from within your Java application.
By altering the arguments passed to the ``OpenClient()`` method, you could launch other VisIt_ clients too.
A VisIt_ client is a program that uses the ``ViewerProxy`` class to control the viewer.
Examples of VisIt_ clients are: VisIt_'s **GUI**, VisIt_'s Python interface (**CLI**), and any program that uses the VisIt_ Java interface. 

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . OpenGUI -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar OpenGUI -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

The important part of this code is the call to the ``OpenClient()`` method.
The ``OpenClient`` method takes 3 arguments: ``clientName``, ``clientProgram``, ``clientArgs``.
The ``clientName`` is the internal name that will be used to identify the client inside of VisIt_.
You can pass any name that you want for this.
The ``clientProgram`` argment is a string that identifies the executable for your program.
The ``clientArgs`` argument lets you pass command line arguments to your program when it is started.
When you call ``OpenClient()``, the VisIt_ viewer will attempt to launch the specified VisIt_ client and then the client will be attached to VisIt_ and can control the VisIt_ viewer.
Any number of VisIt_ clients can be connected to the VisIt_ viewer.

.. literalinclude:: ../../../java/OpenGUI.java
   :language: Java


