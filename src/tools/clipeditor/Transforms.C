/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ----------------------------------------------------------------------------
// File:  Transforms.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Thu Sep 18 11:28:24 PDT 2003
//    Added quad and triangle transforms.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added voxel and pixel transform.
//
//    Jeremy Meredith, Thu Jun  7 13:21:32 EDT 2007
//    Added line transform.
//
// ----------------------------------------------------------------------------

#include "Transforms.h"

// Generated automatically by genhexcases.C
HexTransform hexTransforms[48] =
{
{{0,1,2,3,4,5,6,7},{'a','b','c','d','e','f','g','h','i','j','k','l'},false },
{{0,1,5,4,3,2,6,7},{'a','j','e','i','c','l','g','k','d','b','h','f'},true  },
{{0,4,7,3,1,5,6,2},{'i','h','k','d','j','f','l','b','a','e','c','g'},true  },
{{0,3,7,4,1,2,6,5},{'d','k','h','i','b','l','f','j','a','c','e','g'},false },
{{0,4,5,1,3,7,6,2},{'i','e','j','a','k','g','l','c','d','h','b','f'},false },
{{0,3,2,1,4,7,6,5},{'d','c','b','a','h','g','f','e','i','k','j','l'},true  },
{{3,2,1,0,7,6,5,4},{'c','b','a','d','g','f','e','h','k','l','i','j'},true  },
{{4,5,1,0,7,6,2,3},{'e','j','a','i','g','l','c','k','h','f','d','b'},false },
{{3,7,4,0,2,6,5,1},{'k','h','i','d','l','f','j','b','c','g','a','e'},false },
{{4,7,3,0,5,6,2,1},{'h','k','d','i','f','l','b','j','e','g','a','c'},true  },
{{1,5,4,0,2,6,7,3},{'j','e','i','a','l','g','k','c','b','f','d','h'},true  },
{{1,2,3,0,5,6,7,4},{'b','c','d','a','f','g','h','e','j','l','i','k'},false },
{{4,5,6,7,0,1,2,3},{'e','f','g','h','a','b','c','d','i','j','k','l'},true  },
{{3,2,6,7,0,1,5,4},{'c','l','g','k','a','j','e','i','d','b','h','f'},false },
{{1,5,6,2,0,4,7,3},{'j','f','l','b','i','h','k','d','a','e','c','g'},false },
{{1,2,6,5,0,3,7,4},{'b','l','f','j','d','k','h','i','a','c','e','g'},true  },
{{3,7,6,2,0,4,5,1},{'k','g','l','c','i','e','j','a','d','h','b','f'},true  },
{{4,7,6,5,0,3,2,1},{'h','g','f','e','d','c','b','a','i','k','j','l'},false },
{{7,6,5,4,3,2,1,0},{'g','f','e','h','c','b','a','d','k','l','i','j'},false },
{{7,6,2,3,4,5,1,0},{'g','l','c','k','e','j','a','i','h','f','d','b'},true  },
{{2,6,5,1,3,7,4,0},{'l','f','j','b','k','h','i','d','c','g','a','e'},true  },
{{5,6,2,1,4,7,3,0},{'f','l','b','j','h','k','d','i','e','g','a','c'},false },
{{2,6,7,3,1,5,4,0},{'l','g','k','c','j','e','i','a','b','f','d','h'},false },
{{5,6,7,4,1,2,3,0},{'f','g','h','e','b','c','d','a','j','l','i','k'},true  },
{{1,0,3,2,5,4,7,6},{'a','d','c','b','e','h','g','f','j','i','l','k'},true  },
{{1,0,4,5,2,3,7,6},{'a','i','e','j','c','k','g','l','b','d','f','h'},false },
{{4,0,3,7,5,1,2,6},{'i','d','k','h','j','b','l','f','e','a','g','c'},false },
{{3,0,4,7,2,1,5,6},{'d','i','h','k','b','j','f','l','c','a','g','e'},true  },
{{4,0,1,5,7,3,2,6},{'i','a','j','e','k','c','l','g','h','d','f','b'},true  },
{{3,0,1,2,7,4,5,6},{'d','a','b','c','h','e','f','g','k','i','l','j'},false },
{{2,3,0,1,6,7,4,5},{'c','d','a','b','g','h','e','f','l','k','j','i'},false },
{{5,4,0,1,6,7,3,2},{'e','i','a','j','g','k','c','l','f','h','b','d'},true  },
{{7,3,0,4,6,2,1,5},{'k','d','i','h','l','b','j','f','g','c','e','a'},true  },
{{7,4,0,3,6,5,1,2},{'h','i','d','k','f','j','b','l','g','e','c','a'},false },
{{5,1,0,4,6,2,3,7},{'j','a','i','e','l','c','k','g','f','b','h','d'},false },
{{2,1,0,3,6,5,4,7},{'b','a','d','c','f','e','h','g','l','j','k','i'},true  },
{{5,4,7,6,1,0,3,2},{'e','h','g','f','a','d','c','b','j','i','l','k'},false },
{{2,3,7,6,1,0,4,5},{'c','k','g','l','a','i','e','j','b','d','f','h'},true  },
{{5,1,2,6,4,0,3,7},{'j','b','l','f','i','d','k','h','e','a','g','c'},true  },
{{2,1,5,6,3,0,4,7},{'b','j','f','l','d','i','h','k','c','a','g','e'},false },
{{7,3,2,6,4,0,1,5},{'k','c','l','g','i','a','j','e','h','d','f','b'},false },
{{7,4,5,6,3,0,1,2},{'h','e','f','g','d','a','b','c','k','i','l','j'},true  },
{{6,7,4,5,2,3,0,1},{'g','h','e','f','c','d','a','b','l','k','j','i'},true  },
{{6,7,3,2,5,4,0,1},{'g','k','c','l','e','i','a','j','f','h','b','d'},false },
{{6,2,1,5,7,3,0,4},{'l','b','j','f','k','d','i','h','g','c','e','a'},false },
{{6,5,1,2,7,4,0,3},{'f','j','b','l','h','i','d','k','g','e','c','a'},true  },
{{6,2,3,7,5,1,0,4},{'l','c','k','g','j','a','i','e','f','b','h','d'},true  },
{{6,5,4,7,2,1,0,3},{'f','e','h','g','b','a','d','c','l','j','k','i'},false }
};

