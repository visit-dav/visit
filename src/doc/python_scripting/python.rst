Python
======

Overview
--------

Python is a general purpose, interpreted, extensible, object-oriented
scripting language that was chosen for VisIt’s scripting language due to
its ease of use and flexibility. VisIt’s Python interface was
implemented as Python module and it allows you to enhance your Python
scripts with coding to control VisIt. This chapter explains some of
Python’s syntax so it will be more familiar when you examine the
examples found in this document. For more information on programming in
Python, there are a number of good references, including on the Internet
at http://www.python.org.

Indentation
-----------

One of the most obvious features of Python is its use of indentation for
new scopes. You must take special care to indent all program logic
consistently or else the Python interpreter may halt with an error, or
worse, not do what you intended. You must increase indentation levels
when you define a function, use an if/elif/else statement, or use any
loop construct.

Note the different levels of indentation:

::

    def example_function(n): 
      v = 0 
      if n > 2: 
        print "n greater than 2." 
      else: 
        v = n * n 
      return v

Comments
--------

Like all good programming languages, Python supports the addition of
comments in the code. Comments begin with a pound character (#) and
continue to the end of the line.

::

    # This is a comment 
    a = 5 * 5 

Identifiers
-----------

The Python interpreter accepts any identifier that contains letters
’A’-’Z’, ’a’-’z’ and numbers ’0’-’9’ as long as the identifier does not
begin with a number. The Python interpreter is case-sensitive so the
identifier “case" would not be the same identifier as “CASE". Be sure to
case consistently throughout your Python code since the Python
interpreter will instantiate any identifier that it has not seen before
and mixing case would cause the interpreter to use multiple identifiers
and cause problems that you might not expect. Identifiers can be used to
refer to any type of object since Python is flexible in its treatment of
types.

Data types
----------

Python supports a wide variety of data types and allows you to define
your own data types readily. Most types are created from a handful of
building-block types such as integers, floats, strings, tuples, lists,
and dictionaries.

Strings
~~~~~~~

Python has built-in support for strings and you can create them using
single quotes or double quotes. You can even use both types of quotes so
you can create strings that include quotes in case quotes are desired in
the output. Strings are sequence objects and support operations that can
break them down into characters.

::

    s = 'using single quotes' 
    s2 = "using double quotes" 
    s3 = 'nesting the "spiffy" double quotes'

Tuples
~~~~~~

Python supports tuples, which can be thought of as a read-only set of
objects. The members of a tuple can be of different types. Tuples are
commonly used to group multiple related items into a single object that
can be passed around more easily. Tuples support a number of operations.
You can subscript a tuple like an array to access its individual
members. You can easily determine whether an object is a member of a
tuple. You can iterate over a tuple. There are many more uses for
tuples. You can create tuples by enclosing a comma-separated list of
objects in parenthesis.

::

    # Create a tuple 
    a = (1,2,3,4,5) 
    print "The first value in a is:", a[0] 
    # See if 3 is in a using the "in" operator. 
    print "3 is in a:", 3 in a 
    # Create another tuple and add it to the first one to create c. 
    b = (6,7,8,9) 
    c = a + b 
    # Iterate over the items in the tuple 
    for value in c: 
      print "value is: ", value 

Lists
~~~~~

Lists are just like tuples except they are not read-only and they use
square brackets [] to enclose the items in the list instead of using
parenthesis.

::

    # Start with an empty list. 
    L = [] 
    for i in range(10): 
      # Add i to the list L 
      L = L + [i] 
    print L 
    # Assign a value into element 6 
    L[5] = 1000 
    print L 

Dictionaries
~~~~~~~~~~~~

Dictionaries are Python containers that allow you to store a value that
is associated with a key. Dictionaries are convenient for mapping 1 set
to another set since they allow you to perform easy lookups of values.
Dictionaries are declared using curly braces and each item in the
dictionary consists of a key: value pair with the key and values being
separated by a colon. To perform a lookup using a dictionary, provide
the key whose value you want to look up to the subscript [] operator.

::

    colors = {"red" : "rouge", "orange" : "orange", \
    "yellow" : "jaune", "green" : "vert", "blue" : "bleu"}
    # Perform lookups using the keys. 
    for c in colors.keys(): 
       print "%s in French is: %s" % (c, colors[c]) 

NumPy Arrays
~~~~~~~~~~~~

VisIt_ is often used together with NumPy.
It is natural to want to use NumPy arrays in assignments to members of VisIt_ attribute and dictionary objects or as arguments to functions.
For the most part, NumPy arrays do work in assignment to members of attribute objects or via setter functions on attribute objects.
In cases where entries in a NumPy array are to be interpreted as arguments to a function, it is possible to *dereference* a NumPy array by preceding it with ``*``.
For example, this does not work ::

    NumPyArray = numpy.array([1.1,2.2,3.3,4.4])
    va = VolumeAttributes()
    va.SetMaterialProperties(NumPyArray)

whereas this does ::

    NumPyArray = numpy.array([1.1,2.2,3.3,4.4])
    va = VolumeAttributes()
    va.SetMaterialProperties(*NumPyArray)

as well as this ::

    NumPyArray = numpy.array([1.1,2.2,3.3,4.4])
    va = VolumeAttributes()
    va.materialProperties = NumPyArray

Whenever passing NumPy arrays to VisIt_ is causing problems, it is possible to *convert* them to *list* objects using the ``.tolist()`` method as in ::

    NumPyArray = numpy.array([1.1,2.2,3.3,4.4])
    va = VolumeAttributes()
    va.SetMaterialProperties(NumPyArray.tolist())

Control flow
------------

Python, like other general-purpose programming languages provides
keywords that implement control flow. Control flow is an important
feature to have in a programming language because it allows complex
behavior to be created using a minimum amount of scripting.

if/elif/else
~~~~~~~~~~~~

Python provides if/elif/else for conditional branching. The if statement
takes any expression that evaluates to an integer and it takes the if
branch if the integer value is 1 other wise it takes the else branch if
it is present.

::

    # Example 1 
    if condition: 
         do_something() 

    # Example 2 
    if condition: 
         do_something() 
    else: 
         do_something_else() 

    # Example 3 
    if condition: 
         do_domething() 
    elif conditionn: 
         do_something_n() 
    else: 
         do_something_else() 

For loop
~~~~~~~~

Python provides a for loop that allows you to iterate over all items
stored in a sequence object (tuples, lists, strings). The body of the
for loop executes once for each item in the sequence object and allows
you to specify the name of an identifier to use in order to reference
the current item.

::

    # Iterating through the characters of a string 
    for c in "characters": 
       print c 

    # Iterating through a tuple 
    for value in ("VisIt", "is", "coolness", "times", 100): 
       print value 

    # Iterating through a list 
    for value in ["VisIt", "is", "coolness", "times", 100]: 
       print value 

    # Iterating through a range of numbers [0,N) created with range(N). 
    N = 100 
    for i in range(N): 
       print i, i*i

While loop
~~~~~~~~~~

Python provides a while loop that allows you to execute a loop body
indefinitely based on some condition. The while loop can be used for
iteration but can also be used to execute more complex types of loops.

::

    token = get_next_token() 
    while token != "": 
      do_something(token) 
      token = get_next_token() 

Functions
---------

Python comes with many built-in functions and modules that implement
additional functions. Functions can be used to execute bodies of code
that are meant to be re-used. Functions can optionally take arguments
and can optionally return values. Python provides the def keyword, which
allows you to define a function. The def keyword is followed by the name
of the function and its arguments, which should appear as a tuple next
to the name of the function.

::

    # Define a function with no arguments and no return value. 
    def my_function(): 
        print "my function prints this..."

    # Define a function with arguments and a return value. 
    def n_to_the_d_power(n, d): 
        value = 1 
        if d > 0: 
            for i in range(d): 
                value = value * n 
        elif d < 0: 
            value = 1. / float(n_to_the_d_power(n, -d)) 

    return value

.. _finding_stuff_from_python:

Finding Stuff from Python Prompt
--------------------------------

If you are having trouble finding the right functions or objects, you can use ``apropos(regex)``, where regex is a regular expression string, and you will get back a list of all objects and functions whose names, doc strings or stringified instances (for objects only) match the regular expression.
For example, 

::

    >>> apropos("icosahedron")
    ['SubsetAttributes', 'ScatterAttributes', 'PseudocolorAttributes', 'FilledBoundaryAttributes', 'MeshAttributes']
    >>> apropos(".*mir.*")
    ['SetDefaultMaterialAttributes', 'MaterialAttributes', 'GetMaterialAttributes', 'SetMaterialAttributes']
    >>> apropos(".*reconstruct.*")
    ['GetMeshManagementAttributes', 'SetDefaultMeshManagementAttributes', 'SetMeshManagementAttributes', 'SetDefaultMaterialAttributes', 'GetMaterialAttributes', 'SetMaterialAttributes']

``apropos()`` also always does case-insensitive searches.

In `Python Regular Expressions <https://docs.python.org/3/library/re.html>`_ the ``.*`` is needed for an arbitrary number of unspecified characters.
See `this HOWTO <https://docs.python.org/3/howto/regex.html>`_ for more information about Python Regular Expressions.
The function name ``apropos`` was inspired by a `function of similar name and purpose <https://en.wikipedia.org/wiki/Apropos_(Unix)>`_ in the Unix operating system. 

Just use ``help()``
~~~~~~~~~~~~~~~~~~~

One drawback with using Python's *standard* ``help(thing)`` facility is that it requires prior knowledge of the name(s) (including correct capitalization) of the thing.
Consequently, VisIt_'s Python environment adjusts the default behavior of ``help(thing)`` in a couple of ways.
First, it accepts string arguments and then returns the result of ``apropos(thing)``.
Second, for non-string arguments it will attempt to present output from Python's *standard* help but then to also follow that up with the output of ``apropos(thing)``.
For example, ``help("materials")`` (e.g. passing a string to ``help()``) produces...

::

    >>> help("materials")
    NOTE: The following VisIt functions and objects also mention 'materials'...
    ['GetActiveTimeSlider', 'GetMaterialAttributes', 'GetMaterials', 'ListDomains', 'ListMaterials', 'MaterialAttributes', 'SetViewExtentsType', 'TurnDomainsOff', 'TurnDomainsOn', 'TurnMaterialsOff', 'TurnMaterialsOn']

whereas passing the name of a function such as ``TurnMaterialsOn`` produces the help for that function followed by a list of other items that mention that funciton...

::

    >>> help(TurnMaterialsOn)
    Help on built-in function TurnMaterialsOn:


    TurnMaterialsOn(...)
        TurnMaterialsOn

    .
    .
    .

    NOTE: The following VisIt functions and objects also mention 'TurnMaterialsOn'...
    ['TurnDomainsOff', 'TurnDomainsOn', 'TurnMaterialsOff', 'TurnMaterialsOn']

For another example,

::

    >>> help("copy")

    Help on module copy:

    NAME
        copy - Generic (shallow and deep) copying operations.

    MODULE REFERENCE
        https://docs.python.org/3.7/library/copy
    
        The following documentation is automatically generated from the Python
        source files.  It may be incomplete, incorrect or include features that
        location listed above.

    DESCRIPTION
        Interface summary:
    
                import copy
    
                x = copy.copy(y)        # make a shallow copy of y
                x = copy.deepcopy(y)    # make a deep copy of y
    
        For module specific errors, copy.Error is raised.

    .
    .
    .

    NOTE: The following VisIt functions and objects also mention 'copy'...
    ['CopyAnnotationsToWindow', 'CopyLightingToWindow', 'CopyPlotsToWindow', 'CopyViewToWindow', 'GetPlotList', 'InitializeNamedSelectionVariables']

If there is a need to bypass VisIt_'s override of ``help()``, ``python_help()`` is an alias for Python's *default* ``help()``.
Likewise, ``visit_help()`` is an alias for VisIt_'s overridden ``help()``.

Use lsearch to limit search results
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VisIt_'s *CLI* provides a large set of functions.
The scope of a search can be limited using the ``lsearch()`` helper function in the visit_utils module: ::

    from visit_utils.common import lsearch
    lsearch(dir(),"Material")
    lsearch(apropos("subset"),"domain"))

*lsearch()* returns a python list of strings with the names that match the given pattern.
Here is another example that prints each of the result strings on a separate line. ::

    from visit_utils.common import lsearch
    for value in lsearch(dir(),"Material"):
        print value
