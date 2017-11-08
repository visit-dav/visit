.. _Contributing:

Contributing
============

A short primer on the VisIt project's use of Sphinx for its documentation
including some notes on style, word usage and grammar.

Sphinx Usage
~~~~~~~~~~~~

Headings
""""""""

* Top level heading (e.g. 19), ``---``
* Second level heading (e.g. 19.1), ``===``
* Third level heading (e.g. 19.1.1), ``~~~``
* Fourth level heading (e.g. 19.1.1.1), ``"""``

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

Tables
""""""


Math
""""

We add the Sphinx built-in extension ``sphinx.ext.mathjax`` to the
``extensions`` variable in ``conf.py``. This allows Sphinx to use
`mathjax <https://www.mathjax.org>`_ to do Latex-like math equations in our
documentation. For example, this code

::

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

Should work to define only terms unique to VisIt

Style and Grammar
~~~~~~~~~~~~~~~~~
