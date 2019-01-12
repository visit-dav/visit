/*
 *  This file is part of Cognomen.
 *
 *  Cognomen is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Cognomen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Cognomen.  If not, see <http://www.gnu.org/licenses/>.
 *****
 * `bool' type which uses C99 if possible, else mimics C99.
 *****/
#ifndef COG_BOOL_H
#define COG_BOOL_H

#include <visit-config.h>

/* This implementation was recommended by the autoconf manual. */
#ifdef HAVE_STDBOOL_H
#   include <stdbool.h>
#else
#   ifndef HAVE__BOOL
#       ifdef __cplusplus
typedef bool _Bool;
#       else
#undef _Bool
typedef unsigned char _Bool;
#       endif
#   endif
#   define bool _Bool
#   define false 0
#   define true 1
#   define __bool_true_false_are_defined 1
#endif 

#endif /* COG_BOOL_H */
