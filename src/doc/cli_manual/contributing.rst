Contributing To VisIt CLI Documentation
=======================================

**Note: This procedure is planned for change in the future.**

At present, all VisIt Python CLI documentation is actually composed directly
as Python strings in the source C++ file in ``../../visitpy/common/MethodDoc.C``.

We recognize this isn't the most convenient way to write documentation and are
planning on changing it in the future. However, it does permit us to have a 
single source file for documentation which is then used to provide ``help(func)``
at the Python prompt as well as generate the restructured text used here.

In the future, we will swap this arrangement and write documentation in 
restructured text and then generate the contents of ``../../visitpy/common/MethodDoc.C``
from the restructured text.

The documentation here is then generated from the ``MethodDoc.C`` file using the script
``../sphinx_cli_extractor.py``. That script produces the following files...

* ``attributes.rst``
* ``events.rst``
* ``functions.rst``

The other ``.rst`` files here are manually managed and can be modified as needed.

Steps to update the CLI Manual
------------------------------

#. Modify ``../../visitpy/common/MethodDoc.C`` as needed
#. Build and run the VisIt_ cli and assure yourself ``help(<your-new-func-doc>)``
   produces the desired output 
#. Run the ``sphinx_cli_extractor.py`` tool producing new ``attributes.rst``,
   ``events.rst`` and ``functions.rst`` files. To do so, you need to tell it
   where to find VisIt_'s ``site-packages`` Python directory. On my OSX machine,
   the command would look like...

.. code-block:: shell

    env VISIT_SITE_PACKAGES=/Applications/VisIt.app/Contents/Resources/2.13.3/darwin-x86_64/lib/python/lib/python2.7/site-packages/ ./sphinx_cli_extractor.py

#. Do a local build of the documentation here and confirm there are no errors
   in the build

.. code-block:: shell

    sphinx-build -b html . _build -a

#. Add all the changes files to a commit and push to GitHub
#. The GitHub integration with ReadTheDocs should result in your documentation
   updates going live a short while (<15 mins) later.
