#
# The contents of this file are subject to the University of Utah Public
# License (the "License"); you may not use this file except in
# compliance with the License.
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License.
#
# The Original Source Code is "teem", released March 23, 2001.
#  
# The Original Source Code was developed by the University of Utah.
# Portions created by UNIVERSITY are Copyright (C) 2001, 1998 University
# of Utah. All Rights Reserved.
#
#
CC = /usr/bin/gcc
LD = /usr/bin/ld 
AR = /usr/bin/ar
RM = /bin/rm -f
INSTALL = /usr/bin/install

SHEXT = so
#PIC_MATTERS = true
#PIC_CFLAG = -fpic
#NONPIC_CFLAG = -fno-pic
PIC_MATTERS = false
PIC_CFLAG = 
NONPIC_CFLAG = 
OPT_CFLAG = -O2
STATIC_CFLAG = -Wl,-Bstatic
SHARED_CFLAG = -Wl,-Bdynamic
SHARED_LDFLAG = -shared
ARCH_CFLAG = 
