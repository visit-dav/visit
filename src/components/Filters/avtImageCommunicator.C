/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtImageCommunicator.C                         //
// ************************************************************************* //

#include <avtImageCommunicator.h>

#include <vtkImageData.h>

#include <avtImagePartition.h>
#include <avtImageRepresentation.h>
#include <avtParallel.h>

#include <ImproperUseException.h>
#include <TimingsManager.h>


//
// Most of the code for the communicator depends on mpi calls, so ifdef out
// parallel code out if we are not running in parallel.
//
#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
//  Method: avtImageCommunicator constructor
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

avtImageCommunicator::avtImageCommunicator()
{
#ifdef PARALLEL
    MPI_Comm_size(VISIT_MPI_COMM, &numProcs);
    MPI_Comm_rank(VISIT_MPI_COMM, &myRank);
#else
    numProcs = 1; myRank = 0;
#endif
    imagePartition = NULL;
}


// ****************************************************************************
//  Method: avtImageCommunicator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageCommunicator::~avtImageCommunicator()
{
    ;
}


// ****************************************************************************
//  Method: avtImageCommunicator::SetImagePartition
//
//  Purpose:
//      Sets the image partition to be used for the image communicator.
//
//  Arguments:
//      ip       The image partition to use.
//
//  Programmmer: Hank Childs
//  Creation:    March 6, 2001
//
// ****************************************************************************

void
avtImageCommunicator::SetImagePartition(avtImagePartition *ip)
{
    imagePartition = ip;
}


// ****************************************************************************
//  Method: avtImageCommunicator::Execute
//
//  Purpose:
//      Communicates the image to processor 0.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar  6 10:33:51 PST 2001
//    Accounted for image coming out-of-order from dynamic image partitioning.
//
//    Eric Brugger, Mon Nov  5 13:42:17 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Mon Dec 17 17:56:32 PST 2001
//    Make initialization to prevent UMR.
//
//    Hank Childs, Tue Jan  1 14:00:57 PST 2002
//    Reflect new interface for avtImagePartition.
//
//    Hank Childs, Thu Jan  3 09:50:29 PST 2002
//    Allow for NULL images to be passed down from the compositer -- this
//    case occurs when an empty partition is assigned to a processor.
//
//    Hank Childs, Fri Jan  4 10:03:52 PST 2002
//    Account for case when processor 0 got an empty partition.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtImageCommunicator::Execute(void)
{
#ifdef PARALLEL
    int timingsIndex = visitTimer->StartTimer();

    if (imagePartition == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    //
    // Get the scanlines for this processor.
    //
    int minW, maxW, minH, maxH;
    imagePartition->GetThisPartition(minW, maxW, minH, maxH);
    int width = (maxW-minW)+1;
    if (width < 0)
        width = 0;
    int height = (maxH-minH)+1;
    if (height < 0)
        height = 0;

    unsigned char *data = NULL;
    if (GetImageRep().Valid() && width > 0 && height > 0)
    {
        vtkImageData  *image = GetImageRep().GetImageVTK();
        data   = (unsigned char *) image->GetScalarPointer(0, 0, 0);
    }

    //
    // Determine the size of this image.
    //
    int  size = 3 * width * height;   // 3 = one char per r, g, b.

    //
    // Gather the sizes on processor 0.
    //
    int *sizelist = NULL;
    if (myRank == 0)
    {
        sizelist = new int[numProcs];
    }
    MPI_Gather(&size, 1, MPI_INT, sizelist, 1, MPI_INT, 0, VISIT_MPI_COMM);

    //
    // Calculate the displacements for the gathering of the data.
    //
    int *disp = NULL;
    if (myRank == 0)
    {
        disp = new int[numProcs];
        disp[0] = 0;
        for (int i = 1 ; i < numProcs ; i++)
        {
            disp[i] = disp[i-1] + sizelist[i-1];
        }
    }

    //
    // Make an array that the data can go onto.
    //
    unsigned char *alldata = NULL;
    int total = 0;
    if (myRank == 0)
    {
        for (int i = 0 ; i < numProcs ; i++)
        {
            total += sizelist[i];
        }
        alldata = new unsigned char[total];
    }

    //
    // Gather all of the data onto processor 0.
    //
    MPI_Gatherv(data, size, MPI_UNSIGNED_CHAR, alldata, sizelist, disp,
                MPI_UNSIGNED_CHAR, 0, VISIT_MPI_COMM);

    //
    // Create the VTK image from all of the scanlines.
    //
    if (myRank == 0)
    {
        height = imagePartition->GetTileHeight();
        width  = imagePartition->GetTileWidth();
        vtkImageData *newimage = avtImageRepresentation::NewImage(width,
                                                                  height);

        //
        // We can get the image buffer directly from the VTK object.
        //
        unsigned char *d = (unsigned char *)newimage->GetScalarPointer(0, 0,0);

        //
        // Get the image partition assignments from the image partition.  If
        // assignments are thought of as a function, then the domain is
        // the partitions and the range is the processors.
        //
        const int *assignments = 
                          imagePartition->GetPartitionToProcessorAssignments();

        //
        // The messages have come out-of-order, so sort them as they are copied
        // into the image buffer.
        //
        char *tmp = (char *) d;
        int numParts = numProcs; // For clarity of what we are iterating over.
        for (int i = 0 ; i < numParts ; i++)
        {
            int p = assignments[i]; // the processor that partition i sits on.
            memcpy(tmp, ((char *)alldata) + disp[p], sizelist[p]);
            tmp += sizelist[p];
        }

        SetOutputImage(newimage);
        newimage->Delete();
        delete [] alldata;
        delete [] sizelist;
        delete [] disp;
    }

    visitTimer->StopTimer(timingsIndex, "Image Communication");
#endif
}



