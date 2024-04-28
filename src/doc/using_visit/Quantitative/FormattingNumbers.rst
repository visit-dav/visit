.. _FormattingNumbers:

Formatting numbers
------------------
In various places in VisIt's interfaces (both the GUI and the CLI), there is often the need to control the formatting of numerical data to, for example, specify how many significant digits to or to use a fixed point format (e.g. ``123.456``) or an exponential format (e.g. ``1.23e+02``).

In many places in VisIt's interface, a `printf-style <https://cplusplus.com/reference/cstdio/printf>`__ formatting specification string is used to handle this.
This involves a percent sign (``%``), followed by several other *fields* of the general form ``%[flags][width][.precision][length]type`` where items in brackets are optional.

By default, VisIt uses ``%g`` for most numerical data which defaults to a precision of 6 and a width determined by the actual data according to the rules below.

For precision ``p >= 1``, this rounds the number to ``p`` significant digits and then formats the result in either fixed-point or scientific notation, depending on its magnitude.

Suppose that the result formatted with type specifier ``e`` and precision ``p-1`` would have exponent ``exp``.
Then if ``-4 <= exp < p``, the number is formatted with type specifier ``f``  and precision ``p-1-exp``.
Otherwise, the number is formatted with type specifier ``e`` and precision ``p-1``.
In both cases insignificant trailing zeros are removed from the significand, and the decimal point is also removed if there are no remaining digits following it.
Positive and negative infinity, positive and negative zero, and nans, are formatted as ``inf``, ``-inf``, ``0``, ``-0`` and ``nan`` respectively, regardless of the precision.
A precision of 0 is treated as equivalent to a precision of 1.
The default precision is 6.

Often, users are interested only in specifying ``width`` and ``precision`` as in, for example, ``%8.5g``.
In this example, the width of ``8``, is the *minimum* number of digits (or more generally characters because of the need for +/- sign, decimal point, exponentiation, etc.) to be printed.
If the value to be printed requires fewer than this number of characters, the result is padded with blank spaces.
The value is never truncated even if the result is larger.
The precision of ``5`` means to print 5 digits after the decimal point.
The type specification of ``g`` means the *general* type specification for floating point data.

The full `printf-style <https://cplusplus.com/reference/cstdio/printf>`__ formatting specification is quite flexible and general allowing a lot of control over printing of floating point, integer and even character data.
