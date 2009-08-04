/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                           avtImagePluginWriter.C                          //
// ************************************************************************* //

#include <avtImagePluginWriter.h>

#include <snprintf.h>

#include <vtkBMPWriter.h>
#include <vtkDataArray.h>
#include <vtkCellData.h>
#include <vtkImageData.h>
#include <vtkImageWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPNMWriter.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkTIFFWriter.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <ImproperUseException.h>

#include <string>
#include <vector>

using     std::string;
using     std::vector;

// ****************************************************************************
//  Method: avtImagePluginWriter constructor
//
//  Purpose: Handle database options 
//
//  Programmer: Mark C. Miller
//  Creation:   November 28, 2007 
//
// ****************************************************************************
avtImagePluginWriter::avtImagePluginWriter(DBOptionsAttributes *dbOpts)
{
    format = dbOpts->GetEnum("Format");
    normalize = dbOpts->GetBool("Normalize [0,255]");
    compression = dbOpts->GetEnum("TIFF Compression");
    quality = dbOpts->GetInt("JPEG Quality [0,100]");
}

// ****************************************************************************
//  Method: avtImagePluginWriter::OpenFile
//
//  Purpose: Record file name and number of blocks.
//
//  Programmer: Mark C. Miller
//  Creation:   November 28, 2007 
//
// ****************************************************************************

void
avtImagePluginWriter::OpenFile(const string &fname, int nb)
{
    fileName = fname;
    nblocks = nb;
}

// ****************************************************************************
//  Method: avtImagePluginWriter::WriteHeaders
//
//  Purpose: Writes out the ImagePlugin header file.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

void
avtImagePluginWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           vector<string> &scalars, vector<string> &vectors,
                           vector<string> &materials)
{
    //
    // We can only handle single block files.
    //
    if (nblocks != 1)
    {
        EXCEPTION1(InvalidDBTypeException, 
                         "The ImagePlugin writer can only handle single block files.");
    }

    // 
    // Don't bother writing the header now.  We only support single block
    // datasets and we need to examine that dataset before we can write out
    // the header.  So just punt on writing the header and do it when we get
    // the single block.
    //
}

// ****************************************************************************
//  Function ConvertToUnsignedChar
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

template <class iT>
static void ConvertToUnsignedChar(unsigned char *obuf, const iT *ibuf, int n, bool normalize)
{
    int i;

    if (normalize)
    {
        double min = ibuf[0];
        double max = min;
        for (i = 1; i < n; i++)
        {
            if (ibuf[i] < min)
                min = ibuf[i];
            else if (ibuf[i] > max)
                max = ibuf[i];
        }
        double range = (double) max - (double) min;
	if (range == 0.0) range = 1.0;
        for (i = 0; i < n; i++)
        {
            double t = ((double) ibuf[i] - (double) min) / range;
            obuf[i] = (unsigned char) (t * 254.99);
        }
    }
    else
    {
        for (i = 0; i < n; i++)
            obuf[i] = (unsigned char) ibuf[i];
    }
}

// ****************************************************************************
//  Function ConvertDataArrayToUnsignedChar 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

static vtkUnsignedCharArray *
ConvertDataArrayToUnsignedChar(vtkDataArray *oldArr, bool normalize)
{
    vtkUnsignedCharArray *newArr = vtkUnsignedCharArray::New();

    int numTuples = oldArr->GetNumberOfTuples();
    int numComponents = oldArr->GetNumberOfComponents();

    newArr->SetNumberOfComponents(numComponents);
    newArr->SetNumberOfTuples(numTuples);

    unsigned char *newBuf = (unsigned char*) newArr->GetVoidPointer(0);
    void *oldBuf = oldArr->GetVoidPointer(0);

    int numValues = numTuples * numComponents;
    switch (oldArr->GetDataType())
    {
        case VTK_SHORT:
            ConvertToUnsignedChar(newBuf, (short*) oldBuf, numValues, normalize);
            break;
        case VTK_UNSIGNED_SHORT:
            ConvertToUnsignedChar(newBuf, (unsigned short*) oldBuf, numValues, normalize);
            break;
        case VTK_INT:
            ConvertToUnsignedChar(newBuf, (int*) oldBuf, numValues, normalize);
            break;
        case VTK_UNSIGNED_INT:
            ConvertToUnsignedChar(newBuf, (unsigned int*) oldBuf, numValues, normalize);
            break;
        case VTK_LONG:
            ConvertToUnsignedChar(newBuf, (long*) oldBuf, numValues, normalize);
            break;
        case VTK_UNSIGNED_LONG:
            ConvertToUnsignedChar(newBuf, (unsigned long*) oldBuf, numValues, normalize);
            break;
        case VTK_FLOAT:
            ConvertToUnsignedChar(newBuf, (float*) oldBuf, numValues, normalize);
            break;
        case VTK_DOUBLE:
            ConvertToUnsignedChar(newBuf, (double*) oldBuf, numValues, normalize);
            break;
        case VTK_ID_TYPE:
            ConvertToUnsignedChar(newBuf, (vtkIdType*) oldBuf, numValues, normalize);
            break;
        default:
            {   char msg[256];
                SNPRINTF(msg, sizeof(msg), "Cannot convert array to unsigned char");
                EXCEPTION1(ImproperUseException, msg); 
            }
    }

    return newArr;
}

