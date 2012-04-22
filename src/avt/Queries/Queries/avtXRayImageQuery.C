/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <vtkBMPWriter.h>
#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkPointData.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>

#include <avtDatasetExaminer.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtXRayFilter.h>

#include <vectortypes.h>

#include <visitstream.h>
#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <QueryArgumentException.h>

#include <float.h>
#include <stdio.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtXRayImageQuery::avtXRayImageQuery
//
//  Purpose:
//    Basic constructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Sep  7 16:23:06 PDT 2010
//    I reduced the number of pixels per iteration, since it doesn't
//    increase the run time significantly, but reduces the memory
//    requirements significantly.
//
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Kathleen Biagas, Tue Jul 26 12:36:11 PDT 2011
//    Changed default nx,ny to 200 (per Eric).
//
// ****************************************************************************

avtXRayImageQuery::avtXRayImageQuery():
    absVarName("absorbtivity"),
    emisVarName("emissivity")
{
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    theta  = 0.0;
    phi    = 0.0;
    width  = 1.0;
    height = 1.0;
    nx     = 200;
    ny     = 200;
    numPixels = nx * ny;
    divideEmisByAbsorb = false;
    outputType = 2; // png
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
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetInputParams
//
//  Purpose:
//    Set the input parameters.
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 17, 2011
//
// ****************************************************************************

void
avtXRayImageQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("vars"))
        SetVariableNames(params.GetEntry("vars")->AsStringVector());
    else
        EXCEPTION1(QueryArgumentException, "vars");

    if (params.HasEntry("origin"))
    {
        if (params.GetEntry("origin")->TypeName() == "doubleVector")
            SetOrigin(params.GetEntry("origin")->AsDoubleVector());
        else if (params.GetEntry("origin")->TypeName() == "intVector")
            SetOrigin(params.GetEntry("origin")->AsIntVector());
    }

    if (params.HasEntry("theta"))
    {
        if (params.GetEntry("theta")->TypeName() == "double")
            SetTheta(params.GetEntry("theta")->AsDouble());
        else if (params.GetEntry("theta")->TypeName() == "int")
            SetTheta(params.GetEntry("theta")->AsInt());
    }

    if (params.HasEntry("phi"))
    {
        if (params.GetEntry("phi")->TypeName() == "double")
            SetPhi(params.GetEntry("phi")->AsDouble());
        else if (params.GetEntry("phi")->TypeName() == "int")
            SetPhi(params.GetEntry("phi")->AsInt());
    }

    if (params.HasEntry("width"))
    {
        if (params.GetEntry("width")->TypeName() == "double")
            SetWidth(params.GetEntry("width")->AsDouble());
        else if (params.GetEntry("width")->TypeName() == "int")
            SetWidth(params.GetEntry("width")->AsInt());
    }

    if (params.HasEntry("height"))
    {
        if (params.GetEntry("height")->TypeName() == "double")
            SetHeight(params.GetEntry("height")->AsDouble());
        else if (params.GetEntry("height")->TypeName() == "int")
            SetHeight(params.GetEntry("height")->AsInt());
    }

    if (params.HasEntry("image_size"))
        SetImageSize(params.GetEntry("image_size")->AsIntVector());

    if (params.HasEntry("divide_emis_by_absorb"))
        SetDivideEmisByAbsorb(params.GetEntry("divide_emis_by_absorb")->AsInt());

    if (params.HasEntry("output_type"))
    {
        if (params.GetEntry("output_type")->TypeName() == "int")
            SetOutputType(params.GetEntry("output_type")->AsInt());
        else if (params.GetEntry("output_type")->TypeName() == "string")
            SetOutputType(params.GetEntry("output_type")->AsString());
    }
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
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to doubleVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetOrigin(const doubleVector &_origin)
{
    origin[0] = _origin[0];
    origin[1] = _origin[1];
    origin[2] = _origin[2];
}