// Generated automatically by genvoxcases.C
VoxTransform voxTransforms[48] =
{
{{0,1,2,3,4,5,6,7},{'a','b','c','d','e','f','g','h','i','j','k','l'},false },
{{0,1,4,5,2,3,6,7},{'a','j','e','i','c','l','g','k','d','b','h','f'},true  },
{{0,4,2,6,1,5,3,7},{'i','h','k','d','j','f','l','b','a','e','c','g'},true  },
{{0,2,4,6,1,3,5,7},{'d','k','h','i','b','l','f','j','a','c','e','g'},false },
{{0,4,1,5,2,6,3,7},{'i','e','j','a','k','g','l','c','d','h','b','f'},false },
{{0,2,1,3,4,6,5,7},{'d','c','b','a','h','g','f','e','i','k','j','l'},true  },
{{2,3,0,1,6,7,4,5},{'c','b','a','d','g','f','e','h','k','l','i','j'},true  },
{{4,5,0,1,6,7,2,3},{'e','j','a','i','g','l','c','k','h','f','d','b'},false },
{{2,6,0,4,3,7,1,5},{'k','h','i','d','l','f','j','b','c','g','a','e'},false },
{{4,6,0,2,5,7,1,3},{'h','k','d','i','f','l','b','j','e','g','a','c'},true  },
{{1,5,0,4,3,7,2,6},{'j','e','i','a','l','g','k','c','b','f','d','h'},true  },
{{1,3,0,2,5,7,4,6},{'b','c','d','a','f','g','h','e','j','l','i','k'},false },
{{4,5,6,7,0,1,2,3},{'e','f','g','h','a','b','c','d','i','j','k','l'},true  },
{{2,3,6,7,0,1,4,5},{'c','l','g','k','a','j','e','i','d','b','h','f'},false },
{{1,5,3,7,0,4,2,6},{'j','f','l','b','i','h','k','d','a','e','c','g'},false },
{{1,3,5,7,0,2,4,6},{'b','l','f','j','d','k','h','i','a','c','e','g'},true  },
{{2,6,3,7,0,4,1,5},{'k','g','l','c','i','e','j','a','d','h','b','f'},true  },
{{4,6,5,7,0,2,1,3},{'h','g','f','e','d','c','b','a','i','k','j','l'},false },
{{6,7,4,5,2,3,0,1},{'g','f','e','h','c','b','a','d','k','l','i','j'},false },
{{6,7,2,3,4,5,0,1},{'g','l','c','k','e','j','a','i','h','f','d','b'},true  },
{{3,7,1,5,2,6,0,4},{'l','f','j','b','k','h','i','d','c','g','a','e'},true  },
{{5,7,1,3,4,6,0,2},{'f','l','b','j','h','k','d','i','e','g','a','c'},false },
{{3,7,2,6,1,5,0,4},{'l','g','k','c','j','e','i','a','b','f','d','h'},false },
{{5,7,4,6,1,3,0,2},{'f','g','h','e','b','c','d','a','j','l','i','k'},true  },
{{1,0,3,2,5,4,7,6},{'a','d','c','b','e','h','g','f','j','i','l','k'},true  },
{{1,0,5,4,3,2,7,6},{'a','i','e','j','c','k','g','l','b','d','f','h'},false },
{{4,0,6,2,5,1,7,3},{'i','d','k','h','j','b','l','f','e','a','g','c'},false },
{{2,0,6,4,3,1,7,5},{'d','i','h','k','b','j','f','l','c','a','g','e'},true  },
{{4,0,5,1,6,2,7,3},{'i','a','j','e','k','c','l','g','h','d','f','b'},true  },
{{2,0,3,1,6,4,7,5},{'d','a','b','c','h','e','f','g','k','i','l','j'},false },
{{3,2,1,0,7,6,5,4},{'c','d','a','b','g','h','e','f','l','k','j','i'},false },
{{5,4,1,0,7,6,3,2},{'e','i','a','j','g','k','c','l','f','h','b','d'},true  },
{{6,2,4,0,7,3,5,1},{'k','d','i','h','l','b','j','f','g','c','e','a'},true  },
{{6,4,2,0,7,5,3,1},{'h','i','d','k','f','j','b','l','g','e','c','a'},false },
{{5,1,4,0,7,3,6,2},{'j','a','i','e','l','c','k','g','f','b','h','d'},false },
{{3,1,2,0,7,5,6,4},{'b','a','d','c','f','e','h','g','l','j','k','i'},true  },
{{5,4,7,6,1,0,3,2},{'e','h','g','f','a','d','c','b','j','i','l','k'},false },
{{3,2,7,6,1,0,5,4},{'c','k','g','l','a','i','e','j','b','d','f','h'},true  },
{{5,1,7,3,4,0,6,2},{'j','b','l','f','i','d','k','h','e','a','g','c'},true  },
{{3,1,7,5,2,0,6,4},{'b','j','f','l','d','i','h','k','c','a','g','e'},false },
{{6,2,7,3,4,0,5,1},{'k','c','l','g','i','a','j','e','h','d','f','b'},false },
{{6,4,7,5,2,0,3,1},{'h','e','f','g','d','a','b','c','k','i','l','j'},true  },
{{7,6,5,4,3,2,1,0},{'g','h','e','f','c','d','a','b','l','k','j','i'},true  },
{{7,6,3,2,5,4,1,0},{'g','k','c','l','e','i','a','j','f','h','b','d'},false },
{{7,3,5,1,6,2,4,0},{'l','b','j','f','k','d','i','h','g','c','e','a'},false },
{{7,5,3,1,6,4,2,0},{'f','j','b','l','h','i','d','k','g','e','c','a'},true  },
{{7,3,6,2,5,1,4,0},{'l','c','k','g','j','a','i','e','f','b','h','d'},true  },
{{7,5,6,4,3,1,2,0},{'f','e','h','g','b','a','d','c','l','j','k','i'},false }
};

