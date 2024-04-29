.. _FormattingNumbers:

Formatting printed numbers
--------------------------
In various places in VisIt_'s interfaces (both the GUI and the CLI), there is often the need to control the formatting of printed numerical data to specify such things as the number of significant digits or whether to use a fixed point (e.g. ``123.456``) or an exponential format (e.g. ``1.23e+02``).

In many places in VisIt_'s interface, a `printf-style <https://cplusplus.com/reference/cstdio/printf>`__ formatting specification string is used to handle this.
This involves a percent sign (``%``), followed by several other *fields* of the general form :: 

    %[flags][width][.precision][length]type

where fields in square brackets are optional and within the context of VisIt_, ``[length]`` should almost always be ignored.
The table below provides a number of examples.

+-----------------+------------------------------+-----------------------------------+
| Format string   | Result                       | Remarks                           |
+=================+==============================+===================================+
|               value is ``"31415.92653589793"`` as ``double`` type                  |
+-----------------+------------------------------+-----------------------------------+
| ``%f``          | ``"31415.926536"``           | ``f`` is fixed point              |
+-----------------+------------------------------+-----------------------------------+
| ``%e``          | ``"3.141593e+04"``           | ``e`` is exponential              |
+-----------------+------------------------------+-----------------------------------+
| ``%g``          | ``"31415.9"``                | ``g`` yeilds ``e`` or ``f`` form  |
|                 |                              | depending on value (see below)    |
+-----------------+------------------------------+-----------------------------------+
| ``%a``          | ``"0x1.eadfb4c5d390bp+14"``  | ``%a`` shows actual memory        |            
+-----------------+------------------------------+-----------------------------------+
| ``%3.1g``       | ``"3e+04"``                  | width=3, .precision=1             |
+-----------------+------------------------------+-----------------------------------+
| ``%10.1f``      | ``"   31415.9"``             | width=10, .precision=1            |
+-----------------+------------------------------+-----------------------------------+
| ``%10.1g``      | ``"     3e+04"``             | here, ``g`` yields ``e`` form     |
+-----------------+------------------------------+-----------------------------------+
| ``%12.1g``      | ``"       3e+04"``           | default is right justification    |
+-----------------+------------------------------+-----------------------------------+
| ``%-12.4g``     | ``"3.142e+04   "``           | left justification                |
+-----------------+------------------------------+-----------------------------------+
| ``% 12.4g``     | ``"   3.142e+04"``           | note space after ``%``            |
+-----------------+------------------------------+-----------------------------------+
| ``%+12.4g``     | ``"  +3.142e+04"``           | always show a ``+`` or ``-`` sign |
+-----------------+------------------------------+-----------------------------------+
| ``%012.4G``     | ``"0003.142E+04"``           | leading 0's, cap ``E`` not ``e``  |
+-----------------+------------------------------+-----------------------------------+
|                     value is ``"1234"`` as ``int`` type                            |
+-----------------+------------------------------+-----------------------------------+
| ``%d``          | ``"1234"``                   |                                   |
+-----------------+------------------------------+-----------------------------------+
| ``%08d``        | ``"00001234"``               | width 8 with leading zeros        |
+-----------------+------------------------------+-----------------------------------+
| ``%o``          | ``"2322"``                   | print as octal                    |
+-----------------+------------------------------+-----------------------------------+
| ``08X``         | ``"000004D2"``               | print as hexidecimal (caps)       |
+-----------------+------------------------------+-----------------------------------+
|                value is ``"Einstein"`` as ``char*`` type                           |
+-----------------+------------------------------+-----------------------------------+
| ``%s``          | ``"Einstein"``               |                                   |
+-----------------+------------------------------+-----------------------------------+
| ``%16.5s``      | ``"           Einst"``       | 5 chars, right justified          |
+-----------------+------------------------------+-----------------------------------+
| ``%-16.5s``     | ``"Einst           "``       | 5 chars, left justified           |
+-----------------+------------------------------+-----------------------------------+

By default, VisIt_ uses ``%g`` for most numerical data which uses a default ``[.precision]`` of 6 and a default ``[width]`` which is *automatically* determined based on the actual data value to be printed according to the rules below.

  For precision ``p >= 1``, this rounds the number to ``p`` significant digits and then formats the result in either fixed-point or scientific notation, depending on its magnitude.

  Suppose that the result formatted with type specifier ``e`` and precision ``p-1`` would have exponent ``exp``.
  Then if ``-4 <= exp < p``, the number is formatted with type specifier ``f`` (e.g. fixed point format)  and precision ``p-1-exp``.
  Otherwise, the number is formatted with type specifier ``e`` (e.g. exponential format) and precision ``p-1``.
  In both cases insignificant trailing zeros are removed from the significand, and the decimal point is also removed if there are no remaining digits following it.
  Positive and negative infinity, positive and negative zero, and nans, are formatted as ``inf``, ``-inf``, ``0``, ``-0`` and ``nan`` respectively, regardless of the precision.
  A precision of 0 is treated as equivalent to a precision of 1.
  The default precision is 6.

Often, users are interested in controlling only ``[width]`` and ``[.precision]`` as in, for example, ``%8.5g``.
In this example, the width of ``8``, is the *minimum* number of characters.
If the value to be printed requires fewer than this number of characters, the result is padded with blank spaces.
The value is never truncated even if the result will use more than the minimum number of characters.
The precision of ``.5`` means to print 5 digits after the decimal point.
The type specification of ``g`` is the *general* type specification for *floating point* type data as described above.

The full `printf-style <https://cplusplus.com/reference/cstdio/printf>`__ formatting specification is quite flexible and general allowing a lot of control over how numerical data is printed.
