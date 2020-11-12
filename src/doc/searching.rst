Search Syntax
=============

Searching here uses simple query string syntax which supports
the following operators:

* ``+`` signifies AND operation
* ``|`` signifies OR operation
* ``-`` negates a single token
* ``"`` wraps a number of tokens to signify a phrase for searching
* ``*`` at the end of a term signifies a prefix query
* ``(`` and ``)`` signify precedence
* ``~N`` after a word signifies
  `edit distance <https://en.wikipedia.org/wiki/Levenshtein_distance>`_ (fuzziness)
* ``~N`` after a phrase signifies slop amount
* To use any of the above characters literally, escape it with a
  preceding backslash (``\``).
* A space between search terms implies the *default* operator of OR.
* When upper case is used, the search is case-sensitive. Otherwise it
  is case-insenstive.

Examples
--------

Searching : ``annot*``
    returns pages with ``Annotation``, ``Annotations``,
    ``annotate``, ``annotated``, etc.

Searching : ``Annot*``
    is case-sensitive and returns pages with ``Annotations``,
    ``Annotation`` but not ``annotate``.

Searching : ``annot* +object``
    returns pages with ``Annotation``, ``Annotations``,
    ``Annotated`` AND ``object``.

Searching : ``getannotationobject\(\)``
    returns pages with ``GetAnnotationObject()``

Searching : ``annot* | object``
    returns pages with ``Annotation``, ``Annotations``, ``Annotated``
    and also returns pages with ``object``.

Searching : ``load~4``
    returns pages including ``load``, ``lead``, ``head``, ``goal``

Searching : ``"load balance"``
    returns pages with the whole quoted phrase as opposed to
    pages that contain both ``load`` AND ``balance`` somewhere on
    the page.

Searching : ``load -balance``
    returns pages with contain ``load`` AND **not** do not also contain
    ``balance``.

Searching : ``foo bar -baz``
    returns pages containing ``foo`` or ``bar`` as well as any pages
    that do not contain ``baz`` (which probably *expands* the results 
    well beyond those containing just ``foo`` or ``bar``). This is probably
    not the intention. This is because the default operator (implied by
    spaces) is OR. To return documents that contain ``foo`` or ``bar`` but
    do not contain ``baz``, the search string would be ``foo bar +-baz``
    or ``(foo bar) +-baz``.
