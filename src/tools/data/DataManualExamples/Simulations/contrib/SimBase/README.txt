This comprises two files:

 * simbase.py a general-purpose base class for creating sims
 * updateplots.py which follows loosely on the as-shipped
   version, demonstrating how it looks when built on top of
   simbase.py

simbase.py is intended for instrumenting GPU simulations, where
there is a single "node" being instrumented (i.e. multiple GPUs
in a single machine). If you want to see the documentation:

  pydoc ./simbase.py

(while sitting in the same directory containing simbase.py) will
display the inline documentation.

updateplots.py drops a few features, and adds a couple more (a
point mesh demo, with labels) compared to the as-shipped
standalone version.

----------------------------------

This example is provided by:
Fred Morris of Tacoma WA USA (just doing this in my spare time)
This document was last updated 06-Jun-2014
