// ************************************************************************* //
//                         avtDataObjectInformation.C                        //
// ************************************************************************* //

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtDataObjectInformation.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>

static avtDataObjectWriter_p unifyDobInfo_writer;
// ****************************************************************************
//  Function: UnifyDobInfo
//
//  Purpose: User defined MPI reduce operator for unifying data object
//  information
//
//  I have to pass an avtDataObjectWriter into this function, somehow and the
//  API is set by MPI. So, I've resorted to a file-scope global
//
//  Programmer: Mark C. Miller
//  Created:    20Mar03
// ****************************************************************************
#ifdef PARALLEL
static void
UnifyDobInfo(void *ibuf, void *iobuf, int *count, MPI_Datatype *datatype)
{
   char  *istr = (char *) ibuf;
   char *iostr = (char *) iobuf;
   avtDataObjectInformation  iDobInfo;
   avtDataObjectInformation ioDobInfo;
 
   int len;
   MPI_Type_size(*datatype, &len);
 
   // convert from string rep. back to real avtDataObjectInformation objects
   iDobInfo.Read(istr);
   ioDobInfo.Read(iostr);
 
   // merge infos
   ioDobInfo.Merge(iDobInfo);
 
   // convert back to string rep.
   avtDataObjectString ostr;
   ioDobInfo.Write(ostr, *unifyDobInfo_writer);
   char *tmp; int tmpLen;
   ostr.GetWholeString(tmp,tmpLen);
 
   // check lengths
   if (tmpLen > len)
      MPI_Abort(MPI_COMM_WORLD, 1);
 
   // copy string to output buffer
   memcpy(iobuf, tmp, tmpLen);
 
}
#endif

// initialize static class members
int avtDataObjectInformation::objectCount = 0;

#ifdef PARALLEL
MPI_Op avtDataObjectInformation::mpiOpUnifyDobInfo = MPI_OP_NULL;

// ****************************************************************************
// Method:    avtDataObjectInformation::InitializeMPIStuff
//
// Purpose:      Build MPI objects necessary to support avtWholeImageCompositer
//               class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03
// ****************************************************************************
 
void
avtDataObjectInformation::InitializeMPIStuff(void)
{
   MPI_Op_create(UnifyDobInfo, 1, &avtDataObjectInformation::mpiOpUnifyDobInfo);
}

// ****************************************************************************
// Function:    FinalizeMPIStuff
//
// Purpose:      Free MPI objects used support avtWholeImageCompositer class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03
// ****************************************************************************
void
avtDataObjectInformation::FinalizeMPIStuff(void)
{
   MPI_Op_free(&avtDataObjectInformation::mpiOpUnifyDobInfo);
}

#else

void avtDataObjectInformation::InitializeMPIStuff(void) {;};
void avtDataObjectInformation::FinalizeMPIStuff(void) {;};

#endif


// ****************************************************************************
//  Method: avtDataObjectInformation::avtDataObjectInformation
//
//  Purpose: Constructor
//
//  Programmer: Mark C. Miller
//  Creation:   01Apr03
//
// ****************************************************************************

avtDataObjectInformation::avtDataObjectInformation()
{
   if (avtDataObjectInformation::objectCount == 0)
      InitializeMPIStuff();
   avtDataObjectInformation::objectCount++;
}

// ****************************************************************************
//  Method: avtDataObjectInformation::~avtDataObjectInformation
//
//  Purpose: Destructor 
//
//  Programmer: Mark C. Miller
//  Creation:   01Apr03
//
// ****************************************************************************

avtDataObjectInformation::~avtDataObjectInformation()
{
   avtDataObjectInformation::objectCount--;
   if (avtDataObjectInformation::objectCount == 0)
      FinalizeMPIStuff();
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Copy
//
//  Purpose:
//      Copies the data object information from the argument to this object.
//
//  Arguments:
//      di      Data object information from another object.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Mar 24 15:21:04 PST 2001
//    Re-wrote function, blew away previous comments.
//
// ****************************************************************************

void
avtDataObjectInformation::Copy(const avtDataObjectInformation &di)
{
    atts.Copy(di.atts);
    validity.Copy(di.validity);
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Merge
//
//  Purpose:
//      Merges the data object information from the argument and this object.
//
//  Arguments:
//      di      Data object information from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
// ****************************************************************************

void
avtDataObjectInformation::Merge(const avtDataObjectInformation &di)
{
    atts.Merge(di.atts);
    validity.Merge(di.validity);
}

// ****************************************************************************
//  Method: avtDataObjectInformation::ParallelMerge
//
//  Purpose:
//      Merges the data object information from this object across all
//      processors
//
//  Arguments:
//      dobw data object writer used in .Write calls 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 1, 2003
//
// ****************************************************************************

void
avtDataObjectInformation::ParallelMerge(const avtDataObjectWriter_p dobw)
{
#ifdef PARALLEL

   MPI_Datatype mpiTypeDobStr;
 
   // convert this processor's avtDataObjectInformation to its char* rep.
   avtDataObjectString localDobStr;
   Write(localDobStr, *dobw);
   char *localStr; int len, reducedLen;
   localDobStr.GetWholeString(localStr,len);
 
   // make sure all processors agree on len of string
   MPI_Allreduce(&len, &reducedLen, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

   // create storage for copy and reduced result
   char *copiedStr = new char [reducedLen];
   char *reducedStr = new char [reducedLen];

   // copy localStr to the copiedStr buffer for MPI
   strncpy(copiedStr, localStr, len);

   // create mpi type to circumvent possible buffer chopping during reduce
   MPI_Type_contiguous(reducedLen, MPI_CHAR, &mpiTypeDobStr);
   MPI_Type_commit(&mpiTypeDobStr);

   // setup unifyDobInfo_writer variable so UnifyDobInfo operator can see it
   unifyDobInfo_writer = dobw;
 
   // do the parallel reduction on char* rep of avtDataObjectInformation
   MPI_Allreduce(copiedStr, reducedStr, 1, mpiTypeDobStr,
      avtDataObjectInformation::mpiOpUnifyDobInfo, MPI_COMM_WORLD);
 
   // convert char* rep. back to real avtDataObjectInformation objects
   avtDataObjectInformation reducedDobInfo;
   reducedDobInfo.Read(reducedStr);
 
   // override data object information of the writer's output avtDataObject
   Copy(reducedDobInfo);

   // indicate that it is ok to use cummulative true extents as true extents;
   GetAttributes().SetCanUseCummulativeAsTrueOrCurrent(true);
 
   // cleanup
   MPI_Type_free(&mpiTypeDobStr);
   delete [] reducedStr;
   delete [] copiedStr;

#endif
}

// ****************************************************************************
//  Method: avtDataObjectInformation::Write
//
//  Purpose:
//      Writes the data object information to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
// ****************************************************************************

void
avtDataObjectInformation::Write(avtDataObjectString &str, 
                                const avtDataObjectWriter *wrtr)
{
    atts.Write(str, wrtr);
    validity.Write(str, wrtr);
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Read
//
//  Purpose:
//      Reads the data object information from a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
// ****************************************************************************

int
avtDataObjectInformation::Read(char *str)
{
    int size1 = atts.Read(str);
    int size2 = validity.Read(str + size1);
    return (size1 + size2);
}


