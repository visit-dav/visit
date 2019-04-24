.. _Contributing:

Contributing
============

This is a short contributing guide on the VisIt_ project's use of
`Sphinx <http://www.sphinx-doc.org/en/stable/tutorial.html>`_ for
documentation.

You can check out the Sphinx manual with::

    svn co svn+ssh://<USERNAME>@edison.nersc.gov/project/projectdirs/visit/svn/visit/trunk/docs/SphynxDocs

If you have `Sphinx <http://www.sphinx-doc.org/en/stable/tutorial.html>`_ You can
build the html manual locally using the command::

    sphinx-build -b html . _build -a

You can then browse the root of the manual by pointing your browser to
:file:`./_build/index.html`.  The ``-a`` forces a re-build of everything.
Remove it when you are constantly revising and rebuilding.

Your changes to any files in :file:`trunk/docs/SphinxDocs` will go live
`here <http://visit-sphinx-user-manual.readthedocs.io/en/latest/index.html>`_
at approximately 50 minutes passed every even numbered hour.

Quick Reference
---------------
Note that the original source of most of the content here is the OpenOffice
document produced with heroic effort by Brad Whitlock. A conversion tool was
used to move most of the content there to Sphinx. As such, most of the Sphinx
usage conventions adopted here were driven by whatever the conversion tool
produced.  There are numerous opportunities for adjusting this to make better
use of Sphinx as we move forward. These are discussed at the
:ref:`end <contributing_forward>` of this section.

* A few documents about reStructuredText and Sphinx are useful:

  * `reStructuredText Primer <http://docutils.sourceforge.net/docs/user/rst/quickref.html>`_
  * `Sphinx Documentation <http://www.sphinx-doc.org/en/stable/contents.html>`_
  * `reStructuredText Markup Specification <http://docutils.sourceforge.net/docs/ref/rst/restructuredtext.html>`_
  * `reStructuredText Reference Documentation <http://docutils.sourceforge.net/rst.html#reference-documentation>`_

* Sphinx uses blank lines as block separators and 2 or 4 spaces of
  indentation to guide parsing and interpretation of content. So, be sure
  to pay careful attention to blank lines and indentation. They are not
  there merely for style.  They *need* to be there for Sphinx to parse and
  interpret the content correctly.
* Line breaks *within* reStructuredText inline markup constructs often cause
  build errors. 
* Create headings by a sequence of *separator characters* immediately
  underneath and the same length as the heading. Different types of
  separator characters define different levels of headings ::

    First Level Heading
    ===================
    This is an example of some text under the heading...

    Second Level Heading
    --------------------
    This is an example of some text under the heading...

    Third Level Heading
    ~~~~~~~~~~~~~~~~~~~
    This is an example of some text under the heading...

    Fourth level heading
    """"""""""""""""""""
    This is an example of some text under the heading...

  yields these headings...

.. figure:: images/headings.png

* If you want to divide sections and subsections across multiple ``.rst``
  files, you can link them together using the ``.. toctree::`` directive
  as is done for example in the section on VisIt_ **Plots** ::

    Plots
    =====
 
    This chapter explains the concept of a plot and goes into detail
    about each of VisIt's different plot types.
 
    .. toctree::
        :maxdepth: 1
 
        Working_with_Plots
        PlotTypes/index

  Note that the files listed in the ``.. toctree::`` block do not include
  their ``.rst`` extensions.

* Wherever possible, keep lines in ``.rst`` files to 80 columns or less.
* Avoid contractions such as ``isn't``, ``can't`` and ``you've``.
* Avoid hyphenation of words.
* Use ``VisIt_`` or ``VisIt_'s`` when referring to VisIt_ by name.
* Use upper case for all letters in acronyms (:abbr:`MPI (Message Passing Interface)`, VTK)
* Use case conventions of product names (QuickTime, TotalView, Valgrind).
* Bracket word(s) with two stars (``**some words**``) for **bold**.
* Bracket word(s) with one star (``*word*``) for *italics*.
* Bracket word(s) with two backticks (:samp:`\ ``some words```) for ``literal``.
* Bracketed word(s) should not span line breaks.
* Use **bold** to refer to VisIt_ **Widget** names, **Operator** or **Plot**
  names and other named objects part of VisIt_'s interface.
* Use the following terminology when referring to widget names.

.. figure:: images/GUIWidgetNames.png

.. figure:: images/GUIWidgetNames2.png

* Avoid use of **bold** for other purposes. Instead use *italics*.
* Use ``literals`` for code, commands, arguments, file names, etc.
* Use :samp:`\ :t\ erm:`glossary term`` at least for the *first* use of a
  glossary term in a section.
