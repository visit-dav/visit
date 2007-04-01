// ************************************************************************* //
//                              avtNullData.C                                //
// ************************************************************************* //

#include <avtNullData.h>

#include <avtNullDataWriter.h>


// ****************************************************************************
//  Method: avtNullData destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullData::~avtNullData()
{
    ;
}


// ****************************************************************************
//  Method: avtNullData::Instance
//
//  Purpose:
//      Creates an instance of an avtNullData taking care to copy the incomming
//      null data object's type string.
//
//  Programmer: Mark C. Miller
//  Creation:   February 4, 2004
//
// ****************************************************************************

avtDataObject *
avtNullData::Instance(void)
{
    avtDataObjectSource *src = NULL;
    avtNullData *nd = new avtNullData(src,GetType());
    return nd;
}

// ****************************************************************************
//  Method: avtNullData::InstantiateWriter
//
//  Arguments:
//      src     An data object source that is the upstream object.
//
//  Programmer: Mark C. Miller
//  Creation:   March 12, 2003 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added bool argument to call to construct the writer
//
// ****************************************************************************

avtDataObjectWriter *
avtNullData::InstantiateWriter()
{
   return new avtNullDataWriter(writerShouldMergeParallelStreams);
}
