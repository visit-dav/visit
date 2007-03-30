// ************************************************************************* //
//                          avtDataTreeStreamer.h                            //
// ************************************************************************* //

#ifndef AVT_DOMAINTREESTREAMER_H
#define AVT_DOMAINTREESTREAMER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>


class  avtExtents;


// ****************************************************************************
//  Class: avtDataTreeStreamer
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will "stream" 
//      datasets through a single filter one at a time during execution.  It
//      could be part of the actual avtDatasetToDatasetFilter class, since
//      almost every derived type of avtDatasetToDatasetFilter will be 
//      interested in streaming their domains through, but it was made a
//      separate class to separate what functionality was for a filter (or
//      process object) in the pipeline and what sent domains through vtk
//      filters one at a time.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Nade ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Fri Feb  9 14:47:10 PST 2001 
//    Renamed this class from avtStreamer.  Made ExecuteDomain return
//    an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr 12 10:25:04 PDT 2001 
//    Renamed this class as avtDataTreeStreamer. Added recursive
//    Execute method to walk down input tree.
//
//    Hank Childs, Wed Jun 20 09:39:01 PDT 2001
//    Added support progress callback.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001
//    Added string argument to ExecuteDataTree method. 
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Moved PreExecute and PostExecute to avtFilter.
//
//    Hank Childs, Tue Nov  6 11:46:10 PST 2001
//    Add support for overriding extents.
//
// **************************************************************************** 

class PIPELINE_API avtDataTreeStreamer : public avtDatasetToDatasetFilter
{
   public:
                             avtDataTreeStreamer();
     virtual                ~avtDataTreeStreamer();

  protected:
    int                      currentNode;
    int                      totalNodes;

    virtual void             Execute(void);
    virtual avtDataTree_p    Execute(avtDataTree_p);
    virtual avtDataTree_p    ExecuteDataTree(vtkDataSet *,int,std::string) = 0;

    void                     OverrideTrueSpatialExtents(void)
                                 { overrideTrueSpatialExtents = true; };
    void                     OverrideTrueDataExtents(void)
                                 { overrideTrueDataExtents = true; };

  private:
    bool                     overrideTrueSpatialExtents;
    bool                     overrideTrueDataExtents;
    avtExtents              *trueSpatialExtents;
    avtExtents              *trueDataExtents;

    void                     UpdateExtents(avtDataTree_p);
};


#endif