// Generated by hand - Jeremy Meredith, August 11, 2003
WedgeTransform wedgeTransforms[12] =
{
{{0,1,2,3,4,5},{'a','b','c','d','e','f','g','h','i'},false },
{{2,0,1,5,3,4},{'c','a','b','f','d','e','i','g','h'},false },
{{1,2,0,4,5,3},{'b','c','a','e','f','d','h','i','g'},false },
{{3,4,5,0,1,2},{'d','e','f','a','b','c','g','h','i'},true  },
{{5,3,4,2,0,1},{'f','d','e','c','a','b','i','g','h'},true  },
{{4,5,3,1,2,0},{'e','f','d','b','c','a','h','i','g'},true  },
{{0,2,1,3,5,4},{'c','b','a','f','e','d','g','i','h'},true  },
{{2,1,0,5,4,3},{'b','a','c','e','d','f','i','h','g'},true  },
{{1,0,2,4,3,5},{'a','c','b','d','f','e','h','g','i'},true  },
{{3,5,4,0,2,1},{'f','e','d','c','b','a','g','i','h'},false },
{{5,4,3,2,1,0},{'e','d','f','b','a','c','i','h','g'},false },
{{4,3,5,1,0,2},{'d','f','e','a','c','b','h','g','i'},false }
};

