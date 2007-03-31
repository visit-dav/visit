// ************************************************************************* //
//                             avtNullData.h                                 //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_H
#define AVT_NULL_DATA_H
#include <string>
#include <pipeline_exports.h>
#include <ref_ptr.h>

#include <avtDataObject.h>

class avtDataObjectWriter;

const char * const AVT_NULL_IMAGE_MSG = "avtImage withheld; under scalable threshold";
const char * const AVT_NULL_DATASET_MSG = "avtDataset withheld; over scalable threshold";

// ****************************************************************************
//  Class: avtNullData
//
//  Purpose:
//      This is another data object that avt pipelines can handle. It
//      represents no data.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003
//
// ****************************************************************************

class PIPELINE_API avtNullData : public avtDataObject
{
  public:
                              avtNullData(avtDataObjectSource *src, const char *_typeStr = NULL)
                                 : avtDataObject(src)
                                 { typeStr = _typeStr ? _typeStr : "avtNullData"; };
    virtual                  ~avtNullData() {;};

    virtual const char       *GetType(void)  { return typeStr.c_str(); };
    virtual int               GetNumberOfCells(void) const { return 0;};
    virtual void              SetType(char *_typeStr )  { typeStr = _typeStr; };
    avtDataObjectWriter      *InstantiateWriter(void);

  protected:
    virtual void              DerivedMerge(avtDataObject *dob) {;};

  private:
     std::string              typeStr;
};

typedef ref_ptr<avtNullData>  avtNullData_p;

#endif