* Use :samp:`\ :a\ bbr:`ABR (Long Form)`` at least for the *first* use of an
  acronym or abbreviation in a section.
* Subscripting, H\ :sub:`2`\ O, and superscripting, E = mc\ :sup:`2`, are supported::

    Subscripting, H\ :sub:`2`\ O, and superscripting, E = mc\ :sup:`2`, are supported

  Note the use of backslashed spaces so Sphinx treats it all as one word.
* Use ``.. figure::`` and not ``.. image::``, include captions with figures
  and use ``:scale: P %`` to adjust image size where needed
  (:ref:`see more below <contributing_images>`).
* LaTeX style equations can be included too
  (:ref:`see below <contributing_math>`).
* Spell checking is supported too (:ref:`see below <contributing_spell>`) but
  you need to have 
  `PyEnchant <https://pythonhosted.org/pyenchant/>`_ and
  `sphinx-contrib.spelling <http://sphinxcontrib-spelling.readthedocs.io/en/latest/index.html>`_
  installed.
* Begin a line with ``..`` followed by space for single line comments::

    .. this is a single line comment

    ..
        This is a multi-line
        comment

.. _my_anchor:

* Define anchors ahead of sections or paragraphs you want to cross reference::

    .. _my_anchor:

    Section Heading
    ---------------

  Note that the leading underscore is **not** part of the anchor name.
* Make anchor names unique over all pages of documentation by using
  the convention of prepending heading and subheading names.
* Link to anchors *within* this documentation like :ref:`this one <my_anchor>`::

    Link to anchors *within* this documentation like :ref:`this one <my_anchor>`

* Link to other documents elsewhere online like
  `visitusers.org <https://www.visitusers.org/>`_::

    Link to other documents elsewhere online like
    `visitusers.org <https://www.visitusers.org/>`_

* Link to *numbered* figures or tables *within* this documentation like
  :numref:`Fig. %s <my_figure2>`::

    Link to *numbered* figures or tables *within* this documentation like
    :numref:`Fig. %s <my_figure2>`

* Link to a downloadable file *within* this documentation like
  :download:`this one <../Quantitative/VerdictManual-revA.pdf>`::

    Link to a downloadable file *within* this documentation like
    :download:`this one <../Quantitative/VerdictManual-revA.pdf>`

* If you are having trouble getting the formatting for a section worked
  out and the time involved to re-gen the documentation is too much, you
  could try an
  `on-line, real-time reStructuredText Renderer <http://rst.ninjs.org>`_
  to quickly try different things and see how they work.

.. _contributing_images:

More on Images
--------------

Try to use PNG formatted images. We plan to use the Sphinx generated
documentation both for online HTML and for printed PDF. So, images sizes
cannot be too big or they will slow HTML loads but not so small they are
unusable in PDF.

Some image formats wind up enforcing **physical** dimensions instead of
just pixel dimensions. This can have the effect of causing a nicely sized
image (from pixel dimensions perspective anyways), to either be unusually
large or unusually small in HTML or PDF output. In these cases, you can
use the Sphinx ``:scale:`` and ``:width:`` or ``:height:`` options for
a ``.. figure::`` block. Also, be sure to use a ``.. figure::`` directive
instead of an ``.. image::`` directive for embedding images. This is because
the ``.. figure::`` directive also supports anchoring for cross referencing.

Although all images get copied into a common directory during generation,
Sphinx takes care of remapping names so there is no need to worry about
collisions in image file names potentially used in different subdirectories
within the source tree.

An ordinary image...

.. code-block:: RST

  .. figure:: images/array_compose_with_bins.png

.. figure:: images/array_compose_with_bins.png

Same image with ``:scale: 50%`` option

.. code-block:: RST

  .. figure:: images/array_compose_with_bins.png
     :scale: 50% 

.. figure:: images/array_compose_with_bins.png
   :scale: 50% 

Same image with an anchor for cross referencing...

.. code-block:: RST

  .. _my_figure:

  .. figure:: images/array_compose_with_bins.png
     :scale: 50% 

.. _my_figure:

.. figure:: images/array_compose_with_bins.png
   :scale: 50% 

which can now be cross referenced using an inline :numref:`Fig. %s <my_figure>` 
like so...

.. code-block:: RST

  Which can now be cross referenced using an inline :numref:`Fig. %s <my_figure>` 
  like so...

Note the anchor has a leading underscore which the reference does not include.

Same image (different anchor though because anchors need to be unique) with
a caption.