// ****************************************************************************
//  Function CreateImageData 
//
//  Purpose: Create VTK Image data object from a rectilinear grid's data array
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

static vtkImageData *
CreateImageData(int *dims, const double *spacing, vtkDataArray *da,
    int isCellData, bool normalize)
{
    bool is3D = dims[2] > 1;

    vtkImageData *image = vtkImageData::New();
    image->SetDimensions(dims);
    if (spacing[0] == -1 || spacing[1] == -1 || (is3D && spacing[2] == -1))
        image->SetSpacing(1.0,1.0,is3D?1.0:0.0);
    else
        image->SetSpacing(spacing[0],spacing[1],is3D?spacing[2]:0.0);
    image->SetNumberOfScalarComponents(da->GetNumberOfComponents());
    image->SetScalarTypeToUnsignedChar();
    int n = isCellData ? 2 : 1;
    image->SetExtent(0,dims[0]-n,0,dims[1]-n,0,is3D?dims[2]-n:0);

    if (da->GetDataType() != VTK_UNSIGNED_CHAR && 
        da->GetDataType() != VTK_CHAR)
    {
        vtkUnsignedCharArray *uca = ConvertDataArrayToUnsignedChar(da, normalize);
        image->GetPointData()->SetScalars(uca);
	uca->Delete();
    }
    else
    {
        image->GetPointData()->SetScalars(da);
    }

    image->Update();

    return image;

}

// ****************************************************************************
//  Function WriteImage
//
//  Purpose: Write image file(s) for 2 or 3D data in variety of formats
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

static void
WriteImage(int format, int compression, int quality,
    const int *dims, vtkImageData *image, string fileName,
    const char *arrName, int arrNum, int numArrs)
{
    bool is3D = dims[2] > 1;

    vtkImageWriter *writer;
    string ext;
    switch (format)
    {
        case 0:
	{
            vtkTIFFWriter *tiffWriter = vtkTIFFWriter::New();
	    switch (compression)
	    {
	        case 0: tiffWriter->SetCompressionToNoCompression(); break;
	        case 1: tiffWriter->SetCompressionToPackBits(); break;
	        case 2: tiffWriter->SetCompressionToDeflate(); break;
	    }
	    writer = tiffWriter;
	    ext = ".tif";
	    break;
	}
        case 1:
	{
            writer = vtkPNGWriter::New();
	    ext = ".png";
	    break;
        }
	case 2:
	{
            vtkJPEGWriter *jpegWriter = vtkJPEGWriter::New();
	    jpegWriter->SetQuality(quality);
	    jpegWriter->ProgressiveOff();
	    writer = jpegWriter;
	    ext = ".jpeg";
	    break;
	}
	case 3:
	{
            writer = vtkBMPWriter::New();
	    ext = ".bmp";
	    break;
	}
	case 4:
	{
            writer = vtkPNMWriter::New();
	    ext = ".pnm";
	    break;
	}
    }


    if (is3D)
    {
        writer->SetFileDimensionality(3);

	char prefix[256];
	if (arrName)
	{
            SNPRINTF(prefix, sizeof(prefix), "%s_%s", fileName.c_str(), arrName);
	}
	else
	{
	    if (numArrs > 1)
	        SNPRINTF(prefix, sizeof(prefix), "%s_%02d",
	            fileName.c_str(), arrNum);
	    else
	        SNPRINTF(prefix, sizeof(prefix), "%s", fileName.c_str());
	}
	writer->SetFilePrefix(prefix);

	char pattern[256];
	SNPRINTF(pattern, sizeof(pattern), "%%s_%%03d%s", ext.c_str());
	writer->SetFilePattern(pattern);
    }
    else
    {
	char filename[256];
	if (arrName)
	{
            SNPRINTF(filename, sizeof(filename), "%s_%s%s",
                fileName.c_str(), arrName, ext.c_str());
	}
	else
	{
	    if (numArrs > 1)
	        SNPRINTF(filename, sizeof(filename), "%s_%02d%s",
	            fileName.c_str(), arrNum, ext.c_str());
	    else
	        SNPRINTF(filename, sizeof(filename), "%s%s",
	            fileName.c_str(), ext.c_str());
	}
        writer->SetFileName(filename);
    }

    writer->SetInput(image);
    writer->Write();
    writer->Delete();
}

