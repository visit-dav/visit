// ************************************************************************* //
//                              avtDataObject.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_H
#define AVT_DATA_OBJECT_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObjectInformation.h>
#include <avtPipelineSpecification.h>

class     avtDataObjectSource;
class     avtDataObjectWriter;
class     avtTerminatingSource;
class     avtQueryableSource;


// **************************************************************************** 
//  Class: avtDataObject
//
//  Purpose:
//      The is the fundamental type handed around by process objects.  A data
//      object contains data in some form.  The base class is defined to handle
//      issues with the Update/Execute model.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2001
//
// ****************************************************************************

class PIPELINE_API avtDataObject
{
  public:
                                     avtDataObject(avtDataObjectSource *);
    virtual                         ~avtDataObject() {;};

    avtTerminatingSource            *GetTerminatingSource(void);
    avtQueryableSource              *GetQueryableSource(void);

    bool                             Update(avtPipelineSpecification_p);
    void                             SetSource(avtDataObjectSource *);

    avtDataObjectInformation        &GetInfo(void) { return info; };
    const avtDataObjectInformation  &GetInfo(void) const { return info; };
    virtual int                      GetNumberOfCells(bool polysOnly = false) const = 0;

    virtual const char              *GetType(void) = 0;
    virtual void                     ReleaseData(void);

    avtDataObject                   *Clone(void);
    void                             Copy(avtDataObject *);
    void                             Merge(avtDataObject *, bool = false);
    virtual avtDataObject           *Instance(void);

    virtual avtDataObjectWriter     *InstantiateWriter(void);
    
  protected:
    avtDataObjectInformation         info;
    avtDataObjectSource             *source;

    virtual void                     DerivedCopy(avtDataObject *);
    virtual void                     DerivedMerge(avtDataObject *);
    void                             CompatibleTypes(avtDataObject *);
};


typedef ref_ptr<avtDataObject> avtDataObject_p;


#endif


