#!/usr/bin/sh
# teem: Gordon Kindlmann's research software
# Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# The sh path set above (#!/usr/bin/sh) is only for debugging this:
# this shell is not run by make.  Its contents are read by the 
# "." intrinsic command, from the shell that make spawns.

# Little shell script to determine if installed headers and libs
# should be named as prerequisites (thereby triggering their
# re-install).  Before this is run, two variables are set: 
#
#   $mes : filenames for "me", library L
#   $needs : filenames for every library which L depends on
#
# The re-install is needed if:
# (test 0) One of $mes doesn't exist.
# (test 1) A needed file wasn't there.   The fact that it will
# named elsewhere as a prerequisite, and that its prerequisites in turn 
# will also be named, means that the corresponding needed library will be
# built, which means that L will have to re-installed, or
# (test 2) One of the needed files is newer than one of the $mes
#
# If a re-install is needed, then $mes is returned, otherwise nothing

# initialized to empty string
older=

# loop over mes and needs
for me in $mes
do #  --test 0--
  if [ ! -f $me ]
    then older=yes
  fi
  for need in $needs
  do #    --test 1--      ----test 2---
    if [  ! -f $need  -o  $need -nt $me ]
      then older=yes
    fi
  done
done
if [ -n "$older" ]
  then echo $mes
fi
