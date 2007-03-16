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
//                       avtConnComponentsSummaryQuery.h                     //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_SUMMARY_QUERY_H
#define AVT_CONN_COMPONENTS_SUMMARY_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <string>


class avtRevolvedVolume;
class avtVMetricArea;
class avtVMetricVolume;

class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsSummaryQuery
//
//  Purpose:
//      Finds the connected components of a mesh and creates a point dataset 
//      with per component quantities. This dataset is saved in as an Xmdv
//      file.  
// 
//      Component Fields:
//       x,y,z              Component centroid location
//       comp_area          Component area (if 2D data)
//       comp_volume        Component volume (if 3D or revolved volume data)
//       comp_sum           Component variable sum 
//       comp_weighted_sum  Component weighted variable sum
//
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsSummaryQuery 
: public avtConnComponentsQuery
{
  public:
                               avtConnComponentsSummaryQuery();
    virtual                   ~avtConnComponentsSummaryQuery();

    virtual const char        *GetType(void)
                         { return "avtConnComponentsSummaryQuery"; };
    virtual const char        *GetDescription(void)
                         { return "Generating connected components summary.";};

    void                       SetOutputFileName(const std::string &fname)
                                 {outputFileName = fname; }
  protected:

    std::string                outputFileName;

    avtRevolvedVolume         *revolvedVolumeFilter;
    avtVMetricArea            *areaFilter;
    avtVMetricVolume          *volumeFilter;

    std::string                variableName;

    vector<int>                nCellsPerComp;

    vector<double>             xCentroidPerComp;
    vector<double>             yCentroidPerComp;
    vector<double>             zCentroidPerComp;

    vector<double>             areaPerComp;
    vector<double>             volPerComp;

    vector<double>             sumPerComp;
    vector<double>             wsumPerComp;

    bool                       findArea;
    bool                       findVolume;

    virtual void               Execute(vtkDataSet *, const int);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    virtual void               VerifyInput(void);

    virtual void               SaveComponentResults(string fname);
    virtual void               PrepareComponentResults(vector<double> &);
};


#endif



