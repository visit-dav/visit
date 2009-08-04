/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                avtCellList.h                              //
// ************************************************************************* //

#ifndef AVT_CELL_LIST_H
#define AVT_CELL_LIST_H

#include <pipeline_exports.h>

#include <avtCellTypes.h>

class     avtHexahedronExtractor;
class     avtImagePartition;
class     avtPointExtractor;
class     avtPyramidExtractor;
class     avtTetrahedronExtractor;
class     avtWedgeExtractor;
class     avtVolume;


typedef struct
{
    int   minx;
    int   maxx;
    int   miny;
    int   maxy;
    int   size;
    char *cell;
} avtSerializedCell;



// ****************************************************************************
//  Class: avtCellList
//
//  Purpose:
//      Stores cells so that sample points don't always have to be immediately
//      extracted from them.  This is really only used when operating in
//      parallel.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 15:14:50 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Tue Jan  1 13:15:09 PST 2002
//    Add estimates of number of samples per scanline to help the image
//    partition to create non-uniform partitions with approximately equal
//    sized partitions with respect to number of samples.
//
//    Hank Childs, Tue Jan 24 16:50:30 PST 2006
//    Add support for points.
//
//    Hank Childs, Fri Jan 27 14:50:38 PST 2006
//    Added method "Restrict".
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtCellList
{
  public:
                             avtCellList(int);
    virtual                 ~avtCellList();

    void                     Store(const avtHexahedron &, int, int, int, int);
    void                     Store(const avtPoint &, int, int, int, int);
    void                     Store(const avtPyramid &, int, int, int, int);
    void                     Store(const avtTetrahedron &, int, int, int, int);
    void                     Store(const avtWedge &, int, int, int, int);

    void                     ExtractCells(const char * const *, const int *,
                                          int, avtVolume *);
    char                    *ConstructMessages(avtImagePartition *, char **,
                                               int *);
    void                     EstimateNumberOfSamplesPerScanline(int *);
    int                      GetNumberOfCells(void) const { return celllistI; };

    void                     Restrict(int, int, int, int);

    void                     SetJittering(bool j) { jittering = j; };

  protected:
    avtSerializedCell      **celllist;
    int                      celllistI;
    int                      celllistN;
    int                      nVars;

    bool                     useRestriction;
    int                      minWidth, maxWidth;
    int                      minHeight, maxHeight;

    bool                     jittering;

    char                    *Serialize(const float (*)[3], 
                                       const float (*)[AVT_VARIABLE_LIMIT],int);
    char                    *SerializePoint(const float *, const float *);
    void                     Store(char *, int, int, int, int, int);
    void                     Unserialize(float (*)[3],
                                         float (*)[AVT_VARIABLE_LIMIT], int, 
                                         const char *&);
    void                     UnserializePoint(float *, float *, const char *&);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                             avtCellList(const avtCellList &) {;};
    avtCellList             &operator=(const avtCellList &) { return *this; };
};


#endif


