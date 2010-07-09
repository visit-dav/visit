/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <avtXRayImageQuery.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkBMPWriter.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkPointData.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>

#include <avtCallback.h>
#include <avtXRayFilter.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>

#include <vectortypes.h>

#include <visitstream.h>
#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <stdio.h>

// ****************************************************************************
//  Method: avtXRayImageQuery::avtXRayImageQuery
//
//  Purpose:
//    Basic constructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

avtXRayImageQuery::avtXRayImageQuery():
    absVarName("absorbtivity"),
    emisVarName("emissivity")
{
    origin[0] = 0.0f;
    origin[1] = 0.0f;
    origin[2] = 0.0f;
    theta  = 0.0f;
    phi    = 0.0f;
    width  = 1.0f;
    height = 1.0f;
    nx     = 10;
    ny     = 10;
    numPixels = nx * ny;
    divideEmisByAbsorb = false;
    outputType = 2; // png
    radBins = NULL;

    numBins = 1;
    numPixelsPerIteration = 40000;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::~avtXRayImageQuery
//
//  Purpose:
//    The destructor.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

avtXRayImageQuery::~avtXRayImageQuery()
{
    delete[] radBins;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetVariableNames
//
//  Purpose:
//    Set the names of absorbtion and emission variables.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetVariableNames(const stringVector &names)
{
    if (names.size() != 1 && names.size() != 2)
        EXCEPTION1(VisItException, "Not able to find the absorption and "
                   "emissivity variable names.");
    absVarName  = names[0];
    if (names.size() == 2)
        emisVarName = names[1];
    else
        // They put in the same name twice.
        emisVarName = names[0];
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetOrigin
//
//  Purpose:
//    Set the origin of the image plane.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetOrigin(float x, float y, float z)
{
    origin[0] = x;
    origin[1] = y;
    origin[2] = z;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetThetaPhi
//
//  Purpose:
//    Set the theta and phi offsets from the z-axis in 3D, or the x-axis in 2D.
//    In 3D, theta is a rotation about the x-axis, and phi is a rotation about
//    the z-axis.  In 2D, only theta is used.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetThetaPhi(float thetaInDegrees, float phiInDegrees)
{
    theta = thetaInDegrees * M_PI / 180.0;
    phi   = phiInDegrees * M_PI / 180.0;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetWidthHeight
//
//  Purpose:
//    Set the width and height of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetWidthHeight(float w, float h)
{
    width  = w;
    height = h;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetImageSize
//
//  Purpose:
//    Set the size of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetImageSize(int nx_, int ny_)
{
    nx = nx_;
    ny = ny_;
    numPixels = nx * ny;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetDivideEmisByAbsorb
//
//  Purpose:
//    Set the flag that controls if the emissivity divided by the absorbtivity
//    is used in place of the emissivity.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetDivideEmisByAbsorb(bool flag)
{
    divideEmisByAbsorb = flag;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetOutputType
//
//  Purpose:
//    Set the output image type.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(int type)
{
    outputType = type;
}


typedef struct
{
    int lineId;
    int ptId;
    double dist;
}  IdPoint;

// ****************************************************************************
//  Function: IdPointSorter
//
//  Purpose:
//      Comparison routine used to sort the line segments.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

static int
IdPointSorter(const void *arg1, const void *arg2)
{
    const IdPoint *r1 = (const IdPoint *) arg1;
    const IdPoint *r2 = (const IdPoint *) arg2;

    if (r1->lineId > r2->lineId)
        return 1;
    else if (r1->lineId < r2->lineId)
        return -1;

    if (r1->dist > r2->dist)
        return 1;
    else if (r1->dist < r2->dist)
        return -1;

    return 0;
}

// ****************************************************************************
//  Function: SortSegments
//
//  Purpose:
//      Sort the line segments by pixel id and distance.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

static int *
SortSegments(vtkPolyData *input)
{
    vtkIntArray *lineIdArray =
        (vtkIntArray *)input->GetPointData()->GetArray("LineIds");
    vtkDoubleArray *distsArray =
        (vtkDoubleArray *)input->GetPointData()->GetArray("Dists");

    int nLines = lineIdArray->GetNumberOfTuples();
    int *lineId = lineIdArray->GetPointer(0);
    double *dists = distsArray->GetPointer(0);

    //
    // Sort the distances for each line segment.
    //
    for (int i = 0; i < nLines; i++)
    {
        if (dists[i*2] > dists[i*2+1])
        {
            double tmp = dists[i*2];
            dists[i*2] = dists[i*2+1];
            dists[i*2+1] = tmp;
        }
    }

    //
    // Sort the segments by line id and distance.
    //
    IdPoint *idPoints = new IdPoint[nLines];
    for (int i = 0 ; i < nLines; i++)
    {
        idPoints[i].lineId = lineId[i]; // See assumption above
        idPoints[i].ptId = i;
        idPoints[i].dist = dists[i*2];
    }
    qsort(idPoints, nLines, sizeof(IdPoint), IdPointSorter);

    //
    // Form the output array.
    //
    int *compositeOrder = new int[nLines];
    for (int i = 0; i < nLines; i++)
        compositeOrder[i] = idPoints[i].ptId;

    delete [] idPoints;

    return compositeOrder;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::Execute
//
//  Purpose:
//      Integrates the line segments generated by the xray filter.  Note
//      that the data set consists of a bunch of polylines that only contain
//      field data.  The integration order is determined by sorting on line
//      id and then distance along the line.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(vtkDataSet *ds, const int chunk)
{
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(ImproperUseException);
    }

    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);

    vtkPolyData *pd = (vtkPolyData *) ds;

    int *compositeOrder = SortSegments(pd);

    IntegrateLines(pd, compositeOrder);
}


// ****************************************************************************
//  Method: avtXRayImageQuery::GetSecondaryVars
//
//  Purpose:
//    Adds the user-specified absorption and emission variables, using those 
//    for the query rather than the currently plotted variable.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::GetSecondaryVars(std::vector<std::string> &outVars)
{
    outVars.clear();
    outVars.push_back(absVarName);
    outVars.push_back(emisVarName);
}


// ****************************************************************************
//  Method: avtXRayImageQuery::Execute
//
//  Purpose:
//      Applies the xray filter to the data set and then integrates the
//      resulting lines. Because processing too many pixels can overflow
//      memory, it will execute the xray filter multiple times with a
//      small number of pixels each time.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(avtDataTree_p tree)
{
    avtDataset_p input = GetTypedInput();

    actualPixelsPerIteration = (numPixelsPerIteration / nx) * nx;

    pixelsForFirstPass = actualPixelsPerIteration;
    pixelsForLastPass = ((numPixels % actualPixelsPerIteration) == 0) ?
        actualPixelsPerIteration : numPixels % actualPixelsPerIteration;

    pixelsForFirstPassFirstProc = pixelsForFirstPass / PAR_Size() + 1;
    pixelsForFirstPassLastProc = 
        ((pixelsForFirstPass % pixelsForFirstPassFirstProc) == 0) ?
        pixelsForFirstPassFirstProc :
        pixelsForFirstPass % pixelsForFirstPassFirstProc;
    pixelsForLastPassFirstProc = pixelsForLastPass / PAR_Size() + 1;
    pixelsForLastPassLastProc =
        ((pixelsForLastPass % pixelsForLastPassFirstProc) == 0) ?
        pixelsForLastPassFirstProc :
        pixelsForLastPass % pixelsForLastPassFirstProc;

    avtContract_p contract =
        input->GetOriginatingSource()->GetGeneralContract();
    if (GetInput()->GetInfo().GetAttributes().ValidActiveVariable())
        varname = GetInput()->GetInfo().GetAttributes().GetVariableName();
    else
        varname = contract->GetDataRequest()->GetVariable();

    numPasses = numPixels / actualPixelsPerIteration;
    if (numPixels % actualPixelsPerIteration != 0)
        numPasses++;

    iFragment = 0;
    nImageFragments = numPasses;
    imageFragmentSizes = new int[nImageFragments];
    imageFragments = new float*[nImageFragments];

    int t1 = visitTimer->StartTimer();
    int iLine = 0;
    for (iPass = 0 ; iPass < numPasses ; iPass++)
    {
        int pixelsForThisPass = (iPass == numPasses - 1) ?
            pixelsForLastPass : pixelsForFirstPass;

        int pixelsForThisProc = pixelsForThisPass / PAR_Size() + 1;
        if (PAR_Rank() == PAR_Size() - 1)
            if (pixelsForThisPass % pixelsForThisProc != 0)
                pixelsForThisProc = pixelsForThisPass % pixelsForThisProc;

        imageFragmentSizes[iPass] = pixelsForThisProc;

        //
        // Create an artificial pipeline.
        //
        avtDataset_p ds;
        CopyTo(ds, input);
        avtSourceFromAVTDataset termsrc(ds);
        avtDataObject_p dob = termsrc.GetOutput();

        avtXRayFilter *filt = new avtXRayFilter;
        filt->SetImageProperties(origin, theta, phi, width, height, nx, ny);
        filt->SetNumberOfLines(pixelsForThisPass);
        filt->SetInitialLine(iLine);
        filt->SetInput(dob);
        //
        // Cause our artificial pipeline to execute.
        //
        filt->GetOutput()->Update(contract);

        avtDataset_p ds2 = filt->GetTypedOutput();
        avtDataTree_p tree = ds2->GetDataTree();
        ExecuteTree(tree);
        delete filt;

        //
        // Reset the timeout for the next iteration.
        //
        avtCallback::ResetTimeout(60*5);

        iLine += pixelsForThisPass;
    }
    visitTimer->StopTimer(t1, "avtXRayImageQuery::ExecutePipeline");
}


// ****************************************************************************
//  Method: avtXRayImageQuery::ExecuteTree
//
//  Purpose:
//      The mechanism for executing over a tree.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::ExecuteTree(avtDataTree_p inDT)
{
    if (*inDT == NULL)
    {
        return;
    }

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }

    if ( nc == 0 )
    {
        //
        // There is only one dataset to process.
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();

        Execute(in_ds, dom);
        if (!timeVarying)
        {
            currentNode++;
            UpdateProgress(currentNode, totalNodes);
        }
    }
    else
    {
        //
        // There is more than one input dataset to process.
        // Recurse through the children.
        //
        for (int j = 0; j < nc; j++)
        {
            if (inDT->ChildIsPresent(j))
            {
                Execute(inDT->GetChild(j));
            }
        }
    }
}


// ****************************************************************************
//  Method: avtXRayImageQuery::PreExecute
//
//  Purpose: 
//    This method is called before we start executing on the data.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::PreExecute()
{
    avtDatasetQuery::PreExecute();

    if (numBins <= 0)
    {
        EXCEPTION1(VisItException, "There must be at least one bin.");
    }
    if (numPixels <= 0)
    {
        EXCEPTION1(VisItException, "There must be at least one pixel.");
    }
}

// ****************************************************************************
//  Method: avtXrayImageQuery::PostExecute
//
//  Purpose:
//    Gather the results over all processes and output the images.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::PostExecute(void)
{
    int t1;
    if (PAR_Size() > 1)
    {
        //
        // Collect the images.
        //
        t1 = visitTimer->StartTimer();
        float *image = CollectImages(0, nImageFragments, imageFragmentSizes,
            imageFragments);
        visitTimer->StopTimer(t1, "avtXRayImageQuery::CollectImages");

        //
        // Swap out the current imageFragments and replace them with the
        // unified one.
        //
        for (int i = 0; i < nImageFragments; i++)
            delete [] imageFragments[i];
        nImageFragments = 1;
        imageFragmentSizes[0] = numPixels;
        imageFragments[0] = image;
    }

    //
    // Write out the image.
    //
    t1 = visitTimer->StartTimer();
    if (PAR_Rank() == 0)
    {
        if (outputType >= 0 && outputType <=3)
        {
            for (int i = 0; i < numBins; i++)
                WriteImage(i, nImageFragments, imageFragmentSizes,
                           imageFragments);
        }
        else if (outputType == 4)
        {
            for (int i = 0; i < numBins; i++)
                WriteFloats(i, nImageFragments, imageFragmentSizes,
                            imageFragments);
        }
    }
    visitTimer->StopTimer(t1, "avtXRayImageQuery::WriteImage");

    //
    // Delete the image.
    //
    for (int i = 0; i < nImageFragments; i++)
        delete [] imageFragments[i];
    delete [] imageFragments;
    delete [] imageFragmentSizes;

    //
    // Output the result message.
    //
    if (outputType >=0 && outputType <= 4)
    {
        string msg = "";
        char *exts[5] = {"bmp", "jpeg", "png", "tif", "bof"};
        char buf[512];
    
        if (numBins == 1)
            SNPRINTF(buf, 512, "The x ray image query results were written "
                     "to the file output00.%s\n", exts[outputType]);
        else
            SNPRINTF(buf, 512, "The x ray image query results were written "
                     "to the files output00.%s - output%02d.%s\n",
                     exts[outputType], numBins - 1, exts[outputType]);
        msg += buf;

        SetResultMessage(msg);
    }
    else
    {
        SetResultMessage("No x ray image query results were written "
                         "because the output type was invalid\n");
    }
}


// ****************************************************************************
//  Method: avtXRayImageQuery::IntegrateLines
//
//  Purpose:
//    Integrate the line segments using the order specified by segmentOrder.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::IntegrateLines(vtkPolyData *pd, int *segmentOrder)
{
    // 
    // Calculate the point id offset.
    //
    int pixelOffset = (iPass == (numPasses - 1)) ?
        PAR_Rank() * pixelsForLastPassFirstProc :
        PAR_Rank() * pixelsForFirstPassFirstProc;

    // Do Stuff
    if (pd == NULL)
    {
        const char *msg = "The image is blank. Please change the ray center,"
               " theta and phi, or the width and height to fit your simulation"
               " data.";
        EXCEPTION1(VisItException, msg);
    }

    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);

    //
    // Get the field variables.
    //
    vtkFloatArray *absorbtivityBins;
    vtkFloatArray *emissivityBins;

    absorbtivityBins = (vtkFloatArray *)pd->GetCellData()->GetArray(absVarName.c_str());
    emissivityBins   = (vtkFloatArray *)pd->GetCellData()->GetArray(emisVarName.c_str());
    if (absorbtivityBins == NULL)
    {
        char msg[256];
        if (pd->GetCellData()->GetArray(absVarName.c_str()) != NULL)
        {
            SNPRINTF(msg,256, "Failure: variable %s is node-centered, "
                              "but it must be zone-centered for this query.", 
                              absVarName.c_str());
        }
        else
            SNPRINTF(msg,256, "Variable %s not found.", absVarName.c_str());
        EXCEPTION1(VisItException, msg);
    }
    if (emissivityBins == NULL)
    {
        char msg[256];
        if (pd->GetCellData()->GetArray(emisVarName.c_str()) != NULL)
        {
            SNPRINTF(msg,256, "Failure: variable %s is node-centered, "
                              "but it must be zone-centered for this query.", 
                              emisVarName.c_str());
        }
        else
            SNPRINTF(msg,256, "Variable %s not found.", emisVarName.c_str());
        EXCEPTION1(VisItException, msg);
    }
    if ( emissivityBins->GetNumberOfComponents() != 
         absorbtivityBins->GetNumberOfComponents() )
    {
        EXCEPTION1(VisItException, "Number of bins for absorption and "
                                   "emission did not match.");
    }
    
    if (radBins == NULL)
    {
        numBins = emissivityBins->GetNumberOfComponents();
        radBins = new double[numBins];
    }
    else
    {
        if (numBins != emissivityBins->GetNumberOfComponents())
        {
            EXCEPTION1(VisItException,
                       "On nth pass, different number of bins found.");
        }
    }

    vtkIntArray *lineIdArray =
        (vtkIntArray *)pd->GetPointData()->GetArray("LineIds");
    vtkDoubleArray *distArray =
        (vtkDoubleArray *)pd->GetPointData()->GetArray("Dists");

    int nPts = lineIdArray->GetNumberOfTuples();
    int *lineId = lineIdArray->GetPointer(0);
    double *dist = distArray->GetPointer(0);

    float *absorbtivity = absorbtivityBins->GetPointer(0);
    float *emissivity = emissivityBins->GetPointer(0);

    //
    // Do the integration.
    //
    for (int i = 0 ; i < numBins ; i++)
    {
        radBins[i] = 0.;
    }

    int prevLineId = -1;

    imageFragments[iFragment] = new float[imageFragmentSizes[iFragment]*numBins];
    float *currentImageFragment = imageFragments[iFragment];
    for (int j = 0; j < imageFragmentSizes[iFragment]*numBins; j++)
        currentImageFragment[j] = 0.;
    iFragment++;

    for (int i = 0; i < nPts; i++)
    {
        int iPt = segmentOrder[i];

        if (lineId[iPt] != prevLineId)
        {
            if (prevLineId != -1)
            {
                for (int j = 0; j < numBins; j++)
                    currentImageFragment[(prevLineId-pixelOffset)*numBins+j] =
                        radBins[j];
            }

            for (int j = 0; j < numBins; j++)
            {
                radBins[j] = 0.;
            }
            prevLineId = lineId[iPt];
        }

        double segLength = dist[iPt*2+1] - dist[iPt*2];
        float *a = &(absorbtivity[iPt*numBins]);
        float *e = &(emissivity[iPt*numBins]);

        if (divideEmisByAbsorb)
        {
            for (int j = 0 ; j < numBins ; j++)
            {
                double tmp = exp(-a[j]*segLength);
                radBins[j] = radBins[j] * tmp + (e[j] / a[j]) * (1.0 - tmp);
            }
        }
        else
        {
            for (int j = 0 ; j < numBins ; j++)
            {
                double tmp = exp(-a[j]*segLength);
                radBins[j] = radBins[j] * tmp + e[j] * (1.0 - tmp);
            }
        }
    }

    if (prevLineId != -1)
    {
        for (int j = 0; j < numBins; j++)
            currentImageFragment[(prevLineId-pixelOffset)*numBins+j] =
                radBins[j];
    }
}


// ****************************************************************************
//  Method: avtXRayImageQuery::CollectImages
//
//  Purpose:
//      Collect the images on the first processor.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

float *
avtXRayImageQuery::CollectImages(int root, int nImageFragments,
    int *imageFragmentSizes, float **imageFragments)
{
#ifdef PARALLEL
    int nProcs = PAR_Size();

    //
    // Set up the send information.
    //
    int sendCount = 0;
    for (int i = 0; i < nImageFragments; i++)
        sendCount += imageFragmentSizes[i]*numBins;

    float *sendBuf = new float[sendCount];
    for (int i = 0, ndx = 0; i < nImageFragments; i++)
    {
        float *currentImageFragment = imageFragments[i];
        for (int j = 0; j < imageFragmentSizes[i]*numBins; j++)
        {
            sendBuf[ndx] = currentImageFragment[j];
            ndx++;
        }
    }

    //
    // Set up the receive information.
    //
    int *recvCounts = new int[nProcs];
    int *displs = new int[nProcs];
   
    recvCounts[0] = ((nImageFragments - 1) * pixelsForFirstPassFirstProc +
        pixelsForLastPassFirstProc) * numBins;
    displs[0] = 0;
    for (int i = 1; i < nProcs-1; i++)
    {
        recvCounts[i] = ((nImageFragments - 1) * pixelsForFirstPassFirstProc +
            pixelsForLastPassFirstProc) * numBins;
        displs[i] = displs[i-1] + recvCounts[i-1];
    }
    recvCounts[nProcs-1] = ((nImageFragments - 1) * pixelsForFirstPassLastProc +
        pixelsForLastPassLastProc) * numBins;
    displs[nProcs-1] = displs[nProcs-1-1] + recvCounts[nProcs-1-1];

    float *recvBuf = new float[numPixels * numBins];

    MPI_Gatherv(sendBuf, sendCount, MPI_FLOAT, recvBuf, recvCounts,
        displs, MPI_FLOAT, root, VISIT_MPI_COMM);

    //
    // Reorganize the receive buffer in the correct order.
    //
    float *image = new float[numPixels * numBins];
    int ii = 0;
    for (int i = 0; i < nImageFragments-1; i++)
    {
        for (int j = 0; j < nProcs-1; j++)
        {
            for (int k = 0; k < pixelsForFirstPassFirstProc*numBins; k++)
            {
                image[ii] = recvBuf[displs[j]];
                ii++;
                displs[j]++;
            }
        }
        int j = nProcs - 1;
        for (int k = 0; k < pixelsForFirstPassLastProc*numBins; k++)
        {
            image[ii] = recvBuf[displs[j]];
            ii++;
            displs[j]++;
        }
    }
    int i = nImageFragments - 1;
    for (int j = 0; j < nProcs-1; j++)
    {
        for (int k = 0; k < pixelsForLastPassFirstProc*numBins; k++)
        {
            image[ii] = recvBuf[displs[j]];
            ii++;
            displs[j]++;
        }
    }
    int j = nProcs - 1;
    for (int k = 0; k < pixelsForLastPassLastProc*numBins; k++)
    {
        image[ii] = recvBuf[displs[j]];
        ii++;
        displs[j]++;
    }

    delete [] recvBuf;

    return image;
#endif
}


// ****************************************************************************
//  Method: avtXRayImageQuery::WriteImage
//
//  Purpose:
//      Write the image in the appropriate format.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::WriteImage(int iImage, int nImageFragments,
    int *imageFragmentsSizes, float **imageFragments)
{
    float *currentImageFragment = imageFragments[0];
    float minVal = FLT_MAX;
    float maxVal = -FLT_MAX;
    for (int i = 0; i < nImageFragments; i++)
    {
        float *currentImageFragment = imageFragments[i];
        for (int j = 0; j < imageFragmentSizes[i]; j++)
        {
            if (currentImageFragment[j*numBins+iImage] > 0.)
            {
                minVal = currentImageFragment[j*numBins+iImage] < minVal ?
                    currentImageFragment[j*numBins+iImage] : minVal;
                maxVal = currentImageFragment[j*numBins+iImage] > maxVal ?
                    currentImageFragment[j*numBins+iImage] : maxVal;
            }
        }
    }
    float range = maxVal - minVal;
    
    vtkImageData *image = vtkImageData::New();
    image->SetWholeExtent(0, nx-1, 0, ny-1, 0, 0);
    image->SetUpdateExtent(0, nx-1, 0, ny-1, 0, 0);
    image->SetExtent(0, nx-1, 0, ny-1, 0, 0);
    image->SetSpacing(1., 1., 1.);
    image->SetOrigin(0., 0., 0.);
    image->SetNumberOfScalarComponents(3);
    image->SetScalarType(VTK_UNSIGNED_CHAR);
    image->AllocateScalars();
    unsigned char *pixels = (unsigned char *)image->GetScalarPointer(0, 0, 0);

    unsigned char pixel;
    unsigned char *ipixel = pixels;
    for (int i = 0; i < nImageFragments; i++)
    {
        float *currentImageFragment = imageFragments[i];
        for (int j = 0; j < imageFragmentSizes[i]; j++)
        {
            pixel = (unsigned char)
                ((currentImageFragment[j*numBins+iImage] < minVal) ? 0 : (254. *
                ((currentImageFragment[j*numBins+iImage] - minVal) / range)));
            *ipixel = pixel;
            ipixel++;
            *ipixel = pixel;
            ipixel++;
            *ipixel = pixel;
            ipixel++;
        }
    }

    if (outputType == 0)
    {
        vtkImageWriter *writer = vtkBMPWriter::New();
        char fileName[24];
        sprintf(fileName, "output%02d.bmp", iImage);
        writer->SetFileName(fileName);
        writer->SetInput(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 1)
    {
        vtkImageWriter *writer = vtkJPEGWriter::New();
        char fileName[24];
        sprintf(fileName, "output%02d.jpg", iImage);
        writer->SetFileName(fileName);
        writer->SetInput(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 2)
    {
        vtkImageWriter *writer = vtkPNGWriter::New();
        char fileName[24];
        sprintf(fileName, "output%02d.png", iImage);
        writer->SetFileName(fileName);
        writer->SetInput(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 3)
    {
        vtkImageWriter *writer = vtkTIFFWriter::New();
        char fileName[24];
        sprintf(fileName, "output%02d.tif", iImage);
        writer->SetFileName(fileName);
        writer->SetInput(image);
        writer->Write();
        writer->Delete();
    }
}


// ****************************************************************************
//  Method: avtXRayImageQuery::WriteFloats
//
//  Purpose:
//      Write the image as a block of floats.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::WriteFloats(int iImage, int nImageFragments,
    int *imageFragmentsSizes, float **imageFragments)
{
    float *fbuf = new float[nx*ny];

    float *ibuf = fbuf;
    for (int i = 0; i < nImageFragments; i++)
    {
        float *currentImageFragment = imageFragments[i];
        for (int j = 0; j < imageFragmentSizes[i]; j++)
        {
            *ibuf = currentImageFragment[j*numBins+iImage];
            ibuf++;
        }
    }

    char fileName[24];
    sprintf(fileName, "output%02d.bof", iImage);
    FILE *file = fopen(fileName, "w");
    fwrite(fbuf, sizeof(float), nx*ny, file);
    fclose(file);

    delete [] fbuf;
}
