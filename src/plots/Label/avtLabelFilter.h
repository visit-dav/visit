// ************************************************************************* //
//                              avtLabelFilter.h                             //
// ************************************************************************* //

#ifndef AVT_Label_FILTER_H
#define AVT_Label_FILTER_H


#include <avtStreamer.h>

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
//
// ****************************************************************************

class avtLabelFilter : public avtStreamer
{
  public:
                              avtLabelFilter();
    virtual                  ~avtLabelFilter();

    virtual const char       *GetType(void)   { return "avtLabelFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Labelling values"; };

    void                      SetLabelVariable(const char *);
  protected:
    enum QuantizationRetval
    {
        NoAction,
        DiscardedQuantizedNormals,
        CreatedQuantizedNormals
    };

    unsigned char             FindClosestVector(const float *vec) const;
    QuantizationRetval        CreateQuantizedNormalsFromPointNormals(
                                  vtkDataSet *outDS, bool isMesh);
    QuantizationRetval        CreateQuantizedNormalsFromCellNormals(
                                  vtkDataSet *outDS, bool isMesh);

    char                     *labelVariable;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
};


#endif
