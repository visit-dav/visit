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
  [edit distance](https://en.wikipedia.org/wiki/Levenshtein_distance) (fuzziness)
* ``~N`` after a phrase signifies slop amount
* To use any of the above characters literally, escape it with a
  preceding backslash (``\``).
* A space between search terms implies the *default* operator of OR.
* When upper case is used, the search is case-sensitive. Otherwise it
  is case-insenstive.

Examples
--------

Searching : ``annot*``
    returns results including ``Annotation``, ``Annotations``,
    ``annotate``, ``annotated``, etc.

Searching : ``Annot*``
    is case-sensitive and will return results include ``Annotations``,
    ``Annotation`` but not ``annotate``.

Searching : ``annot* object``
    returns results containing various instances of ``Annotation``,
    ``Annotations``, ``Annotated`` AND ``object``.

Searching : ``getannotationobject\(\)`` returns ``GetAnnotationObject()``

Searching : ``annot* | object``
    returns results containing various instances of ``Annotation``,
    ``Annotations``, ``Annotated`` OR ``object``.

Searching : ``load~4`` returns results including ``load``, ``lead``, ``head``, ``goal``

Searching : ``"load balance"`` returns results including the whole
    quoted phrase as opposed to results that simply contain both
    ``load`` AND ``balance``.

Searching : ``load -balance`` returns results containing ``load`` 
    AND **not** containing ``balance``.

Searching : ``foo bar -baz`` returns results containing ``foo`` or ``bar`` as well
    as any results that do not contain ``baz`` (which probably *expands* the results 
    well beyond those containing just ``foo`` or ``bar``). This is probably not the intention.
    This is because the default operator (implied by spaces) is OR. To return documents
    that contain ``foo`` or ``bar`` and do not contain ``baz``, the search string would
    be ``foo bar + -baz``.
