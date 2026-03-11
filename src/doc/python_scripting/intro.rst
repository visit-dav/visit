Introduction to Python Scripting
================================

Starting VisIt_'s Python Interface
----------------------------------

You can invoke VisIt_'s Python scripting interface from the command line by typing:

::

    visit -cli 

VisIt_ provides a Python module if you instead wish to import VisIt functions in an existing Python script.
In that case, you must first import the `visit_launcher` module into Python and then call the ``Launch()`` function to make VisIt_ launch and dynamically load the rest of the VisIt_. 
After ``visit_launcher.Launch()`` you can access the full visit module using ``import visit``. 

.. danger:: The visit_launcher module was introduced in VisIt 3.5.0. It is the strategy for python importing of VisIt 3.5.0 and newer. It is not compatible with older VisIt installs. 

You can tell the Python interpreter where visit is located either by appending a new path to the ``sys.path`` variable as in ::

    import sys
    sys.path.append("/path/to/visit/<version>/<architecture>/lib/site-packages")

or by setting the ``PYTHONPATH`` environment variable as in ::

    env PYTHONPATH=/path/to/visit/<version>/<architecture>/lib/site-packages ./myscript.py

Here is how to import all functions into the global Python namespace:

::
    import visit_launcher
    visit_launcher.Launch()
    from visit import *

Here is how to import all functions into a "visit" module namespace:

::

    import visit_launcher
    visit_launcher.Launch()
    import visit


If you are running VisIt at an HPC center where multiple versions of VisIt are installed, the default version of VisIt may not always match the version you expect.
To avoid confusion, you should use:

::

    import visit_launcher
    visit_launcher.AddArgument("-v")
    visit_launcher.AddArgument("<version>") # for example: "3.5.0"
    visit_launcher.Launch()
    import visit



Mixing and Matching Python Extension Modules
--------------------------------------------

.. danger:: Mixing and matching independently compiled Python extension modules can result in subtle and hard to diagnose failures.

Care must be taken when combining a variety of Python modules especially if any are `extension modules <https://docs.python.org/3/glossary.html#term-extension-module>`_ and not *pure python*.
A pure python module is one that is written *entirely* in Python and is highly portable.
Most python modules involve a combination of compiled C/C++/Fortran code wrapped with a small amount of Python.
These are less portable.
When these kinds of modules are used, a number of additional factors impact their ability to be combined in a single Python script.
These include 

    * The Python library (header files) used to compile the module.
    * The compiler used to compile the module.
    * The compiler used to compile the Python interpreter where the modules are being combined.

It is a best practice to ensure that all modules being combined are compiled with the same compiler and the same Python library.
However, each team supporting installations of a given Python module on a given platform makes their own decisions regarding these choices.
Consequently, when using combinations of Python modules installed by others, its very easy to encounter situations where the installations are incompatible and fail in subtle and hard to diagnose ways.
Worse, things may work for the most part and only intermittently produce invalid results with no warning.

Getting started
---------------

Typically, one of the first things you do with VisIt_ is open a database and draw a plot.
Here is a simple example of opening a database, adding a "Pseudocolor" plot and drawing it.

::

    OpenDatabase("/usr/local/visit/data/multi_curv3d.silo") 
    AddPlot("Pseudocolor", "u") 
    DrawPlots() 

To see a list of the available plots and operators when you use the Python scripting interface, use the Operator Plugins and Plot Plugins functions.
Each of those functions returns a tuple of strings that contain the names of the currently loaded plot or operator plug-ins.
Each plot and operator plug-in provides a function for creating an attributes object to set the plot or operator attributes.
The name of the function is the name of the plug-in in the tuple returned by the OperatorPlugins or PlotPlugins functions plus the word "Attributes".
For example, the "Pseudocolor" plot provides a function called PseudocolorAttributes.
To set the plot attributes or the operator attributes, first use the attributes creation function to create an attributes object.
Assign the newly created object to a variable name and set the fields in the object.
Each object has its own set of fields.
To see the available fields in an object, print the name of the variable at the Python prompt and press the Enter key.
This will print the contents of the object so you can see the fields contained by the object.
After setting the appropriate fields, pass the object to either the SetPlotOptions function or the SetOperatorAttributes function.

Example:

::

    OpenDatabase("/usr/local/visit/data/globe.silo") 
    AddPlot("Pseudocolor", "u") 
    AddOperator("Slice") 
    p = PseudocolorAttributes() 
    p.colorTableName = "rainbow"
    p.opacity = 0.5 
    SetPlotOptions(p) 
    a = SliceAttributes() 
    a.originType = a.Point 
    a.normal, a.upAxis = (1,1,1), (-1,1,-1) 
    SetOperatorOptions(a) 
    DrawPlots() 

That's all there is to creating a plot using VisIt_'s Python Interface.
For more information on creating plots and performing specific actions in VisIt_, refer to the documentation for each function later in this manual.
