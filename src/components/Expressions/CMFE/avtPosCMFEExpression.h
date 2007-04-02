// ************************************************************************* //
//                            avtPosCMFEExpression.h                         //
// ************************************************************************* //

#ifndef AVT_POS_CMFE_EXPRESSION_H
#define AVT_POS_CMFE_EXPRESSION_H

#include <avtCMFEExpression.h>

#include <avtIntervalTree.h>

class     vtkCell;
class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;


// ****************************************************************************
//  Class: avtPosCMFEExpression
//
//  Purpose:
//      Does a position based cross-mesh field evaluation.
//          
//  Programmer: Hank Childs
//  Creation:   October 10, 2005
//
// ****************************************************************************

class EXPRESSION_API avtPosCMFEExpression : public avtCMFEExpression
{
  public:
                              avtPosCMFEExpression();
    virtual                  ~avtPosCMFEExpression();

    virtual const char       *GetType(void){ return "avtPosCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
  protected:
    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    virtual bool              UseIdenticalSIL(void) { return false; };
    virtual bool              HasDefaultVariable(void) { return true; };

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
        void                  CreatePartition(
                                    avtPosCMFEExpression::DesiredPoints &, 
                                    avtPosCMFEExpression::FastLookupGrouping &,
                                    double *);

        int                   GetProcessor(float *);
        int                   GetProcessor(vtkCell *);
        void                  GetProcessorList(vtkCell *, std::vector<int> &);

      protected:
        avtIntervalTree      *itree;
    };
};


#endif


