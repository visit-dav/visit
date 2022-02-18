Contributing To VisIt CLI Documentation
=======================================

.. note::
   We are still refining this procedure!

The Python doc strings for most functions in VisIt's cli are generated
from the examples embedded in the ``cli_manual/functions.rst`` file.
This allows us to have a single source for both our CLI sphinx docs 
and the doc strings embedded in VisIt's compiled Python module. 
The ``functions_to_method_doc.py`` helper script generates ``MethodDoc.C``
from the examples embedded in the rst source.


The Python doc strings for Attribute objects and Events are extracted from the CLI 
for use in the CLI sphinx docs.  The ``sphinx_cli_extractor.py`` runs VisIt to 
generate ``cli_manual/attributes.rst``  and ``cli_manual/events.rst``

Steps to update the CLI Manual
------------------------------

#. Modify ``cli_manual/functions.rst``
#. Run ``functions_to_plain_py.py`` to generate ``PY_RST_FUNCTIONS_TO_PYTHON.py``
#. Run ``2to3 -p PY_RST_FUNCTIONS_TO_PYTHON.py`` to check for Python syntax errors and Python 3 compatibly 
#. Run ``functions_to_method_doc.py`` to regenerate ``MethodDoc.C``
#. Build and run the VisIt_ cli and assure yourself ``help(<your-new-func-doc>)``
   produces the desired output

#. Run the ``sphinx_cli_extractor.py`` tool producing new ``attributes.rst``
   and ``events.rst`` files. To do so, you may need to use
   a combination of the ``PATH`` and ``PYTHONPATH`` environment variables to tell the
   ``sphinx_cli_extractor.py`` script where to find the VisIt_ module, ``visit`` in
   VisIt_'s ``site-packages`` and where to find the Python installation that that
   module is expecting to run with. In addition, you may need to use the ``PYTHONHOME``
   environment variable to tell VisIt_'s ``visit`` module where to find standard Python
   libraries. For example, to use an installed version of VisIt_ on my OSX machine,
   the command would look like...

   .. code-block:: shell

     env PATH=/Applications/VisIt.app/Contents/Resources/2.13.3/darwin-x86_64/bin:/Applications/VisIt.app/Contents/Resources/bin:$PATH \
     PYTHONHOME=/Applications/VisIt.app//Contents/Resources/2.13.3/darwin-x86_64/lib/python \
     PYTHONPATH=/Applications/VisIt.app/Contents/Resources/2.13.3/darwin-x86_64/lib/site-packages \
     ./sphinx_cli_extractor.py 

   Note that the above command would produce CLI documentation for version 2.13.3 of VisIt_.
   Or, to use a current build of VisIt_ on which you are working on documentation related
   to changes you have made to VisIt_, the command would look something like...

   .. code-block:: shell

     env PATH=../../build/third_party/python/2.7.14/i386-apple-darwin17_clang/bin:../../build/visit/build/bin:$PATH \
     PYTHONPATH=../../build/visit/build/lib/site-packages/ \
     ./sphinx_cli_extractor.py 

   The whole process only takes a few seconds.

#. Assuming you successfully ran the above command, producing new ``attributes.rst``
   and ``events.rst`` files, then do a local build of the
   documentation here and confirm there are no errors in the build

   .. code-block:: shell

     sphinx-build -b html . _build -a

#. Then open the file, ``_build/index.html``, in your favorite browser to view.
#. Add all the changed files to a commit and push to GitHub
#. The GitHub integration with ReadTheDocs should result in your documentation
   updates going live a short while (<15 mins) after it has been merged to develop.
