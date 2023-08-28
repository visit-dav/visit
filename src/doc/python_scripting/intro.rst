Introduction to Python Scripting
================================

Starting VisIt_'s Python Interface
----------------------------------

You can invoke VisIt_'s Python scripting interface from the command line by typing:

::

    visit -cli 

VisIt_ provides a separate Python module if you instead wish to include VisIt functions in an existing Python script.
In that case, you must first import the VisIt_ module into Python and then call the ``Launch()`` function to make VisIt_ launch and dynamically load the rest of the VisIt_ functions into the Python namespace.
VisIt_ adopts this somewhat unusual approach to module loading since the lightweight front-end module, named ``visit``, can be installed as one of your Python's site packages yet still dynamically load the real control functions from different versions of VisIt_ selected by the user.

If you do not install the ``visit`` module as a Python site package, you can tell the Python interpreter where it is located either by appending a new path to the ``sys.path`` variable as in ::

    import sys
    sys.path.append("/path/to/visit/<version>/<architecture>/lib/site-packages")

or by setting the ``PYTHONPATH`` environment variable as in ::

    env PYTHONPATH=/path/to/visit/<version>/<architecture>/lib/site-packages ./myscript.py

Here is how to import all functions into the global Python namespace:

::

    from visit import *
    Launch()

Here is how to import all functions into a "visit" module namespace:

::

    import visit
    visit.Launch()
    import visit


If you are running VisIt at an HPC center where multiple versions of VisIt are installed, the default version of VisIt may not always match the version you expect.
To avoid confusion, you should use:

::

    import visit
    visit.AddArgument("-v")
    visit.AddArgument("<version>") # for example: "3.2.0"
    visit.Launch()
    import visit



Python 3 vs Python 2
--------------------

Python 2 has reached end of life and Python 3 is now preferred.
VisIt_ was ported to use Python 3 as part of VisIt's 3.2 release.
Some Python 2 syntax and common patterns no longer work in Python 3.

For example, this is no longer valid in Python 3:

::

    print "Hello from VisIt"

In Python 3 you must call ``print`` like a function:

::

    print("Hello from VisIt")

Since many VisIt_ scripts in the wild are written for Python 2 we provide limited on-the-fly support to convert Python 2 style scripts to valid Python 3 and execute them.
The command line option ``-py2to3`` enables this automatic conversion logic.

When ``-py2to3`` is used, VisIt_ will attempt to convert the input script passed with ``-s`` and any scripts run using ``visit.Source()`` on-the-fly.
For example, if you create script called ``hello_visit.py`` that includes the Python 2 style print above and run it as follows:

::

    visit -nowin -cli -py2to3 -s hello_visit.py

On-the-fly conversion and execution will succeed and you will see:

::

    Running: cli -dv -nowin -py2to3 -s hello_visit.py
    VisIt CLI: Automatic Python 2to3 Conversion Enabled
    Running: viewer -dv -nowin -noint -host 127.0.0.1 -port 5600
    Hello from VisIt

You can also toggle this support in VisIt's CLI using:

::

    visit_utils.builtin.SetAutoPy2to3(True) # or False

You can check the current value using:

::

    visit_utils.builtin.GetAutoPy2to3()

We want to emphasize the limited aspect of the automatic support.
The best long term path is to port your Python 2 style scripts to Python 3.

Python 3 installs provide a utility called ``2to3`` that you can use to help automate porting, see https://docs.python.org/3/library/2to3.htm for more details.

If you need help porting your trusty (or favorite) VisIt script, please reach out to the VisIt_ team.

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
Each of those functions returns a tuple of strings that contain the names of the currently loaded plot or operator plugins.
Each plot and operator plugin provides a function for creating an attributes object to set the plot or operator attributes.
The name of the function is the name of the plugin in the tuple returned by the OperatorPlugins or PlotPlugins functions plus the word "Attributes".
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
