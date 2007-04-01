// ************************************************************************* //
//                              avtNullData.C                                //
// ************************************************************************* //

#include <avtNullData.h>
#include <avtNullDataWriter.h>

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
// ****************************************************************************

avtDataObjectWriter *
avtNullData::InstantiateWriter(void)
{
   return new avtNullDataWriter;
}