.. code-block:: RST

  .. _my_figure2:

  .. figure:: images/array_compose_with_bins.png
     :scale: 50% 

     Here is a caption for the figure.

.. _my_figure2:

.. figure:: images/array_compose_with_bins.png
   :scale: 50% 

   Here is a caption for the figure.

Note that the figure label (e.g. Fig 20.2) will not appear if there is no
caption.

Tables
------
Sphinx supports a variety of mechanisms for defining tables. The conversion
tool used to convert this documentation from its original OpenOffice format
converted all tables to the *grid* style of table which is kinda sorta like
ascii art. Large tables can result in individual lines that span many widths of
the editor window. It is cumbersome to deal with but rich in capabilities.

.. _contributing_math:

Math
----

We add the Sphinx builtin extension ``sphinx.ext.mathjax`` to the
``extensions`` variable in ``conf.py``. This allows Sphinx to use
`mathjax <https://www.mathjax.org>`_ to do LaTeX like math equations in our
documentation. For example, this LaTeX code

.. code-block:: RST

  :math:`x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}`

produces...

:math:`x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}`

You can find a few examples in :ref:`Expressions <Expressions>`. Search
there for `:math:`. Also, this
`LaTeX Wiki page <https://oeis.org/wiki/List_of_LaTeX_mathematical_symbols>`_
has a lot of useful information on various math symbols available in LaTeX
and `this wiki book <https://en.wikibooks.org/wiki/LaTeX/Mathematics>`_ has
a lot of guidance on constructing math equations with LaTeX.

.. _contributing_spell:

Spell Checking Using Aspell
---------------------------
You can do a pretty good job of spell checking using the Unix/Linux ``aspell``
command.

#. Run ``aspell`` looking for candidate miss-spelled words.

   .. code-block:: shell

       find . -name '*.rst' -exec cat {} \; | \
       grep -v '^ *.. image:\|figure:\|code:\|_' | \
       tr '`' '@' | sed -e 's/\(@.*@\)//' | \
       aspell -p ./aspell.en.pws list | \
       sort | uniq > maybe_bad.out

   The ``find`` command will find all ``.rst`` files. Succeeding ``grep``,
   ``tr`` and ``sed`` pipes filter some of the ``.rst`` syntax away. The final
   pipe through ``aspell`` uses the
   `personal word list (also called the personal dictionary) <http://aspell.net/man-html/Format-of-the-Personal-and-Replacement-Dictionaries.html#Format-of-the-Personal-Dictionary-1>`_
   option, ``-p ./aspell.en.pws`` (**note:** the ``./`` is critical so don't
   ignore it), to specify a file containing a list of words we allow that
   ``aspell`` would otherwise flag as incorrect. The ``sort`` and ``uniq``
   pipes ensure the result doesn't contain duplicates. But, be aware that a
   given miss-spelling can have multiple occurrences. The whole process produces
   a list of candidate miss-spelled words in ``maybe_bad.out``.

#. Examine ``maybe_bad.out`` for words that you think are correctly spelled.
   If you find any, remove them from ``maybe_bad.out`` and add them to the end
   of ``aspell.en.pws`` being careful to update the total word count in the
   first line of file where, for example ``572`` is the word count shown in
   that line, ``personal_ws-1.1 en 572`` when this was written.

#. To find instances of remaining (miss-spelled words), use the following
   command.

   .. code-block:: shell

      find . -name '*.rst' -exec grep -wnHFf maybe_bad.out {} \;

#. It may be necessary to iterate through these steps a few times to find
   and correct all the miss-spellings.

It would be nice to create a ``make spellcheck`` target that does much of
the above automatically. However, that involves implementing the above 
steps as a ``cmake`` program and involves more effort than available when
this was implemented.

.. _contributing_forward:

Things To Consider Going Forward
--------------------------------

* Decide what to do about compound words such as *timestep*, *time step* or
  *time-step*. There are many instances to consider such as *keyframe*,
  *checkbox*, *pulldown*, *submenu*, *sublauncher*, etc.
* Need to populate glossary with more VisIt_ specific terms such as...

 * Mixed materials, Species, OnionPeel,  Mesh, Viewer, cycle, timestep
   Client-server, CMFE, Zone-centering, Node-centering, etc.

* Decide upon and then make consistent the usage of terms like
  *zone*/*cell*/*element* and *node*/*point*/*vertex*
* We will need to support *versions* of the manual with each release.
  RTD can do that. We just need to implement it.

  * If we have tagged content, then those would also represent different
    *versions* of the manual.

