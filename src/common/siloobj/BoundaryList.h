/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                             BoundaryList.h                                //
// ************************************************************************* //

#ifndef BOUNDARY_LIST_H
#define BOUNDARY_LIST_H
#include <siloobj_exports.h>

#include <silo.h>

#include "OnionPeel.h"


// ****************************************************************************
//  Class: BoundaryList
//
//  Purpose:
//      Keeps information about boundaries between materials.
//
//  Data Members:
//      nMaterialNumbers        -  The number of materials.
//      materialNumbers         -  The number associated with each material.
//      nShapes                 -  An array of size nMaterialNumbers that
//                                 contains the number of shapes for each
//                                 material.
//      totalShapes             -  The total number of shapes across all 
//                                 materials.  This is the sum of the nShapes
//                                 array.
//      shapeCnt                -  An array of size totalShapes.  It contains
//                                 the number with that shape for each shape
//                                 and each material.
//      shapeSize               -  The size of each shape.  An array of size
//                                 totalShapes.
//      nOppositeMaterial       -  An array of size totalShapes.  It contains
//                                 the number of materials opposite each
//                                 shape for each material. 
//      totalOpposite           -  The total number of material opposite
//                                 each of the shapes for each of the 
//                                 materials.  This is the sum of the 
//                                 nOppositeMaterial array.
//      oppositeMaterialCount   -  The number of shapes for a given material
//                                 opposite this shape for this material.
//      oppositeMaterialNumber  -  The corresponding material number for 
//                                 the oppositeMaterialCount array.
//      nodeList                -  ??? NEEDS HELP ???
//      
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

class SILOOBJ_API BoundaryList
{
  public:
                 BoundaryList();
                ~BoundaryList();

    void         AddMaterial(DBmaterial *, OnionPeel *, DBzonelist *);
    void         SetName(char *);
    void         Write(DBfile *);

  private:
    int          nMaterialNumbers;
    int         *materialNumbers;
    int         *nShapes;
    int          totalShapes;
    int         *shapeCnt;
    int         *shapeSize;
    int         *nOppositeMaterial;
    int          totalOpposite;
    int         *oppositeMaterialCnt;
    int         *oppositeMaterialNumber;
    int         *nodeList;

    char        *name;

    // Constants
    static char * const   SILO_LOCATION;
    static int const      SILO_NUM_COMPONENTS;
    static char * const   SILO_TYPE;
    static char * const   SILO_N_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_SHAPES_NAME;
    static char * const   SILO_SHAPE_CNT_NAME;
    static char * const   SILO_SHAPE_SIZE_NAME;
    static char * const   SILO_N_OPPOSITE_MATERIAL_NAME;
    static char * const   SILO_OPPOSITE_MATERIAL_CNT_NAME;
    static char * const   SILO_OPPOSITE_MATERIAL_NUMBER_NAME;
    static char * const   SILO_NODE_LIST_NAME;
};


#endif


