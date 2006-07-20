/*    
 * Vis5D system for visualizing five dimensional gridded data sets.
 * Copyright (C) 1990 - 2000 Bill Hibbard, Johan Kellum, Brian Paul,
 * Dave Santek, and Andre Battaiola.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * As a special exception to the terms of the GNU General Public
 * License, you are permitted to link Vis5D with (and distribute the
 * resulting source and executables) the LUI library (copyright by
 * Stellar Computer Inc. and licensed for distribution with Vis5D),
 * the McIDAS library, and/or the NetCDF library, where those
 * libraries are governed by the terms of their own licenses.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/*
 * This configuration file contains options which can be safely
 * changed by the user.
 */



#ifndef VIS5D_H
#define VIS5D_H


/*
 * Amount of physical RAM in megabytes:
 * vis5d normally uses a bounded amount of memory to avoid swapping.
 * When the limit is reached, the least-recently-viewed graphics will
 * be deallocated.  If MBS is set to 0, however, vis5d will use ordinary
 * malloc/free and not deallocate graphics (ok for systems with a lot
 * of memory (>=128MB)).
 */
#ifdef VIS5D_MAX_MEM /* from config.h, via configure --with-memory=MB */
#  define MBS VIS5D_MAX_MEM
#else
#  define MBS 0
#endif



/* Default topography file: */
#define TOPOFILE "EARTH.TOPO"


/* Default map lines files: */
#define WORLDFILE "OUTLSUPW"
#define USAFILE "OUTLUSAM"


/* Default filename of Tcl startup commands: */
#define TCL_STARTUP_FILE "vis5d.tcl"


/* Default directory to search for user functions: */
#define FUNCTION_PATH "userfuncs"


/* Default animation rate in milliseconds: */
#define ANIMRATE 100

#define    DEFAULT_MAXTMESH    -1    /* no max    */
#define    DEFAULT_VSTRIDE        4    


/* Default scale and exponent values for logrithmic vertical coordinate system: */
#define DEFAULT_LOG_SCALE  1012.5
#define DEFAULT_LOG_EXP  -7.2

#define DEFAULT_SOUNDFONTNAME "6x12"

/**********************************************************************/
/**********************************************************************/
/***          USERS:  DON'T CHANGE ANYTHING BEYOND THIS POINT       ***/
/**********************************************************************/
/**********************************************************************/

/*
 * Define BIG_GFX to allow larger isosurfaces, contour slices, etc. if
 * there's enough memory.
#if MBS==0 || MBS>=128
#  define BIG_GFX
#endif
 */

#define BIG_GFX


/*
 * Shared by code above and below API:
 */
#define MAX_LABEL   1000
#define MAX_FUNCS   100



#endif
