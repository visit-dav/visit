/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                               Resample.h                                  //
// ************************************************************************* //

#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <visitstream.h>
#include <silo.h>

#include <Cell.h>


// ****************************************************************************
//  Class: Resample
//
//  Purpose:
//      Acts as a filter.  It takes in a mesh (quadmesh, ucdmesh) and creates
//      a resampled rectilinear mesh at the specified resolution.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
//  Modifications:
//
//    Hank Childs, Wed Jun 21 13:18:10 PDT 2000
//    Changed underlying algorithm to determine which nodes in the output
//    mesh were in each cell rather than rasterizing each one.
//
//    Hank Childs, Wed Aug 30 14:18:58 PDT 2000
//    Handle bad input more gracefully.
//
// ****************************************************************************

class Resample
{
  public:
                 Resample();
                ~Resample();

    void         AddExtents(float *);

    void         AddMesh(DBucdmesh *);
    void         AddMesh(DBquadmesh *);
    void         AddVar(DBucdvar *, int);
    void         AddVar(DBquadvar *, int);

    void         ConsolidateExtents(void);

    void         SetResolution(int, char *);
    void         SetResolution(int [3], char *);

    bool         ValidObject(void)  { return !badInput; };

    void         WrapUp(void);
    void         Write(DBfile *);

  private:
    // The current input ucdmesh.
    DBucdmesh   *um;

    // The current input quadmesh.
    DBquadmesh  *qm;

    // The resolution in X, Y, and Z.
    int          resolution[3];

    // The location to dump the rectilinear mesh and its vars in the Silo file.
    char        *siloLocation;

    // The cycle, time, etc with the rectilinear mesh.
    DBoptlist   *optList;

    // The resampled rectilinear mesh.
    DBquadmesh  *outMesh;

    // The variables that are resampled.
    DBquadvar  **outVars;

    // A boolean indicating if any extents have been received.
    bool         gotExtents;

    // The minimum extents of the input.
    float        minExtents[3];

    // The maximum extents of the input.
    float        maxExtents[3];

    // A flag that keeps track of whether any of the inputs were bad.
    bool         badInput;

    void         BadInput(char *);
    void         ConstructCell(Cell &, int);
    void         ConstructCell(Cell &, int, int, int);
    void         ConstructQuadMesh(DBquadmesh *);
    void         ConstructQuadMesh(DBucdmesh *);
    void         ConstructQuadMesh(char *, int, int, float, char *[3], 
                                   char *[3], int, int);
    void         ConstructQuadVar(DBucdvar *, int);
    void         ConstructQuadVar(DBquadvar *, int);
    void         ConstructQuadVar(int, char *, int);

    void         DetermineMinMaxIndex(int [2], float, float, float, float,
                                      int);

    void         GetNode(float [3], int, int, int);

    void         InterpolateCell(Cell &, int);

    void         SetCellValue(Cell &, DBucdvar *, int);
    void         SetCellValue(Cell &, DBquadvar *, int, int, int);

    static int const    VARS_N;
};


#endif


