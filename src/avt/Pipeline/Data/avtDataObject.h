// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtDataObject.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_H
#define AVT_DATA_OBJECT_H

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataObjectInformation.h>
#include <avtContract.h>

class     avtDataObjectSource;
class     avtDataObjectWriter;
class     avtQueryableSource;
class     avtOriginatingSource;
class     avtWebpage;


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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sat Feb 19 14:49:17 PST 2005
//    Added method to get the source of the data object.
//
//    Hank Childs, Thu May 25 16:40:20 PDT 2006
//    Add support for releasing data in the middle of a pipeline execution.
//
//    Hank Childs, Thu Dec 21 09:58:57 PST 2006
//    Added method for debug dumps.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Mon Feb  2 09:26:16 PST 2009
//    Added data member for the contract from a previous execution.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
//    Kathleen Biagas, Mon Jun  5 16:21:30 PDT 2017
//    Add ResetAllExtents.
//
// ****************************************************************************

class PIPELINE_API avtDataObject
{
  public:
                                     avtDataObject(avtDataObjectSource *);
    virtual                         ~avtDataObject();

    avtOriginatingSource            *GetOriginatingSource(void);
    avtQueryableSource              *GetQueryableSource(void);

    bool                             Update(avtContract_p);
    void                             ResetAllExtents(void);
    void                             SetSource(avtDataObjectSource *);
    avtDataObjectSource             *GetSource(void) { return source; };

    avtDataObjectInformation        &GetInfo(void) { return info; };
    const avtDataObjectInformation  &GetInfo(void) const { return info; };
    virtual long long                GetNumberOfCells(bool polysOnly = false)
                                                                     const = 0;

    virtual const char              *GetType(void) = 0;
    virtual void                     ReleaseData(void);

    avtDataObject                   *Clone(void);
    void                             Copy(avtDataObject *);
    void                             Merge(avtDataObject *, bool = false);
    virtual avtDataObject           *Instance(void);

    virtual avtDataObjectWriter     *InstantiateWriter(void);
    
    void                             SetTransientStatus(bool b)
                                                            { transient = b; };
    bool                             IsTransient(void) { return transient; };
    avtContract_p                    GetContractFromPreviousExecution(void)
                                         { return contractFromPrevExecution; };

    virtual void                     DebugDump(avtWebpage *, const char *);

  protected:
    avtDataObjectInformation         info;
    avtDataObjectSource             *source;

    // "Transient" implies that this object is only needed so that a filter
    // can process it and calculate its output.  Almost all data objects are
    // "transient".  An example of a non-transient data object is the
    // "intermediate data object" of a plot, which must not be deleted, 
    // because we need it for queries.
    bool                             transient;

    avtContract_p                    contractFromPrevExecution;

    virtual void                     DerivedCopy(avtDataObject *);
    virtual void                     DerivedMerge(avtDataObject *);
    void                             CompatibleTypes(avtDataObject *);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataObject(const avtDataObject &) {;};
    avtDataObject       &operator=(const avtDataObject &) { return *this; };
};


typedef ref_ptr<avtDataObject> avtDataObject_p;


#endif


