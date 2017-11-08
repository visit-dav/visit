.. _Contributing:

Contributing
============

This is a short primer on the VisIt project's use of Sphinx for its
documentation including some notes on style, word usage and grammar.

Sphinx Usage
~~~~~~~~~~~~

Note that Sphinx uses blank lines as a block separators and indentation
for interpretation. So, be sure to pay careful attention to blank lines
and indentation in these examples. They are not there for style.
They **need** to be there for Sphinx to intepret the content directly.

Headings
""""""""

* Top level heading (e.g. 19), ``---``
* Second level heading (e.g. 19.1), ``===``
* Third level heading (e.g. 19.1.1), ``~~~``
* Fourth level heading (e.g. 19.1.1.1), ``"""``

In addition, keep individual lines to 80 columns or less.

References
""""""""""

Images
""""""

Try to use PNG formatted images. We plan to use the Sphinx generated
documentation both for online HTML and for printed PDF. So, images sizes
cannot be too big or they will slow HTML loads but not so small they are
unusable in PDF.

Some image formats wind up enforcing **physical** dimensions instead of
just pixel dimensions. This can have the effect of causing a nicely sized
image (from pixel dimensions perspective anyways), to either be unusually
large or unusually small in HTML or PDF output. In these cases, you can
use the Sphinx ``:scale:`` and ``:width:`` or ``:height:`` options for
a ``.. figure::``. Also, be sure to use a ``.. figure::`` directive instead
of an ``.. image::`` directive for embedding images. This is because the
``.. figure::`` directive also supports cross-referencing.

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

Same image with an anchor for cross-referencing...

.. code-block:: RST

  .. _my_figure:

  .. figure:: ../Quantitative/images/Array_compose_with_bins.png
     :scale: 50% 

.. _my_figure:

.. figure:: ../Quantitative/images/Array_compose_with_bins.png
   :scale: 50% 

which can now be cross-referenced using an inline :numref:`reference %s <my_figure>` 
like so...

.. code-block:: RST

  Which can now be cross-referenced using an inline :numref:`reference %s <my_figure>` 
  like so...

Note the anchor has a leading underscore which the reference does not include.

Tables
""""""


Math
""""

We add the Sphinx built-in extension ``sphinx.ext.mathjax`` to the
``extensions`` variable in ``conf.py``. This allows Sphinx to use
`mathjax <https://www.mathjax.org>`_ to do Latex-like math equations in our
documentation. For example, this code

.. code-block:: RST

  :math:`x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}`

produces...

:math:`x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}`

You can find more examples in :ref:`Expressions <Expressions>`

Spell Checking
""""""""""""""

There is a shell script, ``check_spelling.sh`` to run a spell check. However,
this script will fail if you do not have the required Sphinx extension and
prerequisite library.  We use a non-builtin extension to Sphinx for spell
checking,
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

Much of the VisIt documentation includes the names of executable applications,
their arguments, GUI widgets, VisIt components and VisIt architectural details.
It is best to typeset such names *exactly* as a user might encounter them while
using VisIt. But, this can lead to a lot of *special case words* that Sphinx
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

Glossary
~~~~~~~~

Should work to define only terms unique to VisIt. Examples of good candidates
are...

* SIL
* Species
* OnionPeel
* Mesh
* Viewer
* cycle
* timestep

Style and Grammar
~~~~~~~~~~~~~~~~~

Its difficult to remember but a key purpose of this content is to educate
*users* of VisIt and not *developers*. As developers, its far too easy to
fall into developer-centric terminology and jargon and get too detailed in
descriptions of things.

This is not to say we could not eventually add developer-specific content here
and use Sphinx *tagging* to filter it out when we produce the HTML for the
*user* manual. But, if/when we do that, we'll still need to maintain a clear
distinction between content for *users* and content for *developers*.

The point of mentioning this is to remind authors to focus on describing things
from the *user's* perspective. For example, does a user need to know about
a **Subset Inclusion Lattice (SIL)** in order to understand how to use the 
**Subset Window**? Probably not.

Should we use *timestep*, *time-step* or *time step*? Likewise for many other
terms such as *dropdown*, *submenu*, *keyframe*, *checkbox*, etc.

From the point of view of quality spell checking and minimizing the need for
special case words in the global wordlist file, it is best to minimize the
use of hyphenated words. Hyphens should be used *only* when absolutely 
necessary to ensure proper meaning.

* Upper case for all letters in an acronyms (MPI, VTK)
* Use case-conventions of product names (QuickTime, TotalView, Valgrind)
* Do we need 

First, Second or Third Person
"""""""""""""""""""""""""""""

Third person reads the most formal and professional. However, it does not
always result in the greatest concision, especially when describing a long
list of steps for the user to follow.