void
avtXRayImageQuery::SetOrigin(const intVector &_origin)
{
    origin[0] = (double)_origin[0];
    origin[1] = (double)_origin[1];
    origin[2] = (double)_origin[2];
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetTheta
//
//  Purpose:
//    Set the theta and phi offsets from the z-axis in 3D, or the x-axis in 2D.
//    In 3D, theta is a rotation about the x-axis, and phi is a rotation about
//    the z-axis.  In 2D, only theta is used.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to doubleVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetTheta(const double &thetaInDegrees)
{
    theta = thetaInDegrees * M_PI / 180.0;
}

void
avtXRayImageQuery::SetPhi(const double &phiInDegrees)
{
    phi   = phiInDegrees * M_PI / 180.0;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetWidth
//
//  Purpose:
//    Set the width of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayImageQuery::SetWidth(const double &size) 
{
    width  = size;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::SetHeight
//
//  Purpose:
//    Set the width of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayImageQuery::SetHeight(const double &size) 
{
    height = size;
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
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to intVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetImageSize(const intVector &size)
{
    nx = size[0];
    ny = size[1];
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


// ****************************************************************************
//  Method: avtXRayImageQuery::SetOutputType
//
//  Purpose:
//    Set the output image type.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 13, 2011
//
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(const std::string &type)
{
    if      (type == "bmp")
        outputType = 0;
    else if (type == "jpeg")
        outputType = 1;
    else if (type == "png")
        outputType = 2;
    else if (type == "tif")
        outputType = 3;
    else if (type == "rawfloats")
        outputType = 4;
    else if (type == "bof")
        outputType = 4;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::Execute
//
//  Purpose:
//    Dummy routine used to satisfy a pure virtual method.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(vtkDataSet *ds, const int chunk)
{
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
//    Applies the xray filter to the data set and writes out the resulting
//    images.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Jul 16 15:43:35 PDT 2010
//    I modified the query to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Tue Sep  7 16:23:06 PDT 2010
//    I added logic to detect RZ meshes with negative R values up front
//    before any processing to avoid more complex error handling during
//    pipeline execution.
//
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(avtDataTree_p tree)
{
    avtDataset_p input = GetTypedInput();

    //
    // If the number of pixels is less than or equal to zero then print
    // an error message and exit.
    //
    if (numPixels <= 0)
    {
        SetResultMessage("VisIt is unable to execute this query because "
                         "the number of pixels specified is less than or "
                         "equal to zero.");
        return;
    }

    //
    // If the grid is 2d and the R coordinates include negative values then
    // print an error message and exit.
    //
    if (input->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        double extents[6] = {0., 0., 0., 0., 0., 0.};
        avtDatasetExaminer::GetSpatialExtents(input, extents);
        UnifyMinMax(extents, 6);
        if (extents[2] < 0.)
        {
            SetResultMessage("VisIt is unable to execute this query "
                             "because it has encountered an RZ mesh with "
                             "negative R values.");
            return;
        }
    }

    int t1 = visitTimer->StartTimer();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    avtXRayFilter *filt = new avtXRayFilter;
    filt->SetImageProperties(origin, theta, phi, width, height, nx, ny);
    filt->SetDivideEmisByAbsorb(divideEmisByAbsorb);
    filt->SetVariableNames(absVarName, emisVarName);
    filt->SetInput(dob);

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        input->GetOriginatingSource()->GetGeneralContract();
    filt->GetOutput()->Update(contract);

    //
    // Get the output and write out the image.
    //
    int t2 = visitTimer->StartTimer();
    if (PAR_Rank() == 0)
    {
        //
        // Get the output.
        //
        avtDataTree_p tree = filt->GetTypedOutput()->GetDataTree();

        int nLeaves;
        vtkDataSet **leaves;
        leaves = tree->GetAllLeaves(nLeaves);

        if (nLeaves <= 0)
        {
            // Free the memory from the GetAllLeaves function call.
            delete [] leaves;
            delete filt;

            EXCEPTION1(VisItException, "There must be at least one bin.");
        }

        //
        // Write out the image.
        //
        vtkDataArray *image;
        if (outputType >= 0 && outputType <=3)
        {
            for (int i = 0; i < nLeaves; i++)
            {
                image = leaves[i]->GetPointData()->GetArray("Image");
                if (image->GetDataType() == VTK_FLOAT)
                   WriteImage(i, numPixels, (float*) image->GetVoidPointer(0));
                else if (image->GetDataType() == VTK_DOUBLE)
                   WriteImage(i, numPixels, (double*) image->GetVoidPointer(0));
                else if (image->GetDataType() == VTK_INT)
                   WriteImage(i, numPixels, (int*) image->GetVoidPointer(0));
            }
        }
        else if (outputType == 4)
        {
            for (int i = 0; i < nLeaves; i++)
            {
                image = leaves[i]->GetPointData()->GetArray("Image");
                if (image->GetDataType() == VTK_FLOAT)
                   WriteFloats(i, numPixels, (float*)image->GetVoidPointer(0));
                else if (image->GetDataType() == VTK_DOUBLE)
                   WriteFloats(i, numPixels, (double*)image->GetVoidPointer(0));
                else if (image->GetDataType() == VTK_INT)
                   WriteFloats(i, numPixels, (int*)image->GetVoidPointer(0));
            }
        }

        //
        // Output the result message.
        //
        if (outputType >=0 && outputType <= 4)
        {
            std::string msg = "";
            const char *exts[5] = {"bmp", "jpeg", "png", "tif", "bof"};
            char buf[512];
    
            if (nLeaves == 1)
                SNPRINTF(buf, 512, "The x ray image query results were "
                         "written to the file output00.%s\n",
                         exts[outputType]);
            else
                SNPRINTF(buf, 512, "The x ray image query results were "
                         "written to the files output00.%s - output%02d.%s\n",
                         exts[outputType], nLeaves - 1, exts[outputType]);
            msg += buf;

            SetResultMessage(msg);
        }
        else
        {
            SetResultMessage("No x ray image query results were written "
                             "because the output type was invalid\n");
        }

        // Free the memory from the GetAllLeaves function call.
        delete [] leaves;
    }
    visitTimer->StopTimer(t2, "avtXRayImageQuery::WriteImage");

    visitTimer->StopTimer(t1, "avtXRayImageQuery::ExecutePipeline");

    delete filt;
}


// ****************************************************************************
//  Method: avtXRayImageQuery::WriteImage
//
//  Purpose:
//    Write the image in the appropriate format.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
// ****************************************************************************

template <typename T>
void
avtXRayImageQuery::WriteImage(int iImage, int nPixels, T *fbuf)
{
    //
    // Determine the range of the data excluding values less than zero.
    //
    double minVal = FLT_MAX;
    double maxVal = -FLT_MAX;
    for (int i = 0; i < nPixels; i++)
    {
        if (fbuf[i] > 0.)
        {
            minVal = fbuf[i] < minVal ?  fbuf[i] : minVal;
            maxVal = fbuf[i] > maxVal ?  fbuf[i] : maxVal;
        }
    }
    double range = maxVal - minVal;
    
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
    for (int i = 0; i < nPixels; i++)
    {
        pixel = (unsigned char)
            ((fbuf[i] < minVal) ? 0 : (254. * ((fbuf[i] - minVal) / range)));
        *ipixel = pixel;
        ipixel++;
        *ipixel = pixel;
        ipixel++;
        *ipixel = pixel;
        ipixel++;
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
//    Write the image as a block of floats.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
// ****************************************************************************
template <typename T>
void
avtXRayImageQuery::WriteFloats(int iImage, int nPixels, T *fbuf)
{
    char fileName[24];
    sprintf(fileName, "output%02d.bof", iImage);
    FILE *file = fopen(fileName, "w");
    fwrite(fbuf, sizeof(T), nPixels, file);
    fclose(file);
}

// ****************************************************************************
//  Method: avtXRayImageQuery::GetDefaultInputParams
//
//  Purpose:
//    Retrieves default values for input variables. 
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011
//
//  Modifications:
//
// ****************************************************************************

void               
avtXRayImageQuery::GetDefaultInputParams(MapNode &params)
{
    params["output_type"] = std::string("png");
    params["divide_emis_by_absorb"] = 0;
    doubleVector o;
    o.push_back(0.0);
    o.push_back(0.0);
    o.push_back(0.0);
    params["origin"] = o;

    params["theta"] = 0.0;
    params["phi"] = 0.0;

    params["width"] = 1.0;
    params["height"] = 1.0;

    intVector is;
    is.push_back(200);
    is.push_back(200);
    params["ImageSize"] = is;

    stringVector v;
    v.push_back("absorbtivity");
    v.push_back("emissivity");
    params["vars"] = v;
}
