// ************************************************************************* //
//                              avtNullData.C                                //
// ************************************************************************* //

#include <avtNullData.h>
#include <avtNullDataWriter.h>

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
