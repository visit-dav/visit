/*****************************************************************************

----COPYRIGHT NOTICE for freely-distributable software by Mark Duchaineau----

    COPYRIGHT (c) 1991-2002 Mark A. Duchaineau
    ALL RIGHTS RESERVED

    License version LibGen20020227

REDISTRIBUTION AND MARKING
--------------------------
This notice covers software and documentation written
by Mark A. Duchaineau (hereafter ``the author'') which is marked:

    by Mark Duchaineau (free but copyrighted, see LibGen/COPYING)

and any files not so marked that are included in LibGen
distributions or on the cognigraph.com website, and are neither
listed in LibGen/README as exceptions nor are marked as being authored
by others.  Any redistribution of covered files of portions thereof
must either (a) retain any markings and the COPYING file, or
(b) be clearly marked as authored by Mark A. Duchaineau and include
the following notice either in printed or digital form:

    Unpaid contributions to this distribution were made by
    Mark A. Duchaineau.  The original version of these
    contributions is available for free at
        http://www.cognigraph.com/
    under COPYRIGHT (c) 1991-2002 Mark A. Duchaineau.

The author's name and the names of any organizations the author is
or has been affiliated with must be kept out of any advertisements
and promotional materials unless specific written permission is given.
Other than these restrictions, software and documentation covered by this
notice may be distributed freely or for profit, or used as you see fit
without royalties to the author.

NO WARRANTY
-----------
THIS SOFTWARE IS EXPERIMENTAL IN NATURE.  Please try your hand at
improving it!  Be aware that bugs exist and that caution should be
exercised in using this software.  The author is solely responsible
for the content, and makes no warranty about its fitness for any use.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHOR OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

CONTACT INFORMATION
-------------------
Please keep in touch with me in person or by email.  I'd de delighted
to hear about how you are using my software or how I can make it better.


--duchaine@llnl.gov           http://graphics.cs.ucdavis.edu/~duchaine
                              http://www.llnl.gov/CASC/people/duchaineau/

-----------------------------------------------------------------------------

*****************************************************************************/


/********** fwav.h created by lib **********/


#ifndef LIB_fwav__TYPE

#define LIB_fwav__TYPE 1

#ifndef LIB_gzlib__TYPEONLY

#define LIB_gzlib__TYPEONLY 1

#include "gzlib.h"

#undef LIB_gzlib__TYPEONLY

#else

#include "gzlib.h"

#endif

typedef struct fwav_structdef fwav_struct,*fwav;

#endif


#ifndef LIB_fwav__TYPEONLY

#ifndef LIB_fwav__HERE

#define LIB_fwav__HERE 1

void lib_init_fwav();

void lib_quit_fwav();


    /* fwav.l         linear wavelet transform library for bricks of floats */
    /* by Mark Duchaineau (free but copyrighted, see LibGen/COPYING) */
    /* */
    /* 09-21-00: based on 12-11-99 Wav/wav.l library */
    /* 09-24-00: allow non power-of-two bricks; added fwavtest.l application */
    /* 09-25-00: fixed stran() failure to init _zero */
    /* 12-08-02: new API to be thread safe, use app-supplied mem allocation */
    /* 12-13-02: API change adds opaque arg to fwav_create() */


    /* lib_include "stdlib.h" */
#include "stdlib.h"

    /* lib_include "Gzlib/gzlib.l" */
#include "gzlib.h"



    fwav fwav_create(int xs,int ys,int zs,float *thresh,
        gzlib_alloctype my_alloc,gzlib_freetype my_free,void *opaque);
    void fwav_destroy(fwav fw);
    void fwav_atran(fwav fw,float *v);
    void fwav_stran(fwav fw,float *v);


#endif


#endif

