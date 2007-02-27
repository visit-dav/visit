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
//                            avtTiledImageCompositor.C                      //
// ************************************************************************* //
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>
#include <avtTiledImageCompositor.h>
#include <vtkImageData.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Function:  AreaOwned
//
//  Purpose:
//    Returns the x,y lower (inclusive) limit and the x,y upper (exclusive)
//    limit on the region owned by any particular processor when doing
//    parallel scalable rendering with transparency and tiled image
//    compositing.
//
//  NOTE:  THIS CODE IS DUPLICATED IN vtkParallelImageSpaceRedistributor.C
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  1, 2004
//
// ****************************************************************************
static void AreaOwned(int rank, int size, int w, int h,
                      int &x1,int &y1, int &x2,int &y2)
{
    x1 = 0;
    x2 = w;
    y1 = (h*rank)/size;
    y2 = ((h*(rank+1))/size);
}

// ****************************************************************************
//  Constructor: avtTiledImageCompositor::avtTiledImageCompositor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
//  Modifications:
//
// ****************************************************************************

avtTiledImageCompositor::avtTiledImageCompositor()
{
   chunkSize = 1000000;
}


// ****************************************************************************
//  Destructor: avtTiledImageCompositor::~avtTiledImageCompositor
//
//  Programmer: Jeremy Meredith
//  Creation:   Mon Aug 30 15:53:33 PDT 2004
//
// ****************************************************************************

avtTiledImageCompositor::~avtTiledImageCompositor()
{
   inputImages.clear();
}


// ****************************************************************************
//  Method: Execute
//
//  Purpose: Perform the composition
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Mar 13 11:19:18 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtTiledImageCompositor::Execute(void)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int size = PAR_Size();

    if (inputImages.size() != 1)
    {
        EXCEPTION1(ImproperUseException, "avtTiledImageCompositor expected "
                   "only a single input image per processor.");
    }

    // Get the whole image size
    int width, height;
    inputImages[0]->GetImage().GetSize(&height, &width);

    // Figure out how much of the screen I own
    int x1,y1,x2,y2;
    AreaOwned(rank, size, width, height, x1,y1,x2,y2);
    int mywidth  = x2 - x1;
    int myheight = y2 - y1;

    unsigned char *inrgb = inputImages[0]->GetImage().GetRGBBuffer();

    // Create an output image if we are the root process
    vtkImageData *outputImageData = NULL;
    unsigned char *outrgb = NULL;
    if (rank == mpiRoot)
    {
        outputImageData = avtImageRepresentation::NewImage(width, height);
        outrgb = (unsigned char *)outputImageData->GetScalarPointer(0,0,0);
    }

    // Determine how many pixels need to be sent by each process
    // Note -- count is for each RGB component separately (thus the "*3")
    int pixelSize = mywidth * myheight * 3;
    int *pixelSizes = (rank == mpiRoot) ? new int[size] : NULL;
    MPI_Gather(&pixelSize, 1, MPI_INT,  pixelSizes, 1, MPI_INT,
               mpiRoot, VISIT_MPI_COMM);

    // Count 'em up
    // ... okay, so there's probably no point ....

    // Gather the pixels
    int *displacements = NULL;
    if (rank == mpiRoot)
    {
        displacements = new int[size];
        displacements[0] = 0;
        for (int i=1; i<size; i++)
        {
            displacements[i] = displacements[i-1] + pixelSizes[i-1];
        }
    }
    // NOTE: assumes all pixels are contiguous in memory!
    MPI_Gatherv(&inrgb[3*(width*y1+x1)], pixelSize, MPI_UNSIGNED_CHAR,
                outrgb, pixelSizes, displacements, MPI_UNSIGNED_CHAR,
                mpiRoot, VISIT_MPI_COMM);


    // Set the output
    avtImageRepresentation theOutput(outputImageData);
    SetOutputImage(theOutput);

    // Free the memory (yes, it is safe not to check for NULL in C++)
    if (outputImageData != NULL)
        outputImageData->Delete();
    delete[] pixelSizes;
    delete[] displacements;
#else
    SetOutputImage(inputImages[0]->GetImage());
#endif
}
