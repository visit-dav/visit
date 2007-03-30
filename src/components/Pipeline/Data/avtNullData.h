// ************************************************************************* //
//                             avtNullData.h                                 //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_H
#define AVT_NULL_DATA_H
#include <pipeline_exports.h>
#include <ref_ptr.h>

#include <avtDataObject.h>

class avtDataObjectWriter;


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
                              avtNullData(avtDataObjectSource *src) : avtDataObject(src) {;};
    virtual                  ~avtNullData() {;};

    virtual const char       *GetType(void)  { return "avtNullData"; };
    avtDataObjectWriter      *InstantiateWriter(void);

  protected:
    virtual void              DerivedMerge(avtDataObject *dob) {;};

};

typedef ref_ptr<avtNullData>  avtNullData_p;

#endif
