These are the files required to incorporate Torsten Moeller's
kernels into nrrd.  The tmFilters_raw.c file is directly from
an email from Torsten.  The file ../tmfKernel.c is created
as follows:

  ./fix1.pl tmFilters_raw.c \
    | ./fix2.pl \
    | ./fix3.pl \
    | cat ../../preamble.c - \
    >! ../tmfKernel.c
