// ************************************************************************* //
//                             avtClipFilter.h                              //
// ************************************************************************* //

#ifndef AVT_CLIP_FILTER_H
#define AVT_CLIP_FILTER_H


#include <avtPluginStreamer.h>
#include <ClipAttributes.h>


class vtkDataSet;
class vtkClipDataSet;
class vtkClipVolume;
class vtkClipPolyData;
class vtkImplicitBoolean;
class vtkImplicitFunction;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtClipFilter
//
//  Purpose:
//      Performs the duty of the Clip operator.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed vtkGeometryFilter.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument to Execute method, in order to match new interface. 
//
//    Hank Childs, Thu Aug 15 21:23:13 PDT 2002
//    Inherited from plugin streamer rather than plugin data tree streamer.
//    Also added some performance gains.
//
// ****************************************************************************

class avtClipFilter : public avtPluginStreamer
{
  public:
                             avtClipFilter();
    virtual                 ~avtClipFilter();

    static avtFilter        *Create();

    virtual const char      *GetType(void)  { return "avtClipFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Clipping"; };
    virtual void             ReleaseData(void);

    virtual void             SetAtts(const AttributeGroup*);
    virtual bool             Equivalent(const AttributeGroup*);

  protected:
    ClipAttributes           atts;
    bool                     needsValidFaceConnectivity;
    bool                     subdivisionOccurred;
    vtkClipDataSet          *clipData;
    vtkClipPolyData         *clipPoly;
    vtkClipVolume           *clipVolume;

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual void             RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);

  private:
    bool                     SetUpClipFunctions(vtkImplicitBoolean *, bool&);
    vtkDataSet              *ClipData(vtkDataSet *, vtkImplicitFunction *,
                                      bool);
    vtkDataSet              *FastClipData(vtkDataSet *, vtkImplicitFunction *,
                                          bool);
};


#endif


