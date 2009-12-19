-----------
enhex/dehex
-----------

enhex and dehex are a stand-alone hex encoder/decoder pair.
They convert between data in raw form (paying no regard to the
endianness), and convert it to hexadecimal form.  

To compile:

  cc -o enhex enhex.c
  cc -o dehex dehex.c

I wrote these as freely distributable (non-copyleft) programs for
doing the hex encoding which is an optional encoding in the nrrd
library in Teem.  The (convoluted) reason is that the only encodings
that are *required* of non-Teem nrrd readers and writers are raw and
ascii.  All other encodings should be able to be handled by
stand-alone tools (such as gzip/gunzip for zlib compression, and
bzip/bunzip2 for bzip compress).  Yet, a google search didn't reveal
simple tools for raw--hex conversion, so I wrote them.  Nothing fancy.

Gordon Kindlmann