// ****************************************************************************
//  Method: avtImagePluginWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

void
avtImagePluginWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int i;

    //
    // Check to make sure we've got a rectilinear grid
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(InvalidDBTypeException, 
                         "The ImagePlugin writer can only handle rectilinear grids.");
    }
    vtkRectilinearGrid *grid = vtkRectilinearGrid::SafeDownCast(ds);

    //
    // Check spacing in all three axes; a -1 result indicates non-uniform
    //
    double spacing[3];
    spacing[0] = grid->GetXCoordinates()->GetComponent(0,0);
    spacing[1] = grid->GetYCoordinates()->GetComponent(0,0);
    spacing[2] = grid->GetZCoordinates()->GetComponent(0,0);
    for (i = 0; i < 3; i++)
    {
        vtkDataArray *da;
	switch (i)
	{
	    case 0: da = grid->GetXCoordinates(); break;
	    case 1: da = grid->GetYCoordinates(); break;
	    case 2: da = grid->GetZCoordinates(); break;
	}
	for (int j = 0; j < da->GetNumberOfTuples(); j++)
	{
	    if (da->GetComponent(j,0) != spacing[i])
	    {
	        spacing[i] = -1.0;
		break;
	    }
	}
    }
    if (spacing[0] == -1 || spacing[1] == -1 || spacing[2] == -1 ||
        spacing[0] != spacing[1] || spacing[0] != spacing[2] ||
	spacing[1] != spacing[2])
    {
	char msg[256];
	SNPRINTF(msg, sizeof(msg),
	    "The dataset has non-uniform coordinate spacing.\n"
	    "The ImagePlugin writer may convert it to uniform.\n");
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }

    int *gridDims = grid->GetDimensions();

    //
    // Handle point data first
    //
    vtkPointData *pd = grid->GetPointData();
    int nArrs = pd->GetNumberOfArrays();
    for (i = 0; i < nArrs; i++)
    {
        vtkDataArray *da = pd->GetArray(i);
        int ncomps = da->GetNumberOfComponents();
        if (ncomps == 2 || ncomps > 4)
        {
	    char msg[256];
	    SNPRINTF(msg, sizeof(msg),
	        "An array on the dataset has an unusual number of\n"
                "components, %d. It is being skipped.\n", ncomps);
            if (!avtCallback::IssueWarning(msg))
                cerr << msg << endl;
            continue;
        }
	vtkImageData *id = CreateImageData(gridDims, spacing, da, 0, normalize);
	WriteImage(format, compression, quality, gridDims, id, fileName,
	    da->GetName(), i, nArrs);
	id->Delete();
    }

    vtkCellData *cd = grid->GetCellData();
    nArrs = cd->GetNumberOfArrays();
    for (i = 0; i < nArrs; i++)
    {
        vtkDataArray *da = cd->GetArray(i);
        int ncomps = da->GetNumberOfComponents();
        if (ncomps == 2 || ncomps > 4)
        {
	    char msg[256];
	    SNPRINTF(msg, sizeof(msg),
	        "An array on the dataset has an unusual number of\n"
                "components, %d. It is being skipped.\n", ncomps);
            if (!avtCallback::IssueWarning(msg))
                cerr << msg << endl;
            continue;
        }
	vtkImageData *id = CreateImageData(gridDims, spacing, da, 1, normalize);
	WriteImage(format, compression, quality, gridDims, id, fileName,
	    da->GetName(), i, nArrs);
	id->Delete();
    }
}


// ****************************************************************************
//  Method: avtImagePluginWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 28, 2007 
//
// ****************************************************************************

void
avtImagePluginWriter::CloseFile(void)
{
    // Just needed to meet interface.
}
