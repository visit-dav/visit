.. _Tutorials_Python_Expressions:

Python Expressions
==================

.. toctree::
       :maxdepth: 2

This tutorial describes using Python Expressions.

Python expressions overview
---------------------------

VisIt_ contains the ability to create expressions using the Python scripting language.
Python expressions run in the compute engine and run in parallel when VisIt_ is run in parallel.
Python expressions operate on vtkDataSets, which provide access to variables as well as the mesh coordinates and topology.
Python expressions also have access to VisIt_'s metadata as well as access to MPI when running in parallel.
Python expressions return a vtkDataArray, which allows returning new variables.
It isn't possible to change the mesh topology or coordinates.
The functionality is available through the *GUI* and the *CLI*.
When using the *GUI*, they can be created in the *Expressions* window. 
When using the *CLI*, they can be created with the *DefinePythonExpression* function.

Creating a Python Expression with the GUI
-----------------------------------------

We'll now go through the steps required to create a Python expression using the GUI.

Let's start by opening a file and creating a plot.

1. Open the file "curv2d.silo".
2. Create a Psuedocolor plot of *d*.

Now let's go ahead and create the Python expression.

3. Go to *Controls->Expressions*.
4. This brings up the Expressions window.
5. Click *New* in the *Expression list* to create a new expression.
6. Change the *Name* in the *Definition* section to "MyExpression".
7. Click on the *Python expression editor* tab in the *Definition* section.
8. Select *Insert variable...->Scalars->d* to add *d* to the *Arguments* text field.
9. Select *Insert variable...->Scalars->p* to add *p* to the *Arguments* text field.
   Note that the variable names are seperated by a comma.
   If the variable names are not separated by commas you will get a cryptic error message when you try to plot the expression.
10. Click *Load script->Template->Simple filter* to add a template of a Python filter in the *Python expression script* editor. 

.. figure:: images/PythonExpressions-GUI1.png

   The Expressions window with the simple filter template

At this point you modify the template to create your expression.
There are a couple of ways in which you can develop your script.
In either case you would make modifications to the script and then try it out using the Pseudocolor plot.
Changes to the script can be made either by modifying the script in the *Python expression script* editor or modifying it in an external text editor and then reloading the script.
Generaly speaking, modifying the script in the *Python expression script* editor is easier than doing it in an external text editor except that it is difficult to tell how many spaces are at the beginning of the line since the editor doesn't use a fixed width font.

Developing the script in the *Python expression script* editor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following steps can be used to iteratively develop your script using the *Python expression script* editor.

1. Edit the script.
2. Click the *Apply* button.
3. Go to *Variables->d* to change the variable to *d*.
   The first time you try your script this won't be necessary since the variable is already *d*.
4. Go to *Variables->MyExpression* to change the variable to "MyExpression" and execute the script.

Developing the script in a text editor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

The following steps can be used to iteratively develop your script using a text editor.

Before you can modify the script you will need to save it.

1. Click *Save script* to save the script.

Now you are ready to modify the script.

2. Edit the script with your favorit editor.
3. Go to *Load script->File* to load the script.
4. Click the *Apply* button.
5. Go to *Variables->d* to change the variable to *d*.
   The first time you try your script this won't be necessary since the variable is already *d*.
6. Go to *Variables->MyExpression* to change the variable to "MyExpression" and execute the script.
 
Python Expression Example 1
---------------------------

This example adds two cell centered variables.
It demonstrates accessing multiple variables and performing simple operations with them to generate a result.

Here is the example script.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 11-40

Let us start off by creating a Pseudocolor plot from the expression.

1. Copy the script into the *Python expression script* editor.
2. Click the *Apply* button.
3. Go to *Variables->MyExpression* to change the variable to the expression.

.. figure:: images/PythonExpressions-Plot1.png

   The Pseudocolor plot of MyExpression

Now let's take a look at the script and see what each portion does.

The ``__init__`` method provides information about the expression, including

