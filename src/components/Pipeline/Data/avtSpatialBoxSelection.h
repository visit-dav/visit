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
//                        avtSpatialBoxSelection.h                           //
// ************************************************************************* //

#ifndef AVT_SPATIAL_BOX_SELECTION_H
#define AVT_SPATIAL_BOX_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtSpatialBoxSelection
//
//  Purpose: Specify a data selection by an axis-aligned bounding box which
//  may be infinitely thin in one or more dimensions. The box is specified in
//  spatial, cartesian coordinates. Note that because it can be infinitely
//  thin in a dimension, we can also use this to specify orthogonal slices.
//
//  The default is a 3D box from FLT_MIN to FLT_MAX on each axis
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************

class PIPELINE_API avtSpatialBoxSelection : public avtDataSelection 
{
  public:

    enum InclusionMode
    {
        Whole,         // whole zone is included only if it is wholly in box 
        Partial,       // whole zone is included if some of it is in box 
        Clip           // part of zone in box after clipping is included 
    };
                            avtSpatialBoxSelection();
    virtual                ~avtSpatialBoxSelection() {} ;

    virtual const char *    GetType() const
                                { return "Spatial Box Data Selection"; }; 

    void                    SetInclusionMode(InclusionMode incMode)
                                { inclusionMode = incMode; };
    void                    SetMins(const double *_mins);
    void                    SetMaxs(const double *_maxs);

    void                    GetMins(double *_mins) const;
    void                    GetMaxs(double *_maxs) const;
    InclusionMode           GetInclusionMode() const { return inclusionMode; };

    bool                    operator==(const avtSpatialBoxSelection &) const;

    void                    GetLogicalBounds(const int nodeCounts[3],
                                             double *nodalCoords[3],
                                             int *firstZone, int *lastZone) const;

    void                    Compose(const avtSpatialBoxSelection& sel,
                                          avtSpatialBoxSelection& composeSel);

  private:

    InclusionMode inclusionMode;
    double mins[3];
    double maxs[3];

};

typedef ref_ptr<avtSpatialBoxSelection> avtSpatialBoxSelectiont_p;


#endif


