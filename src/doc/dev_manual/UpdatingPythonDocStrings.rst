Updating the Python Doc Strings
===============================

.. note::
   We are still refining this procedure!

The Python doc strings for most functions in VisIt_'s scripting interface are generated from the examples embedded in the ``python_scripting/functions.rst`` file.
This allows us to have a single source for both our scripting interface sphinx docs and the doc strings embedded in VisIt_'s compiled Python module. 
The ``functions_to_method_doc.py`` helper script generates ``MethodDoc.C`` from the examples embedded in the rst source.

When editing any of these ``.rst`` files, it is important to maintain consistency with existing formatting.
For each function documented there, it is important to use the same heading levels with the same heading names, the same indentation, etc., etc.
If there is nothing to include for a particular heading, still including the heading but with a body indicating ``Nothing more`` or something to that effect.

The Python doc strings for Attribute objects and Events are extracted from the scripting interface for use in the Python scripting sphinx docs.
The ``sphinx_cli_extractor.py`` runs VisIt to generate ``python_scripting/attributes.rst``  and ``python_scripting/events.rst``.

Steps to update the Python scripting manual
-------------------------------------------

#. Modify ``python_scripting/functions.rst``.
#. Run ``functions_to_plain_py.py`` to generate ``PY_RST_FUNCTIONS_TO_PYTHON.py``.
#. Run ``2to3 -p PY_RST_FUNCTIONS_TO_PYTHON.py`` to check for Python syntax errors and Python 3 compatibly.

   * **NOTE:** ``PY_RST_FUNCTIONS_TO_PYTHON.py`` is just a temporary file to test steps 2 and 3 here.
     It could be named anything and is not part of the repository.
   * **NOTE:** ``2to3`` will run to completion and issue a number of messages.
     A zero return code indicates all is well.

#. Run ``functions_to_method_doc.py`` to regenerate ``MethodDoc.C``.
#. Build and run the VisIt_ scripting interface and assure yourself ``help(<your-new-func-doc>)`` produces the desired output.
#. Run the ``sphinx_cli_extractor.py`` tool producing new ``attributes.rst`` and ``events.rst`` files.
   To do so, you may need to use a combination of the ``PATH`` and ``PYTHONPATH`` environment variables to tell the ``sphinx_cli_extractor.py`` script where to find the VisIt_ module, ``visit`` in VisIt_'s ``site-packages`` and where to find the Python installation that that module is expecting to run with.
   In addition, you may need to use the ``PYTHONHOME`` environment variable to tell VisIt_'s ``visit`` module where to find standard Python libraries.
   For example, to use an installed version of VisIt_ on my macOS machine, the command would look like...

   .. code-block:: shell

     env PATH=/Applications/VisIt.app/Contents/Resources/2.13.3/darwin-x86_64/bin:/Applications/VisIt.app/Contents/Resources/bin:$PATH \
     PYTHONHOME=/Applications/VisIt.app//Contents/Resources/2.13.3/darwin-x86_64/lib/python \
     PYTHONPATH=/Applications/VisIt.app/Contents/Resources/2.13.3/darwin-x86_64/lib/site-packages \
     ./sphinx_cli_extractor.py 

   Note that the above command would produce CLI documentation for version 2.13.3 of VisIt_.
   Or, to use a current build of VisIt_ on which you are working on documentation related to changes you have made to VisIt_, the command would look something like...

   .. code-block:: shell

     env PATH=/Users/miller86/visit/third_party/3.2.0/python/3.7.7/i386-apple-darwin18_clang/bin:/Users/miller86/visit/visit/build/bin:$PATH \
     PYTHONPATH=../../build/lib/site-packages python3 ./sphinx_cli_extractor.py

   The whole process only takes a few seconds.

#. Assuming you successfully ran the above command, producing new ``attributes.rst`` and ``events.rst`` files, then do a local build of the documentation here and confirm there are no errors in the build.

   .. code-block:: shell

     sphinx-build -b html . _build -a

#. Then open the file, ``_build/index.html``, in your favorite browser to view.
#. Add all the changed files to a commit and push to GitHub.
#. The GitHub integration with ReadTheDocs should result in your documentation updates going live a short while (<15 mins) after it has been merged to develop.
