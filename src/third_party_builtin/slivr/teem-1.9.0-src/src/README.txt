  Teem: Gordon Kindlmann's research software
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



Teem is a collection of libraries and programs that I use for my
research.  All of the things below are names of libraries in teem,
many libraries have some associated command-line programs useful for
debugging or accessing the functionality in the library.  Teem
libraries depend on each other in a non-circularly dependent way.
Those things listed *later* depend on a subset of the libraries listed
*earlier*: this is the *reverse* of how they would be listed on the
link ("ld") line.

air:    general utilities which everyone needs
hest:   command-line parsing
biff:   general error reporting utility
nrrd:   nearly raw raster data
ell:    assorted linear algebra stuff
unrrdu: "unu": powerful command-line interface to nrrd
dye:    color conversions (and eventually color maps)
gage:   for measuring things in volumes (currently only scalar volumes)
        (includes "qbert" for generating VGH volumes for simian)
bane:   GK's Master's thesis re-reimplementation
limn:   graphics related stuff and/or postscript drawing
hoover: multi-threaded volume rendering framework
mite:   "miter": simple volume renderer of scalar fields
echo:   dumb ray-tracer, pay no attention
ten:    diffusion tensor related functionality
