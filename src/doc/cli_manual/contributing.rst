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
``../sphinx_cli_extractor.py``. That script produces ``attributes.rst``, ``events.rst``
and ``functions.rst`` files. The other ``.rst`` files here are manually managed and can
be modified normally as needed.

Steps to update the CLI Manual
------------------------------

#. Modify ``../../visitpy/common/MethodDoc.C`` as needed
#. Build and run the VisIt_ cli and assure yourself ``help(<your-new-func-doc>)``
   produces the desired output 
#. Run the ``sphinx_cli_extractor.py`` tool producing new ``attributes.rst``,
   ``events.rst`` and ``functions.rst`` files. To do so, you may need to use
   a combination of the ``PATH`` and ``PYTHONPATH`` environment variables to tell the
   ``sphinx_cli_extractor.py`` script where to find the VisIt_ module, ``visit`` in
   VisIt_'s ``site-packages`` and where to find the Python installation that that
   module is expecting to run with. In addition, you may need to use the ``PTHONHOME``
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

#. Assuming you succesfully run the above command, producing new ``attributes.rts``,
   ``events.rst`` and ``functions.rst`` files, then do a local build of the
   documentation here and confirm there are no errors in the build

   .. code-block:: shell

     sphinx-build -b html . _build -a

#. Add all the changed files to a commit and push to GitHub
#. The GitHub integration with ReadTheDocs should result in your documentation
   updates going live a short while (<15 mins) after it has been merged to develop.
