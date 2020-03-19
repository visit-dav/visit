// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtLabelFilter.h                             //
// ************************************************************************* //

#ifndef AVT_Label_FILTER_H
#define AVT_Label_FILTER_H

#include <avtDataTreeIterator.h>
#include <vtkSystemIncludes.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtLabelFilter
//
//  Purpose:
//      This operator is the implied operator associated with an Label plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jan 22 11:52:38 PDT 2004
//
//  Modifications:
//    Eric Brugger, Tue Aug 19 10:32:52 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kathleen Biagas, Wed Jun  3 10:28:07 PDT 2015
//    Added mayBeLogical, cellOrigin, nodeOrigin, to aid in calculating
//    logical indices.
//
//    Alister Maguire, Mon Feb 26 09:28:05 PST 2018
//    Added ModifyContract to turn on MIR if needed.  
//
// ****************************************************************************

class avtLabelFilter : public avtDataTreeIterator
{
  public:
                              avtLabelFilter();
    virtual                  ~avtLabelFilter();

    virtual const char       *GetType(void)   { return "avtLabelFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Labelling values"; };

    void                      SetLabelVariable(const char *);
    void                      SetMayBeLogical(bool val) 
                                  { mayBeLogical = val; }
    void                      SetCellOrigin(vtkIdType val) 
                                  { cellOrigin = val; }
    void                      SetNodeOrigin(vtkIdType val) 
                                  { nodeOrigin = val; }
  protected:
    enum QuantizationRetval
    {
        NoAction,
        DiscardedQuantizedNormals,
        CreatedQuantizedNormals
    };

    unsigned char             FindClosestVector(const double *vec) const;
    QuantizationRetval        CreateQuantizedNormalsFromPointNormals(
                                  vtkDataSet *outDS, bool isMesh);
    QuantizationRetval        CreateQuantizedNormalsFromCellNormals(
                                  vtkDataSet *outDS, bool isMesh);

    char                     *labelVariable;
    bool                      mayBeLogical;
    vtkIdType                 cellOrigin;
    vtkIdType                 nodeOrigin;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
};


#endif