* That it inherits from ``SimplePythonExpression``.
* The name of the expression.
* A description of the expression.
* A flag indicating that the output is not a point centered value.
* A flag that the output is a scalar.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 12-17

The ``modify_contract`` method can be used to request special information for the expression from VisIt_.
In this case it is a no-op.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 18-19

The ``derive_variable`` method performs the real work of the expression.
It is passed the input vtkDataSet and the domain_id.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 20-21

The following lines get the vtkDataArrays for the cell values for the two variables and the number of cells.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 22-27

The following lines set the output vtkDataArray to be an array of floats with 1 component and ncells values.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 28-31

Now we loop over the cells, setting the output value for each cell.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 32

The following lines add the two variables for the current cell.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 33-35

The following lines set the result value for the current cell.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 36-37

Once we have finished processing all the cells, we return the vtkDataArray.

.. literalinclude:: ../../test/tests/hybrid/python_example_1.vpe
   :lines: 38

Using your Python Expression with the CLI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Python expression we just created can also be used with the CLI.

We'll start by saving the script we just created.

1. Click *Save script* and save the script with the name "MyExpression.py".

The following script will open "curv2d.silo" and create a Pseudocolor plot of the expression.

.. code-block:: python

  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")

  DefinePythonExpression("MyExpression", ['d', 'p'], file="MyExpression.py")

  AddPlot("Pseudocolor", "MyExpression")
  DrawPlots()

Python Expression Example 2
---------------------------

This example operates on 2D meshes and takes the distance around the edges of each cell and multiplies it by the value of the cell.
It demonstrates accessing the coordinates and topology of the mesh as well as a variable.

Here is the example script.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 11-54

Let us start off by creating a Pseudocolor plot from the expression.

1. Copy the script into the *Python expression script* editor.
2. Click the *Apply* button.
3. Go to *Variables->MyExpression* to change the variable to the expression.

.. figure:: images/PythonExpressions-Plot2.png

   The Pseudocolor plot of MyExpression

The ``__init__`` and ``modify_contract`` methods are the same as the previous example, so we will only look at the ``derive_variable`` method.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 21-22

The following lines get the vtkDataArray for the cell values and the number of cells.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 23-26

The following lines set the output vtkDataArray to be an array of floats with 1 component and ncells values.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 27-30

Now we loop over the cells, setting the output value for each cell.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 31

The following lines get the current cell and the number of edges in the cell.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 32-35

Now we loop over the edges, calculating the sum of the lengths of the edges.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 36-38

We calculate the length of the edge from the 3D coordinates of the end points of the edge, which we add to the sum.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 39-47

Once we have summed the lengths of the edges we multiply the sum by the cell value and set it in the result.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 48-51

Once we have finished processing all the cells, we return the vtkDataArray.

.. literalinclude:: ../../test/tests/hybrid/python_example_2.vpe
   :lines: 52

Using your Python Expression with the CLI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This Python expression can also be used with the CLI, just as the one in the first example, except the specification of the variables to use is slightly different.
Since you are only passing a single variable you would use ``("d")`` for the list of variables.

.. code-block:: python

  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")

  DefinePythonExpression("MyExpression", ("d"), file="MyExpression.py")

  AddPlot("Pseudocolor", "MyExpression")
  DrawPlots()

Using VTK in Python
-------------------

The VTK Python interface mirrors the C++ interface.

To find out information on a particular VTK class, type the name of the class in your favorite search engine.

Here are links to some VTK classes that will be of most use to you.

* `vtkCell <https://vtk.org/doc/nightly/html/classvtkCell.html>`_
* `vtkDataArray <https://vtk.org/doc/nightly/html/classvtkDataArray.html>`_
* `vtkDataSet <https://vtk.org/doc/nightly/html/classvtkDataSet.html>`_
* `vtkDoubleArray <https://vtk.org/doc/nightly/html/classvtkDoubleArray.html>`_
* `vtkFloatArray <https://vtk.org/doc/nightly/html/classvtkFloatArray.html>`_
