// ************************************************************************* //
//                              avtClipFilter.h                              //
// ************************************************************************* //

#ifndef AVT_CLIP_FILTER_H
#define AVT_CLIP_FILTER_H


#include <avtPluginStructuredChunkStreamer.h>
#include <ClipAttributes.h>


class vtkDataSet;
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
//    Jeremy Meredith, Mon Aug 11 17:09:53 PDT 2003
//    Rewrote a huge chunk to make use of my new vtkVisItClipper3D.  The 
//    speedup from using this filter whenever possible is really, really big.
//
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Removed data members for filters that are now instantiated on the fly.
//
//    Hank Childs, Sun Mar 27 11:57:52 PST 2005
//    Add support for structured mesh chunking.
//
// ****************************************************************************

class avtClipFilter : public avtPluginStructuredChunkStreamer
{
  public:
                             avtClipFilter();
    virtual                 ~avtClipFilter();

    static avtFilter        *Create();

    virtual const char      *GetType(void)  { return "avtClipFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Clipping"; };

    virtual void             SetAtts(const AttributeGroup*);
    virtual bool             Equivalent(const AttributeGroup*);

  protected:
    ClipAttributes           atts;

    virtual vtkDataSet      *ProcessOneChunk(vtkDataSet *, int, 
                                             std::string, bool);
    virtual void             GetAssignments(vtkDataSet *, const int *,
                      std::vector<avtStructuredMeshChunker::ZoneDesignation>&);

    virtual void             RefashionDataObjectInfo(void);

  private:
    bool                     SetUpClipFunctions(vtkImplicitBoolean *, bool&);
};


#endif


