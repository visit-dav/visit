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

// ************************************************************************* //
//                             avtScatterFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_FILTER_H
#define AVT_SCATTER_FILTER_H

#include <avtStreamer.h>
#include <ScatterAttributes.h>

// ****************************************************************************
//  Class: avtScatterFilter
//
//  Purpose:
//      A filter that combines multiple scalar fields into a point mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:31:23 PST 2004
//
//  Modifications:
//    Brad Whitlock, Mon Jul 18 11:53:27 PDT 2005
//    Added overrides of PreExecute, PerformRestriction, and added new 
//    method PopulateDataInputs. Also added extents members.
//
// ****************************************************************************

class avtScatterFilter : public avtStreamer
{
  public:
                               avtScatterFilter(const std::string &v,
                                                const ScatterAttributes &);
    virtual                   ~avtScatterFilter();

    virtual const char        *GetType(void)  { return "avtScatterFilter"; };
    virtual const char        *GetDescription(void)  
                                   { return "Creating point mesh"; };

protected:
    struct DataInput
    {
        vtkDataArray *data;
        bool          useMin;
        bool          useMax;
        float         min;
        float         max;
        int           scale;
        float         skew;
    };

    std::string                variableName;
    ScatterAttributes          atts;
    double                     xExtents[2];
    double                     yExtents[2];
    double                     zExtents[2];
    double                     colorExtents[2];
    bool                       needXExtents;
    bool                       needYExtents;
    bool                       needZExtents;
    bool                       needColorExtents;

    virtual void               PreExecute(void);
    virtual vtkDataSet        *ExecuteData(vtkDataSet *, int, std::string);
    virtual void               RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p     
                               PerformRestriction(avtPipelineSpecification_p spec);
    vtkDataArray              *GetDataArray(vtkDataSet *inDS,
                                            const std::string &name,
                                            avtCentering targetCentering,
                                            bool &deleteArray);
    vtkDataArray              *Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                                        avtCentering cent) const;
    vtkDataSet                *PointMeshFromVariables(DataInput *d1,
                                                      DataInput *d2,
                                                      DataInput *d3,
                                                      DataInput *d4, bool &);
    int                        CountSpatialDimensions() const;
    void                       PopulateDataInputs(DataInput *orderedArrays,
                                                  vtkDataArray **arr) const;
    void                       PopulateNames(const char **) const;
    bool                       NeedSpatialExtents() const;
};


#endif