// Generated by hand - Jeremy Meredith, August 11, 2003
PyramidTransform pyramidTransforms[8] =
{
{{0,1,2,3,4},{'a','b','c','d','e','f','g','h'},false },
{{3,0,1,2,4},{'d','a','b','c','h','e','f','g'},false },
{{2,3,0,1,4},{'c','d','a','b','g','h','e','f'},false },
{{1,2,3,0,4},{'b','c','d','a','f','g','h','e'},false },
{{0,3,2,1,4},{'a','d','c','b','e','h','g','f'},true  },
{{3,2,1,0,4},{'d','c','b','a','h','g','f','e'},true  },
{{2,1,0,3,4},{'c','b','a','d','g','f','e','h'},true  },
{{1,0,3,2,4},{'b','a','d','c','f','e','h','g'},true  }
};

// Generated automatically by gentetcases.C
TetTransform tetTransforms[24] =
{
{{0,1,2,3},{'a','b','c','d','e','f'},false },
{{0,1,3,2},{'a','e','d','c','b','f'},true  },
{{0,2,1,3},{'c','b','a','d','f','e'},true  },
{{0,2,3,1},{'c','f','d','a','b','e'},false },
{{0,3,1,2},{'d','e','a','c','f','b'},false },
{{0,3,2,1},{'d','f','c','a','e','b'},true  },
{{1,0,2,3},{'a','c','b','e','d','f'},true  },
{{1,0,3,2},{'a','d','e','b','c','f'},false },
{{1,2,0,3},{'b','c','a','e','f','d'},false },
{{1,2,3,0},{'b','f','e','a','c','d'},true  },
{{1,3,0,2},{'e','d','a','b','f','c'},true  },
{{1,3,2,0},{'e','f','b','a','d','c'},false },
{{2,0,1,3},{'c','a','b','f','d','e'},false },
{{2,0,3,1},{'c','d','f','b','a','e'},true  },
{{2,1,0,3},{'b','a','c','f','e','d'},true  },
{{2,1,3,0},{'b','e','f','c','a','d'},false },
{{2,3,0,1},{'f','d','c','b','e','a'},false },
{{2,3,1,0},{'f','e','b','c','d','a'},true  },
{{3,0,1,2},{'d','a','e','f','c','b'},true  },
{{3,0,2,1},{'d','c','f','e','a','b'},false },
{{3,1,0,2},{'e','a','d','f','b','c'},false },
{{3,1,2,0},{'e','b','f','d','a','c'},true  },
{{3,2,0,1},{'f','c','d','e','b','a'},true  },
{{3,2,1,0},{'f','b','e','d','c','a'},false }
};

//  Generated by hand - Jeremy Meredith, September 17, 2003
QuadTransform quadTransforms[8] =
{
{{0,1,2,3},{'a','b','c','d'},false },
{{1,2,3,0},{'b','c','d','a'},false },
{{2,3,0,1},{'c','d','a','b'},false },
{{3,0,1,2},{'d','a','b','c'},false },
{{3,2,1,0},{'c','b','a','d'},true  },
{{0,3,2,1},{'d','c','b','a'},true  },
{{1,0,3,2},{'a','d','c','b'},true  },
{{2,1,0,3},{'b','a','d','c'},true  }
};

//  Generated by hand - Jeremy Meredith, June 23, 2004
PixelTransform pixelTransforms[8] =
{
{{0,1,2,3},{'a','b','c','d'},false },
{{1,3,0,2},{'b','c','d','a'},false },
{{3,2,1,0},{'c','d','a','b'},false },
{{2,0,3,1},{'d','a','b','c'},false },
{{1,0,3,2},{'a','d','c','b'},true  },
{{3,1,2,0},{'b','a','d','c'},true  },
{{2,3,0,1},{'c','b','a','d'},true  },
{{0,2,1,3},{'d','c','b','a'},true  }
};

//  Generated by hand - Jeremy Meredith, September 17, 2003
TriTransform triTransforms[6] =
{
{{0,1,2},{'a','b','c'},false },
{{1,2,0},{'b','c','a'},false },
{{2,0,1},{'c','a','b'},false },
{{2,1,0},{'b','a','c'},true  },
{{0,2,1},{'c','b','a'},true  },
{{1,0,2},{'a','c','b'},true  }
};

//  Generated by hand - Jeremy Meredith, June  7, 2007
LineTransform lineTransforms[2] =
{
{{0,1},{'a'},false },
{{1,0},{'a'},false }
};
