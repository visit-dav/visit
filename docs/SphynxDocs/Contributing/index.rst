.. _Contributing:

Contributing
============

This is a short contributing guide on the VisIt_ project's use of Sphinx for its
documentation including some notes on style, word usage and grammar.

Quick Reference
~~~~~~~~~~~~~~~
Note that the original source of most of the content here is the OpenOffice
document produced with heroic effort by Brad Whitlock. A conversion tool was
used to move most of the content there to Sphinx. As such, most of the Sphinx
usage conventions adopted here were driven by whatever the conversion tool
produced.  There are numerous opportunities for adjusting this to make better
use of Sphinx as we move forward. These are discussed at the
:ref:`end <contributing_forward>` of this section.

* Two documents about Sphinx are useful:

  * `reStructuredText Markup Specification <http://docutils.sourceforge.net/docs/ref/rst/restructuredtext.html>`_
  * `Sphinx Documentation <http://www.sphinx-doc.org/en/stable/contents.html>`_

* Sphinx uses blank lines as a block separators and 2 or 4 spaces of
  indentation to guide parsing and interpretation of content. So, be sure
  to pay careful attention to blank lines and indentation. They are not
  there for style.  They **need** to be there for Sphinx to parse and
  intepret the content directly.
* Create headings by a sequence of *separator characters* immediately
  underneath and the same length as the heading text as in::

    Top level heading
    =================
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

    Second level heading
    ~~~~~~~~~~~~~~~~~~~~
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

    Third level heading
    """""""""""""""""""
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

    Fourth Level Heading
    --------------------
    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

yields these headings...

Top level heading
=================
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

Second level heading
~~~~~~~~~~~~~~~~~~~~
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

Third level heading
"""""""""""""""""""
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

Fourth Level Heading
--------------------
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...

* Wherever possible, keep lines in ``.rst`` files to 80 columns or less.
* Avoid hyphenation of words.
* Upper case for all letters in an acronyms (MPI, VTK)
* Use case conventions of product names (QuickTime, TotalView, Valgrind)
* Bracket word(s) with two stars (``**some words**``) for **bold**.
* Bracket word(s) with one star (``*word*``) for *italics*.
* Bracket word(s) with two backticks (``this is three words``) for literal::

    Bracket word(s) with two backticks (``this is three words``) for literal.

* Line breaks in bracketed word(s) cause problems.
* Subscripting, H\ :sub:`2`\ O, and superscripting, E = mc\ :sup:`2`, are supported::

    Subscripting, H\ :sub:`2`\ O, and superscripting, E = mc\ :sup:`2`, are supported

* Avoid contractions such as ``isn't``, ``couldn't`` and ``you've``.
* Use **Bold** to refer to widget names, operator or plot names, etc.
* Use literals for code, commands, arguments, file names, etc.
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

.. _contributing_images:

More on Images
""""""""""""""

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

  .. figure:: ../Quantitative/images/Array_compose_with_bins.png

.. figure:: ../Quantitative/images/Array_compose_with_bins.png

Same image with ``:scale: 50%`` option

.. code-block:: RST

  .. figure:: ../Quantitative/images/Array_compose_with_bins.png
     :scale: 50% 

.. figure:: ../Quantitative/images/Array_compose_with_bins.png
   :scale: 50% 

Same image with an anchor for cross referencing...

.. code-block:: RST

  .. _my_figure:

  .. figure:: ../Quantitative/images/Array_compose_with_bins.png
     :scale: 50% 

.. _my_figure:

.. figure:: ../Quantitative/images/Array_compose_with_bins.png
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

  .. figure:: ../Quantitative/images/Array_compose_with_bins.png
     :scale: 50% 

     Here is a caption for the figure.

.. _my_figure2:

.. figure:: ../Quantitative/images/Array_compose_with_bins.png
   :scale: 50% 

   Here is a caption for the figure.

Note that the figure label (e.g. Fig 20.2) will not appear if there is no
caption.

Tables
""""""
Sphinx supports a variety of mechanisms for defining tables. The conversion
tool used to convert this documentation from its original OpenOffice format
converted all tables to the *grid* style of table which is kinda sorta like
ascii art. Large tables can result in individual lines that span many widths of
the editor window. It is combersome to deal with but rich in capabilities.

.. _contributing_math:

Math
""""

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

Spell Checking
""""""""""""""

There is a shell script, ``check_spelling.sh`` to run a spell check. However,
this script will fail if you do not have the required Sphinx extension and
prerequisite library.  We use a third party extension (e.g. not a builtin)
to Sphinx for spell checking
`sphinx-contrib.spelling <http://sphinxcontrib-spelling.readthedocs.io/en/latest/index.html>`_
which requires `PyEnchant <https://pythonhosted.org/pyenchant/>`_

The script copies ``conf.py`` to a temporary directory and adds
``'sphinxcontrib.spelling'`` to the ``extensions`` variable before running 
Sphinx like so, ``sphinx-build -c <TMPDIR> -b spelling . _spelling``.
As it runs, if it encounters any spelling errors, it will emit them along
with the file name and line number at which they occur. It will also output
any spelling errors to a file, ``output.txt`` in the ``_spelling`` directory.

When Sphinx reports a spelling error, your options are either to correct or
otherwise adjust the spelling issue or add the word to the
``spelling_wordlist.txt`` file.

Much of the VisIt_ documentation includes the names of executable applications,
their arguments, GUI widgets, VisIt_ components and VisIt_ architectural details.
It is best to typeset such names *exactly* as a user might encounter them while
using VisIt_. But, this can lead to a lot of *special case words* that Sphinx
spelling does not recognize and which need to be added to
``spelling_wordlist.txt``. For example, ``fmt`` is a *word* in
``spelling_wordlist.txt``.

On the other hand, it is best to avoid adding a bunch of special case words to
``spelling_wordlist.txt`` if it can be avoided. And, it turns out there are a
few options we should be able to use. One is the Sphinx ``.. spelling::``
directive described
`here <http://sphinxcontrib-spelling.readthedocs.io/en/latest/customize.html#private-dictionaries>`_

However, adding ``.. spelling::`` directives to our ``.rst`` files causes errors
when doing a *normal (HTML)* Sphinx build if the ``'sphinxcontrib.spelling'`` is
is not added to ``conf.py`` and also available in the Sphinx enviornment where
it is being built. If have tried conditioning ``.. spelling::`` directive blocks
with ``.. only:: spelling`` and even ``.. ifconfig::``. Neither works as
desired.

For the time being, the only solution is to add the words to the global
``spelling_wordlist.txt`` file.

.. _contributing_forward:

Things To Consider Going Forward
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Decide what to do about compound words such as *timestep*, *time step* or
  *time-step*. There are many instances to consider such as *keyframe*,
  *checkbox*, *pulldown*, *submenu*, *sublauncher*, etc.
* Do we need a glossary of VisIt_ specific terms such as...

 * SIL
 * Mixed materials
 * Species
 * OnionPeel
 * Mesh
 * Viewer
 * cycle
 * timestep
 * SR
 * Client-server

* Additional features of Sphinx to consider adopting...

  * ``:term:`` role for glossary terms
  * ``:abbr:`` role for abbreviations
  * ``:guilable:`` role for referring to GUI widgets
  * ``:command:`` role for OS level cammands
  * ``:file:`` role for referring to file names
  * ``:menuselection:`` role for referring to widget paths in GUI menus
  * ``:kbd:`` role for specifying a sequence of key strokes
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
      frequently used text such as |viswin|::

    with the following substitutions defined::

      .. _VisIt: https://visit.llnl.gov
      .. _VTK: https://www.vtk.org
      .. |viswin| replace:: **Viewer Window**

.. _VisIt: https://visit.llnl.gov
.. _VTK: https://www.vtk.org
.. |viswin| replace:: **Viewer Window**

* Possible method for embedding python code to generate and capture images
  automatically

  * With the following pieces....

    * VisIt_ ptyhon CLI
    * `pyscreenshot <http://pyscreenshot.readthedocs.io/en/latest/>`_ 
    * A minor adjustment to VisIt_ GUI to allow a python CLI instance
      which used ``OpenGUI(args...)`` to inform the GUI that widgets
      are to be mapped on state changes.
  
  * We can include python code directly in these ``.rst`` documents
    (prefaced by ``.. only::`` directives to ensure the code does
    not actually appear in the generated manual) that does the work
    and just slurps this code out of these documents to actually run
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
   SetPlotOptions(pcatts) # PC Attrs widget maps due to state change
   pcatts.colorTableName = 'hot'
   SetPlotOptions(pcatts) # PC Attrs widget maps due to state change
   gui_image = pyscreenshot.grab()

   # Save image of VisIt PC Attr window
   #   - computes diff between gui_image and base_gui_image, bounding box
   #   - around it and then saves that bounding box from gui_image
   diff_bbox = BBoxedDiffImage(gui_image, gui_image_base)
   SaveBBoxedImage(gui_image, diff_bbox, 'Plots/PlotTypes/Pseudocolor/images/pcatts_window.png')

   # Make a change to another PC att, capture and save it
   pcatts.limitsMode = pcatts.CurrentPlot
   SetPlotOptions(pcatts) # PC Attrs widget maps due to state change
   gui_image = pyscreenshot.grab()
   SaveBBoxedImage(gui_image, diff_bbox, 'Plots/PlotTypes/Pseudocolor/images/pcatts_limit_mode_window.png')
