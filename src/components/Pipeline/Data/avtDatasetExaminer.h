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
//                            avtDatasetExaminer.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_EXAMINER_H
#define AVT_DATASET_EXAMINER_H

#include <pipeline_exports.h>

#include <avtDataset.h>
#include <avtTypes.h>


class     vtkDataArray;


// ****************************************************************************
//  Class: avtDatasetExaminer
//
//  Purpose:
//      This is a collection of routines that walk through a dataset.  It is
//      separated out so that the avtDataset class does not get too messy.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 4 11:44:40 PDT 2002
//    I added GetDataMagnitudeExtents.
//
//    Kathleen Bonnell, Wed Feb 18 08:31:26 PST 2004
//    Added GetNumberOfNodes, another GetNumberOfZones.
//
//    Hank Childs, Tue Feb 24 17:33:45 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:14:20 PST 2004 
//    Removed GetDataMagnitudeExtents, now handled correctly by GetDataExtents. 
//
//    Hank Childs, Mon Jan  9 09:54:53 PST 2006
//    Added new form of GetSpatialExtents.
//
// ****************************************************************************

class PIPELINE_API avtDatasetExaminer
{
  public:
    static int                GetNumberOfNodes(avtDataset_p &);
    static int                GetNumberOfZones(avtDataset_p &);
    static void               GetNumberOfZones(avtDataset_p &, int &, int &);
    static void               GetVariableList(avtDataset_p &, VarList &);
    static bool               GetSpatialExtents(avtDataset_p &, double *);
    static bool               GetSpatialExtents(std::vector<avtDataTree_p> &, 
                                                double *);
    static bool               GetDataExtents(avtDataset_p &, double *,
                                             const char * = NULL);
    static void               FindMinimum(avtDataset_p &, double *, double &);
    static void               FindMaximum(avtDataset_p &, double *, double &);
    static bool               FindZone(avtDataset_p &, int, int, double *);
    static bool               FindNode(avtDataset_p &, int, int, double *);
    static vtkDataArray      *GetArray(avtDataset_p &, const char *, int,
                                       avtCentering &);
    static avtCentering       GetVariableCentering(avtDataset_p&, const char*);
};


#endif


