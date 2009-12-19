=========================
User-set environment variables which effect global things:
=========================

TEEM_ARCH: the architecture you're compiling with and for
  --> This is the only variable which MUST be set <--

TEEM_ROOT: the top-level Teem directory, under which are the
  directories where object, library, and include files will be
  installed.  If not set, the top-level directory is taken to be
  "../..", when inside the source directory for the individual
  libraries

TEEM_LINK_SHARED: if set to "true" then binaries, when linked, will be
  linked with shared libraries, and not static libraries. If not set,
  we link against static libraries, in order to produce
  stand-alone-ish binaries

=========================
The variables that can/must be set by the individual architecture
.mk files.  Those which must be set are marked by a (*):
=========================

CC, LD, AR, RM, INSTALL, CHMOD (*): programs used during make

SHEXT: the extension on the name of shared libraries (.so, .sl, .dll)

SHARED_CFLAG, STATIC_CFLAG (*): flags which are passed to $(CC) when
  used to create binaries, so as to control whether shared or static
  libraries are linked against.

BIN_CFLAGS: any flags to $(CC) which should be used for compiling 
  binaries (in addition to the SHARED_CFLAG, STATIC_CFLAG flags above)

OPT_CFLAG: how to control optimization

ARCH_CFLAG: any flags to $(CC) which are important for compiling
  particular to the target architecture

CFLAGS: any flags to $(CC) for both .o and binary compiliation, in
  addition to $(OPT_CFLAG) $(ARCH_CFLAG)

ARCH_LDFLAG: any architecture-specific flags to $(LD) which are
  important for making a shared library on the target architecture

SHARED_LDFLAG: the flag to $(LD) which causes a shared library
  generated to be produced, not a static one

LDFLAGS: any flags to $(LD) for making shared libraries, in addition
  to $(ARCH_LDFLAG) $(SHARED_LDFLAG)

NONPIC_CFLAG, PIC_CFLAG (*): flags to $(CC) to force generation of PIC 
  objects off and on

PIC_MATTERS: if set to "true", then a seperate set of .o files,
  compiled as position-independent code, are generated in order to
  make the shared library

=========================
The variables that can be set by the individual library .mk files.
=========================

LIB: the name of the library being compiled.  If this isn't set, the
  assumption is that there is no new library to compile, but simply
  a set of binaries which depend on other libraries

