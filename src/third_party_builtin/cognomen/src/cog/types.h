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
 * Types utilized in Cognomen.
 *****/
#ifndef COG_TYPES_H
#define COG_TYPES_H

#define _SIZEOF_COG_ID 16
#define _SIZEOF_COG_SET 16

#include <stdlib.h>

typedef union {
    char _sz[_SIZEOF_COG_ID];
    int id;
} cog_id;

typedef union {
    char _sz[_SIZEOF_COG_SET];
    struct {
        cog_id *v;
        size_t size;
    } set;
} cog_set;

#endif /* COG_TYPES_H */
