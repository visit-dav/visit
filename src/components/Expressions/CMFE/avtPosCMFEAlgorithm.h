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
//                            avtPosCMFEAlgorithm.h                          //
// ************************************************************************* //

#ifndef AVT_POS_CMFE_ALGORITHM_H
#define AVT_POS_CMFE_ALGORITHM_H

#include <expression_exports.h>

#include <avtDataTree.h>
#include <avtIntervalTree.h>

class     vtkCell;
class     vtkDataArray;
class     vtkDataSet;


// ****************************************************************************
//  Class: avtPosCMFEAlgorithm
//
//  Purpose:
//      Takes in two data sets and evaluates the field from one onto the mesh
//      from another.
//          
//  Programmer: Hank Childs
//  Creation:   January 21, 2006 (originally in avtPosCMFEExpression)
//
//  Modifications:
//
//    Hank Childs, Sat Mar 18 09:42:29 PST 2006
//    Add support for rectilinear desired points.  Also have
//    FastLookupGrouping cache lists of overlapping elements, so that
//    subsequent searches can use this as a guess rather than traversing
//    the interval tree.
//
// ****************************************************************************

class EXPRESSION_API avtPosCMFEAlgorithm
{
  public:
    static avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                         const std::string &,
                                         const std::string &,
                                         const std::string &);
    class SpatialPartition;

    class DesiredPoints
    {
      //
      // DesiredPoints is a tricky class because of two subtleties.
      // First: It has two ways to store data.  It stores rectilinear grids
      // one way and other grids another way.  Further, its interface tries
      // to unify them in places and not unify them in other places.  For
      // example, pt_list and pt_list_size correspond to non-rectilinear grids,
      // while rgrid_pts and rgrid_pts_size correspond to rectilinear grids.
      // But total_nvals corresponds to the total number of values across both.
      // The thinking is that the interface for the class should be
      // generalized, except where having knowledge of rectilinear layout will
      // impact performance, such as is the case for pivot finding.  Of course,
      // the bookkeeping under the covers is difficult.
      //
      // The other subtlety is the two forms this object will take.  Before
      // calling "RelocatePointsUsingSpatialPartition", this object contains
      // the desired points for this processor.  But, after the call, it
      // contains the desired points for this processor's spatial partition.
      // When "UnRelocatePoints" is called, it switches back to the desired
      // points for this processor.  Again, bookkeeping overhead causes the
      // coding of this class to be more complex.
      public:
                              DesiredPoints(bool, int);
        virtual              ~DesiredPoints();

        void                  AddDataset(vtkDataSet *);
        void                  Finalize();

        int                   GetNumberOfPoints() { return total_nvals; };
        int                   GetRGridStart()     { return rgrid_start; };
        int                   GetNumberOfRGrids() { return num_rgrids; };
        void                  GetPoint(int, float *) const;
        void                  GetRGrid(int, const float *&, const float *&,
                                       const float *&, int &, int &, int &);
        void                  SetValue(int, float *);

        const float          *GetValue(int, int) const;
    
        void                  RelocatePointsUsingPartition(SpatialPartition &);
        void                  UnRelocatePoints(SpatialPartition &);

      private:
        bool                  isNodal;
        int                   nComps;
        int                   total_nvals;
        int                   num_datasets;
        int                   num_rgrids;
        int                   rgrid_start;
        vector<float *>       pt_list;
        vector<int>           pt_list_size;
        vector<float *>       rgrid_pts;
        vector<int>           rgrid_pts_size;
        int                  *map_to_ds;
        int                  *ds_start;
        float                *vals;

        vector<float *>       orig_pt_list;
        vector<int>           orig_pt_list_size;
        vector<float *>       orig_rgrid_pts;
        vector<int>           orig_rgrid_pts_size;
        vector<int>           pt_list_came_from;
        vector<int>           rgrid_came_from;

        void                  GetProcessorsForGrid(int, std::vector<int> &,
                                                   std::vector<float> &,
                                                   SpatialPartition &);
        bool                  GetSubgridForBoundary(int, float *, int *);
    };

    class FastLookupGrouping
    {
      public:
                      FastLookupGrouping(std::string varname, bool);
        virtual      ~FastLookupGrouping();

        void          AddMesh(vtkDataSet *);
        vector<vtkDataSet *>   GetMeshes(void) { return meshes; };
        void          ClearAllInputMeshes(void);
        void          Finalize();
        void          RelocateDataUsingPartition(SpatialPartition &);

        bool          GetValue(const float *, float *);
        bool          GetValueUsingList(std::vector<int> &, const float *, 
                                        float *);

      protected:
        std::string            varname;
        bool                   isNodal;
        int                    nZones;
        vector<vtkDataSet *>   meshes;
        avtIntervalTree       *itree;
        int                   *map_to_ds;
        int                   *ds_start;
        std::vector<int>       list_from_last_successful_search;
    };

    class SpatialPartition
    {
      public:
                              SpatialPartition();
        virtual              ~SpatialPartition();
        void                  CreatePartition(DesiredPoints &, 
                                              FastLookupGrouping &,
                                              double *);

        int                   GetProcessor(float *);
        int                   GetProcessor(vtkCell *);
        void                  GetProcessorList(vtkCell *, std::vector<int> &);
        void                  GetProcessorBoundaries(float *,
                                    std::vector<int> &, std::vector<float> &);

      protected:
        avtIntervalTree      *itree;
    };
};


#endif


