// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVectorFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VECTOR_FILTER_H
#define AVT_VECTOR_FILTER_H

#include <avtDataTreeIterator.h>

class vtkVectorReduceFilter;
class vtkVertexFilter;


// ****************************************************************************
//  Class: avtVectorFilter
//
//  Purpose:
//      A filter that takes in vector data and creates vector glyphs as poly
//      data.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 11:51:18 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added the vertex filter.
//
//    Kathleen Bonnell, Mon Aug  9 14:27:08 PDT 2004 
//    Added magVarName, SetMagVarName and ModifyContract.
//
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004 
//    Added keepNodeZone.
//
//    Hank Childs, Fri Mar 11 15:00:05 PST 2005
//    Instantiate VTK filters on the fly.
//
//    Jeremy Meredith, Tue Jul  8 15:15:24 EDT 2008
//    Added ability to limit vectors to come from original cell only
//    (useful for material-selected vector plots).
//
//    Jeremy Meredith, Mon Jul 14 12:40:41 EDT 2008
//    Keep track of the approximate number of domains to be plotted.
//    This will let us calculate a much closer stride value if the
//    user requests a particular number of vectors to be plotted.
//
//    Hank Childs, Wed Dec 22 01:27:33 PST 2010
//    Add PostExecute method.
//
//    Eric Brugger, Tue Aug 19 11:50:28 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtVectorFilter : public avtDataTreeIterator
{
  public:
                              avtVectorFilter(bool us, int red);
    virtual                  ~avtVectorFilter();

    virtual const char       *GetType(void)   { return "avtVectorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating vectors"; };

    void                      SetStride(int);
    void                      SetNVectors(int);
    void                      SetMagVarName(const std::string &);
    void                      SetLimitToOriginal(bool);

  protected:
    bool                      useStride;
    int                       stride;
    int                       nVectors;
    bool                      origOnly;

    std::string               magVarName;
    bool                      keepNodeZone;
    int                       approxDomains;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
};


#endif


