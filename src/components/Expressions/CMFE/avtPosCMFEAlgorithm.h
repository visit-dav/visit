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
      public:
                              DesiredPoints(bool, int);
        virtual              ~DesiredPoints();

        void                  AddDataset(vtkDataSet *);
        void                  Finalize();

        int                   GetNumberOfPoints() {return total_nvals;};
        void                  GetPoint(int, float *) const;
        void                  SetValue(int, float *);

        const float          *GetValue(int, int) const;
    
        void                  RelocatePointsUsingPartition(SpatialPartition &);
        void                  UnRelocatePoints(SpatialPartition &);

      private:
        bool                  isNodal;
        int                   nComps;
        int                   total_nvals;
        int                   num_datasets;
        vector<float *>       pt_list;
        vector<int>           pt_list_size;
        int                  *map_to_ds;
        int                  *ds_start;
        float                *vals;

        vector<float *>       orig_pt_list;
        vector<int>           orig_pt_list_size;
        vector<int>           num_return_to_proc;
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

      protected:
        std::string            varname;
        bool                   isNodal;
        int                    nZones;
        vector<vtkDataSet *>   meshes;
        avtIntervalTree       *itree;
        int                   *map_to_ds;
        int                   *ds_start;
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

      protected:
        avtIntervalTree      *itree;
    };
};


#endif