* All VisIt_ manuals should probably be hosted at a URL like
  ``visit.readthedocs.io`` and from there users can find manuals for GUI, CLI
  Getting Data Into VisIt_, etc.
* Change name of docs dir to ``Sphinx`` and not ``Sphynx``.
* Add at least another LLNL person to RTD project so we have coverage to fix
  issues as they come up.
* Additional features of Sphinx to consider adopting...

  * ``:guilable:`` role for referring to GUI widgets.
  * ``:command:`` role for OS level commands.
  * ``:file:`` role for referring to file names.
  * ``:menuselection:`` role for referring to widget paths in GUI menus.
    Example: :menuselection:`Controls --> View --> Advanced`.
  * ``:kbd:`` role for specifying a sequence of key strokes.
  * ``.. deprecated::`` directive for deprecated functionality
  * ``.. versionadded::`` directive for new functionality
  * ``.. versionchanged::`` directive for when functionality changed
  * ``.. note::``, ``.. warning::`` and/or ``.. danger::`` directives to call
    attention to the reader.
  * ``.. only::`` directives for audience specific (e.g. tagged) content
  
    * Could use to also include developer related content but have it
      not appear in the user manual output

  * ``.. seealso::`` directive for references
  * Substitutions for names of products and projects we refer to frequently
    such as VTK_ or VisIt_ (as is used throughout this section) or for
    frequently used text such as |viswin|::
  
      Substitutions for names of products and projects we refer to frequently
      such as VTK_ or VisIt_ (as is used throughout this section) or for
      frequently used text such as |viswin|.

    with the following substitutions defined::

      .. _VisIt: https://visit.llnl.gov
      .. _VTK: https://www.vtk.org
      .. |viswin| replace:: **Viewer Window**

    Note that the ``.. _VisIt: ...`` substitution is already defined for the whole
    doctree in the ``rst_prolog`` variable in ``conf.py``.

.. _VisIt: https://visit.llnl.gov
.. _VTK: https://www.vtk.org
.. |viswin| replace:: **Viewer Window**

* Possible method for embedding python code to generate and capture images
  (both of the GUI and visualization images produced by VisIt_) automatically

  * With the following pieces....

    * VisIt_ python CLI
    * `pyscreenshot <http://pyscreenshot.readthedocs.io/en/latest/>`_ 
    * A minor adjustment to VisIt_ GUI to allow a python CLI instance
      which used ``OpenGUI(args...)`` to inform the GUI that widgets
      are to be raised/mapped on state changes.
  
  * We can include python code directly in these ``.rst`` documents
    (prefaced by ``.. only::`` directives to ensure the code does
    not actually appear in the generated manual) that does the work
    and just slurp this code out of these documents to actually run
    for automatic image generation.

    * Generate and save VisIt_ visualization images.
    * Use diffs on screen captured images to grab and even annotate images
      of GUI widgets.

.. code-block:: python

   import pyscreenshot
   import PIL

   # The arg (not yet implemented) sets flag in GUI to map windows
   # on state changes
   OpenGUI(MapWidgetsOnStateChanges=True)
   base_gui_image = pyscreenshot.grab()

   OpenDatabase('visit_data_path()/silo_hdf5_test_data/globe.silo') 
   AddPlot("Pseudocolor","dx")
   DrawPlots()

   # Save VisIt rendered image for manual
   SaveWindow('Plots/PlotTypes/Pseudocolor/images/figure15.png')
   ClearPlots()

   # Change something in PC atts to force it to map
   pcatts = PseudocolorAttributes()
   pcatts.colorTableName = 'Blue'
   SetPlotOptions(pcatts) # Causes widget to map due to state change
   pcatts.colorTableName = 'hot'
   SetPlotOptions(pcatts) # Causes widget to map due to state change
   gui_image = pyscreenshot.grab()

   # Save image of VisIt PC Attr window
   #   - computes diff between gui_image and base_gui_image, bounding box
   #   - around it and then saves that bounding box from gui_image
   diff_bbox = BBoxedDiffImage(gui_image, gui_image_base)
   SaveBBoxedImage(gui_image, diff_bbox, 'Plots/PlotTypes/Pseudocolor/images/pcatts_window.png')

   # Make a change to another PC att, capture and save it
   pcatts.limitsMode = pcatts.CurrentPlot
   SetPlotOptions(pcatts) # Causes widget to map due to state change
   gui_image = pyscreenshot.grab()
   SaveBBoxedImage(gui_image, diff_bbox, 'Plots/PlotTypes/Pseudocolor/images/pcatts_limit_mode_window.png')
