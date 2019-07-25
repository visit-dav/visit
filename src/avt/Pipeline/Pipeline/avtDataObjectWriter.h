// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtDataObjectWriter.h                          //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_WRITER_H
#define AVT_DATA_OBJECT_WRITER_H

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <TypeRepresentation.h>

#include <avtDataset.h>
#include <avtImage.h>
#include <avtTerminatingSink.h>

class     avtDataObjectInformation;
class     avtDataObjectString;
class     AttributeGroup;


// ****************************************************************************
//  Class: avtDataObjectWriter
//
//  Purpose:
//      A module that you can hand a data object (meaning dataset or image to)
//      and it will set up the correct writer.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Feb 12 08:38:18 PST 2001
//    Added methods to get the input out for the ui process in parallel.
//
//    Jeremy Meredith, Thu Jul 26 12:36:30 PDT 2001
//    Made it derive from avtTerminatingSink so we could do dynamic
//    load balancing.
//
//    Hank Childs, Mon Oct  1 08:56:17 PDT 2001
//    Made this be an inheritance layer rather than a container for different
//    types of data objects.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Mark C. Miller, Mon Oct 31 18:12:49 PST 2005
//    Added code to support compression of data object string
//
//    Kathleen Bonnell, Mon Nov 27 12:32:24 PST 2006
//    Added WriteAtts method. 
//
// ****************************************************************************

class PIPELINE_API avtDataObjectWriter : virtual public avtTerminatingSink
{
  public:
                               avtDataObjectWriter();
    virtual                   ~avtDataObjectWriter();

    void                       Write(avtDataObjectString &);

    void                       SetDestinationFormat(const TypeRepresentation&);
    void                       SetUseCompression(bool val) { useCompression = val; };

    void                       WriteInt(avtDataObjectString &, int) const;
    void                       WriteFloat(avtDataObjectString &, float) const;
    void                       WriteDouble(avtDataObjectString &,double) const;
    void                       WriteInt(avtDataObjectString &, const int *,
                                        int) const;
    void                       WriteFloat(avtDataObjectString &, const float *,
                                          int) const;
    void                       WriteDouble(avtDataObjectString &,
                                           const double *, int) const;
    void                       WriteAtts(avtDataObjectString &,
                                         AttributeGroup *) const;

    virtual bool               MustMergeParallelStreams(void) { return false;};

  protected:
    TypeRepresentation         sourceFormat;
    TypeRepresentation         destinationFormat;
    bool                       useCompression;

    virtual void               DataObjectWrite(avtDataObjectString &) = 0;
};


typedef  ref_ptr<avtDataObjectWriter>  avtDataObjectWriter_p;


#endif


