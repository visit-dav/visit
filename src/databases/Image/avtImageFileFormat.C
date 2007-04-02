/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                            avtImageFileFormat.C                           //
// ************************************************************************* //

#include <avtImageFileFormat.h>

#include <string>
#include <vector>
#include <visitstream.h>
#include <visit-config.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#ifndef MDSERVER
#include <vtkExtractVOI.h>
#include <vtkPNMReader.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkTIFFReader.h>
#include <vtkBMPReader.h>
#include <vtkStimulateReader.h>
#endif

#include <avtDatabaseMetaData.h>
#include <avtDataSelection.h>
#include <avtGhostData.h>
#include <avtLogicalSelection.h>
#include <avtParallel.h>
#include <avtSpatialBoxSelection.h>

#include <DebugStream.h>
#include <BadIndexException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

using     std::vector;
using     std::string;


// ****************************************************************************
//  Method: avtImage constructor
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications: 
//
//     Chris Wojtan, Mon Jun 21 15:36 PDT 2004
//     Moved most of the Image loading code into ReadInImage() instead of the
//     constructor
//
//     Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//     Moved code to load variable names from PopulateDatabaseMetaData to here
//     Moved code to determine file extension from ReadInImage to here
//
//     Hank Childs, Fri Mar 18 10:11:36 PST 2005
//     Move heavy lifting to Initialize method.
//
// ****************************************************************************

avtImageFileFormat::avtImageFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    fname = filename;
    image = NULL;
    haveReadWholeImage = false;
    haveImageVolume = false;
    haveInitialized = false;
    indexOfImageAlreadyRead = -1;
    indexOfImageToRead = -1;
}

// ***************************************************************************
//  Method: avtImageFileFormat destructor
//
//  Programmer: Chris Wojtan
//  Creation:   June 6, 2004
//
//  Modifications:
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Replaced duplicate code to free things up with a call to
//    FreeUpResources
//
// **************************************************************************

avtImageFileFormat::~avtImageFileFormat()
{
    FreeUpResources();
    cellvarnames.clear();
}


// ****************************************************************************
//  Method: avtImageFileFormat::Initialize
//
//  Purpose:
//      Initializes the reader.  This is not done in the constructor, because
//      many of these objects may be instantiated at one time.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2005
//
//  Modifications:
//
//     Hank Childs, Thu Mar 17 15:33:44 PST 2005
//     If we are using the stimulate image format, don't add color channels.
//     Also add support for image volumes, where we pull out the extension.
//
// ****************************************************************************

void
avtImageFileFormat::Initialize(void)
{
    if (haveInitialized)
        return;
    haveInitialized = true;

    // find the file extension
    int i, start;
    for(i=0; i<fname.size(); i++)
        if(fname[i] == '.')
            start = i;
    fext = string(fname, start+1, fname.size()-1);

    debug4 << "File extension: " << fext.c_str() << endl;

    // If we have an image volume, do some special processing.
    if (fext == "imgvol")
    {
        ReadImageVolumeHeader();
        for(i=0; i<subImages[0].size(); i++)
            if(subImages[0][i] == '.')
                start = i;
        image_fext = string(subImages[0], start+1, subImages[0].size()-1);
    }
    else
    {
        image_fext = fext;
        subImages.push_back(fname);
    }

    // Check to see if we have color channels.
    bool hasColorChannels = true;
    int numExtsWithoutColorChannels = 4;
    char *extsWithoutColorChannels[4] = { "spr", "SPR", "sdt", "SDT" };
    for (i = 0 ; i < numExtsWithoutColorChannels ; i++)
        if (image_fext == extsWithoutColorChannels[i])
            hasColorChannels = false;

    // load variable names
    if (hasColorChannels)
    {
        cellvarnames.push_back("red");
        cellvarnames.push_back("green");
        cellvarnames.push_back("blue");
        cellvarnames.push_back("alpha");
    }
    cellvarnames.push_back("intensity");
}


// ****************************************************************************
//  Method: avtImageFileFormat::ActivateTimestep
//
//  Purpose:
//      Activates this file format, allowing some initialization work to
//      happen.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2005
//
// ****************************************************************************

void
avtImageFileFormat::ActivateTimestep(void)
{
    Initialize();
}


// ****************************************************************************
//  Method: avtImageFileFormat::ReadImageVolumeHeader
//
//  Purpose:
//      Reads in a header file that lists a series of images that should be
//      combined as a volume.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2005
//
//  Modifications:
//
//    Hank Childs, Fri May 20 10:33:16 PDT 2005
//    Add support for relative paths.
//
//    Hank Childs, Mon Jun  6 16:19:23 PDT 2005
//    Read in the header on processor 0 only and broadcast to the other
//    processors.
//
//    Brad Whitlock, Thu May 11 16:22:03 PST 2006
//    Added code to strip off extra windows end of line characters from the
//    end of the file extensions so the reader is not confused later.
//
// ****************************************************************************

void
avtImageFileFormat::ReadImageVolumeHeader(void)
{
    int rank = PAR_Rank();
    if (PAR_UIProcess())
    {
        ifstream ifile(fname.c_str());
        if (ifile.fail())
        {
            debug1 << "Cannot open image volume file \"" << fname.c_str() 
                   << "\"." << endl;
            int success = 0;
            BroadcastInt(success);
            EXCEPTION1(InvalidFilesException, fname.c_str());
        }

        specifiedZStart = 0;
        zStart = 0;
        specifiedZStep = 0;
        zStep = 1;
    
        const char   *filename = fname.c_str();
        char          dir[1024];
        const char   *p = filename, *q = NULL;
        while ((q = strstr(p, SLASH_STRING)) != NULL)
        {
            p = q+1;
        }
        strncpy(dir, filename, p-filename);
        dir[p-filename] = '\0';
    
        char line[1024];
        while (!ifile.eof())
        {
            ifile.getline(line, 1024);
            if (line[0] == '\0')
                continue;
            if (strncmp(line, "Z_START:", strlen("Z_START:")) == 0)
            {
                zStart = atof(line + strlen("Z_START:"));
                specifiedZStart = true;
            }    
            else if (strncmp(line, "Z_STEP:", strlen("Z_STEP:")) == 0)
            {
                zStep = atof(line + strlen("Z_STEP:"));
                specifiedZStep = true;
            }    
            else
            {
                char line_with_dir[1024];
                if (line[0] == SLASH_CHAR)
                {
                    strcpy(line_with_dir, line);
                }
                else
                {
                    sprintf(line_with_dir, "%s%s", dir, line);
                }

                // Trim any weird control characters off of the end.
                int len = strlen(line_with_dir);
                char *cptr = line_with_dir + len - 1;
                while(cptr >= line_with_dir)
                {
                    if(*cptr < ' ')
                        *cptr-- = '\0';
                    else
                        break;
                }

                subImages.push_back(line_with_dir);
                debug4 << "Adding \"" << line_with_dir
                       << "\" to the list of subImages." << endl;
            }
        }

        //
        // Now broadcast what we've read to other processors.
        //
        int success = 1;
        BroadcastInt(success);
        int iTmp = specifiedZStart;
        BroadcastInt(iTmp);
        double dTmp = zStart;
        BroadcastDouble(dTmp);
        iTmp = specifiedZStep;
        BroadcastInt(iTmp);
        dTmp = zStep;
        BroadcastDouble(dTmp);
        BroadcastStringVector(subImages, rank);
    }
    else
    {
        int success = 0;
        BroadcastInt(success);
        if (!success)
        {
            debug1 << "Processor 0 could not open file \"" << fname.c_str() 
                   << "\"." << endl;
            EXCEPTION1(InvalidFilesException, fname.c_str());
        }
        int iTmp;
        BroadcastInt(iTmp);
        specifiedZStart = (bool) iTmp;
        double dTmp;
        BroadcastDouble(dTmp);
        zStart = dTmp;
        BroadcastInt(iTmp);
        specifiedZStep = (bool) iTmp;
        BroadcastDouble(dTmp);
        zStep = dTmp;
        BroadcastStringVector(subImages, rank);
    }

    haveImageVolume = true;
}


// ***************************************************************************
//  Method: CanCacheVariable 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 9, 2004 
//
//  Modifications:
//
//    Hank Childs, Fri Mar 18 11:41:04 PST 2005
//    Disallow caching of image volumes.
//
//    Hank Childs, Tue Jun  7 09:04:29 PDT 2005
//    Re-allow caching of image volumes, because we aren't doing any smart
//    data selections.
//
// **************************************************************************

bool
avtImageFileFormat::CanCacheVariable(const char*)
{
    // if we've read the whole image, we can cache its variables
    return haveReadWholeImage;
}

// ***************************************************************************
//  Method: RegisterDataSelections 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 3, 2004 
//
// **************************************************************************

void
avtImageFileFormat::RegisterDataSelections(
    const vector<avtDataSelection_p> &sels,
    vector<bool> *selectionsApplied)
{
    selList     = sels;
    selsApplied = selectionsApplied;
}

// ***************************************************************************
//  Method: ProcessSelections 
//
//  Purpose: Walk through all the data selections, decide which we can
//  service here and compose all that we can service into a single, logical
//  selection returned in the argument.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 3, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Made it not process any selections for certain image file formats
//
//    Hank Childs, Thu Mar 17 15:33:44 PST 2005
//    Don't process data selections for Stimulate images.
//
// **************************************************************************

bool
avtImageFileFormat::ProcessDataSelections(int *xmin, int *xmax,
                                          int *ymin, int *ymax)
{
    bool retval = false;

    // some image file formats don't support selection on read.
    if ((image_fext == "png")||(image_fext == "PNG")||(image_fext == "tif")||
        (image_fext == "tiff")||(image_fext == "TIF")||(image_fext == "TIFF")||
        (image_fext == "jpg")||(image_fext == "jpeg")||(image_fext == "JPG")||
        (image_fext == "JPEG") || (image_fext == "spr") || (image_fext == "SPR")
        || (image_fext == "sdt") || (image_fext == "SDT") 
        || (image_fext == "imgvol"))
    {
        for (int i = 0; i < selList.size(); i++)
            (*selsApplied)[i] = false;
        return retval;
    }

    avtLogicalSelection composedSel;
    for (int i = 0; i < selList.size(); i++)
    {
        if (string(selList[i]->GetType()) == "Logical Data Selection")
        {
            avtLogicalSelection *sel = (avtLogicalSelection *) *(selList[i]);
            int strides[3];
            sel->GetStrides(strides);

            // currently handles only stride 1 
            bool stridesOk = true;
            for (int j = 0; j < 3; j++)
            {
                if (strides[j] != 1)
                {
                    stridesOk = false;
                    break;
                }
            }

            if (stridesOk == false)
            {
                (*selsApplied)[i] = false;
                continue;
            }

            // overrwrite method-scope arrays with the new indexing
            composedSel.Compose(*sel);
            (*selsApplied)[i] = true;
            retval = true;

        }
        else if (string(selList[i]->GetType()) == "Spatial Box Data Selection")
        {
            avtSpatialBoxSelection *sel = (avtSpatialBoxSelection *) *(selList[i]);

            double mins[3], maxs[3];
            sel->GetMins(mins);
            sel->GetMaxs(maxs);
            avtSpatialBoxSelection::InclusionMode imode =
                sel->GetInclusionMode();

            // we won't handle clipping of zones here
            if ((imode != avtSpatialBoxSelection::Whole) &&
                (imode != avtSpatialBoxSelection::Partial))
            {
                (*selsApplied)[i] = false;
                continue;
            }

            int imins[3], imaxs[3];
            for (int j = 0; j < 3; j++)
            {
                int imin = (int) mins[j];
                if (((double) imin < mins[j]) &&
                    (imode == avtSpatialBoxSelection::Whole))
                    imin++;
                
                int imax = (int) maxs[j];
                if (((double) imax < maxs[j]) &&
                    (imode == avtSpatialBoxSelection::Partial))
                    imax++;

                imins[j] = imin;
                imaxs[j] = imax;
            }

            avtLogicalSelection newSel;
            newSel.SetStarts(imins);
            newSel.SetStops(imaxs);

            composedSel.Compose(newSel);
            (*selsApplied)[i] = true;
            retval = true;
        }
        else
        {
            // indicate we won't handle this selection
            (*selsApplied)[i] = false;
        }
    }

    int mins[3], maxs[3];
    composedSel.GetStarts(mins);
    composedSel.GetStops(maxs);

    *xmin = mins[0]; 
    *xmax = maxs[0]; 
    *ymin = mins[1]; 
    *ymax = maxs[1]; 

    return retval;
}

// *****************************************************************************
//  Method: avtImageVileFormat::ReadInImage
//
//  Purpose:
//      Reads in the image.
//
//  Programmer: Chris Wojtan
//  Creation:   June 6, 2004
//
//  Modifications:
//
//    Chris Wojtan Jun 21, 15:38 PDT 2004
//    Moved most of the loading code into this function in order to speed up
//    the constructor
//
//    Jeremy Meredith, Wed Jul  7 13:49:18 PDT 2004
//    The MDServer doesn't need to read the images; it is making assumptions
//    about what is in the file already for speed purposes, and this lets us
//    get away with not using the extra VTK libraries.  Also commented out
//    the #include files at the top of the file.  Also, there was no case
//    for monochrome images, so I added one, and I threw an exception on an
//    unexpected number of channels.  And, made intensity be the numerical
//    average of the RGB channels instead of the sum.
//
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    Add some support for data selections. Not all formats can perform
//    selection during read. Those that can do. Those that cannot read the
//    whole image and then extract the region of interest after the read.
//    This is still useful to VisIt as it can reduce the amount of data
//    the engine keeps around
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Backed off on the above change, a bit. Now, if a format doesn't support
//    selection on read, then it will read and keep around the whole image
//    If selection on read is supported but the selection is such that it winds
//    up reading the whole image, then it will read and keep the whole image.
//    Otherwise, it will read and keep only the portion requested.
//    Also, removed all code to allocate large, float vectors and populate them
//    to the GetVar call which is when they are actually needed.
//
//    Hank Childs, Fri Mar 11 10:05:51 PST 2005
//    Fix memory leak.
//
//    Hank Childs, Thu Mar 17 15:33:44 PST 2005
//    Add support for the stimulate image format.
//
//    Kathleen Bonnell, Wed May 17 14:14:04 PDT 2006 
//    Remove call to SetSource(NULL), as it now removes information necessary
//    for the dataset. 
//
// *****************************************************************************

void avtImageFileFormat::ReadInImage(void)
{
#ifdef MDSERVER
    return;
#else

    // if we've already read the entire image, then do nothing
    if (haveReadWholeImage &&
        ((indexOfImageToRead < 0) ||
         (indexOfImageToRead == indexOfImageAlreadyRead)))
    {
        return;
    }

    if (image != NULL)
    {
        image->Delete();
        image = NULL;
    }

    // process any data selections we can handle, here
    int xmin, xmax, ymin, ymax;
    bool haveSelections = ProcessDataSelections(&xmin, &xmax, &ymin, &ymax);

    haveReadWholeImage = true;
    indexOfImageAlreadyRead = indexOfImageToRead;
    if (haveSelections)
        haveReadWholeImage = false;

    xStart = yStart = 0;
    xStep = yStep = 1;

    int idx = indexOfImageToRead;
    idx = (idx < 0 ? 0 : idx);

    debug4 << "avtImageFileFormat::ReadInImage: image_fext=" << image_fext << endl;

    // select the appropriate reader for the file extension
    if ((image_fext == "pnm") || (image_fext == "PNM") || 
        (image_fext == "ppm") || (image_fext == "PPM"))
    {
        vtkPNMReader *reader = vtkPNMReader::New();
        if (haveSelections)
            reader->SetDataVOI(xmin,xmax,ymin,ymax,0,0);
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->Delete();
    }
    else if ((image_fext == "png") || (image_fext == "PNG"))
    {
        vtkPNGReader *reader = vtkPNGReader::New();
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->Delete();
    }
    else if ((image_fext == "jpg") || (image_fext == "jpeg") || 
             (image_fext == "JPG") || (image_fext == "JPEG"))
    {
        vtkJPEGReader *reader = vtkJPEGReader::New();
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->Delete();
    }
    else if ((image_fext == "tif") || (image_fext == "tiff") || 
             (image_fext == "TIF") || (image_fext == "TIFF"))
    {
        vtkTIFFReader *reader = vtkTIFFReader::New();
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->Delete();
    }
    else if ((image_fext == "bmp") || (image_fext == "BMP"))
    {
        vtkBMPReader *reader = vtkBMPReader::New();
        if (haveSelections)
            reader->SetDataVOI(xmin,xmax,ymin,ymax,0,0);
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->Delete();
    }
    else if ((image_fext == "spr") || (image_fext == "SPR") || 
             (image_fext == "sdt") || (image_fext == "SDT"))
    {
        vtkStimulateReader *reader = vtkStimulateReader::New();
        reader->SetFileName(subImages[idx].c_str());
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        //image->SetSource(NULL);
        reader->GetOrigin(xStart, yStart);
        reader->GetStep(xStep, yStep);
        reader->Delete();
    }
    else
        EXCEPTION1(InvalidFilesException, fname.c_str());

    // see if the selection is such that we read the whole image anyways
    if (haveSelections && (xmin == 0) && (ymin == 0))
    {
        int extents[6];
        image->GetExtent(extents);
        if ((xmax > extents[1]) && (ymax > extents[3]))
            haveReadWholeImage = true;
    }

#endif
}


// ****************************************************************************
//  Method: avtImageFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Removed unnused pointvarnames and pointvars
//
// ****************************************************************************

void
avtImageFileFormat::FreeUpResources(void)
{
    cellvars.clear();

    if (image != NULL)
    {
        image->Delete();
        image = NULL;
    }

    haveReadWholeImage = false;
}


// ****************************************************************************
//  Method: avtImageFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//  
//     Chris Wojtan Mon Jun 21 15:39 PDT 2004
//     Moved variable name loading from constructor to this function,
//     Changed the 3D vector to 4D vector
//
//     Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//     Moved code to populate variable names to constructor
//
//     Mark C. Miller, Wed Dec 15 10:26:07 PST 2004
//     Added support for node-centered representation of image
//
//     Hank Childs, Fri Mar 18 10:22:20 PST 2005
//     Make sure to call initialize.  Also add support for image volumes.
//
// ****************************************************************************

void
avtImageFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    Initialize();

    int dim = (fext == "imgvol" ? 3 : 2);
    AddMeshToMetaData(md, "ImageMesh", AVT_RECTILINEAR_MESH, NULL, 1, 0, 
                      dim, dim);
    AddMeshToMetaData(md, "ImageMesh_nodal", AVT_RECTILINEAR_MESH, NULL, 
                      1, 0, dim, dim);

    // add these variable names to the metadata
    int i;
    for(i=0; i<cellvarnames.size(); i++)
    {
        AddScalarVarToMetaData(md, cellvarnames[i], "ImageMesh", AVT_ZONECENT);
        AddScalarVarToMetaData(md, cellvarnames[i] + "_nodal", 
                               "ImageMesh_nodal", AVT_NODECENT);
    }

    // Check to see if we support colors.
    bool supportColors = true;
    int numExtsThatDontSupportColors = 5;
    char *extsThatDontSupportColors[5] = { "spr", "SPR", "sdt", "SDT",
                                          "imgvol" };
    for (i = 0 ; i < numExtsThatDontSupportColors ; i++)
        if (image_fext == extsThatDontSupportColors[i])
            supportColors = false;

    // we do not create a list of vector variables,
    // because there is only one vector variable, "color", 
    // which is created from these scalar variables
    if (supportColors)
    {
        AddVectorVarToMetaData(md, "color", "ImageMesh", AVT_ZONECENT, 4);
        AddVectorVarToMetaData(md, "color_nodal", "ImageMesh_nodal",
                               AVT_NODECENT, 4);
    }

    if (fext == "imgvol")
    {
        md->SetFormatCanDoDomainDecomposition(true);
    }
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2005
//
// ****************************************************************************

vtkDataSet *
avtImageFileFormat::GetMesh(const char *meshname)
{
    if (haveImageVolume)
    {
        return GetImageVolumeMesh(meshname);
    }
    else
    {
        return GetOneMesh(meshname);
    }
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetImageVolumeMesh
//
//  Purpose:
//      Gets a series of meshes.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2005
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 11:45:23 PDT 2005
//    Added ghost nodes.
//
// ****************************************************************************

vtkDataSet *
avtImageFileFormat::GetImageVolumeMesh(const char *meshname)
{
    int i;
    bool doGhostNodes = false;

    int addOne = 1;
    if (strcmp(meshname, "ImageMesh_nodal") == 0)
        addOne = 0;

    float myStart = (specifiedZStart ? zStart : 0.);
    int   mySteps = subImages.size();
    int   startImg = 0;
    float myZStep = (specifiedZStep ? zStep : 1.);

#ifdef PARALLEL
    int rank = PAR_Rank();
    int nprocs = PAR_Size();
    int stepsPerProc = mySteps / nprocs;
    if ((mySteps % nprocs) != 0)
        stepsPerProc += 1;
    startImg = rank*stepsPerProc;
    if (startImg >= mySteps)
        startImg = mySteps;
    int endImg = (rank+1)*stepsPerProc;
    if (endImg >= mySteps)
        endImg = mySteps;

    if (!addOne)
    {
        //
        // We want to make sure this mesh matches up with that of the previous
        // processor.  If we don't do this, we will get a gap.
        //
        if (startImg > 0)
            startImg--;
    }
    myStart = myStart + startImg*myZStep;
    mySteps = endImg - startImg;
    doGhostNodes = true;
#endif
   
    // -1 means we will take any image.
    indexOfImageToRead = -1;
    vtkDataSet *one_slice = GetOneMesh(meshname);

    //
    // This shouldn't happen, but it is always good to check assumptions.
    //
    if (one_slice == NULL)
    {
        debug1 << "Return value from avtImageFileFormat::GetOneMesh was "
               << "NULL, returning early" << endl;
        return NULL;
    }
    if (one_slice->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        debug1 << "Return value from avtImageFileFormat::GetOneMesh was "
               << "not rectilinear, returning early" << endl;
        return NULL;
    }

    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(mySteps+addOne);
    for (i = 0 ; i < mySteps+addOne ; i++)
        z->SetTuple1(i, myStart + (i-addOne/2.)*myZStep);

    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) one_slice;
    int dims[3];
    rgrid->GetDimensions(dims);
    dims[2] = mySteps+addOne;
    rgrid->SetDimensions(dims);
    rgrid->SetZCoordinates(z);
    z->Delete();

    if (doGhostNodes)
    {
        int nvals = dims[0]*dims[1]*dims[2];
        vtkUnsignedCharArray *ghost_nodes = vtkUnsignedCharArray::New();
        ghost_nodes->SetName("avtGhostNodes");
        ghost_nodes->SetNumberOfTuples(nvals);
        unsigned char *gnp = ghost_nodes->GetPointer(0);
        for (i = 0 ; i < nvals ; i++)
            gnp[i] = 0;
        if (startImg != 0)
        {
            int nplane = dims[0]*dims[1];
            for (i = 0 ; i < nplane ; i++)
                avtGhostData::AddGhostNodeType(gnp[i], DUPLICATED_NODE);
        }
        if ((startImg+mySteps) != subImages.size())
        {
            int nplane = dims[0]*dims[1];
            int planeStart = nvals-nplane;
            for (i = planeStart ; i < nvals ; i++)
                avtGhostData::AddGhostNodeType(gnp[i], DUPLICATED_NODE);
        }
        rgrid->GetPointData()->AddArray(ghost_nodes);
        ghost_nodes->Delete();
    }

    return rgrid;
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetOneMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    I made it get xdim/ydim (and mins/maxs) from vtkImageData object
//
//    Mark C. Miller, Wed Dec 15 10:26:07 PST 2004
//    Added support for node-centered representation of image
//
//    Hank Childs, Fri Mar 18 11:41:04 PST 2005
//    Renamed to GetOneMesh.  Also added support for setting up images that
//    do not start at (0,0) with steps of size 1.
//
// ****************************************************************************

vtkDataSet *
avtImageFileFormat::GetOneMesh(const char *meshname)
{
    ReadInImage();

    int addOne = 1;
    if (strcmp(meshname, "ImageMesh_nodal") == 0)
        addOne = 0;
    
    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    // Set up rectilinear grid representing the image...
    // Since the colors are cell variables,
    //    we create an extra row of nodes in each dimesion
    //    so we can have the correct number of cells.
    int i;
    vtkFloatArray *xCoords = vtkFloatArray::New();
    for(i=0; i<xdim + addOne; i++)
        xCoords->InsertNextValue((float) xStart + i*xStep-addOne/2.0);
    vtkFloatArray *yCoords = vtkFloatArray::New();
    for(i=0; i<ydim + addOne; i++)
        yCoords->InsertNextValue((float) yStart + i*yStep -addOne/2.0);
    vtkFloatArray *zCoords = vtkFloatArray::New();
    zCoords->InsertNextValue(0.0);
    
    vtkRectilinearGrid *dataset = vtkRectilinearGrid::New();
    dataset->SetDimensions(xdim+addOne,ydim+addOne,1);
    dataset->SetXCoordinates(xCoords);
    dataset->SetYCoordinates(yCoords);
    dataset->SetZCoordinates(zCoords);

    xCoords->Delete();
    yCoords->Delete();
    zCoords->Delete();

    return dataset;
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetVar
//
//  Purpose:
//      Gets the var associated with this file.  
//
//  Arguments:
//      varname    The name of the var of interest.  This can be ignored if
//                 there is only one var.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2005
//
// ****************************************************************************

vtkDataArray *
avtImageFileFormat::GetVar(const char *varname)
{
    if (haveImageVolume)
    {
        return GetImageVolumeVar(varname);
    }
    else
    {
        return GetOneVar(varname);
    }
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetImageVolumeVar
//
//  Purpose:
//      Gets the variable for the image volume.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2005
//
// ****************************************************************************

vtkDataArray *
avtImageFileFormat::GetImageVolumeVar(const char *varname)
{
    int myStart = 0;
    int myStop = subImages.size();

    const char *vtmp = varname;
    int len = strlen(varname);
    int len2 = strlen("_nodal");
    if (len > len2)
        vtmp = varname + len - len2;
    bool isNodal = (strcmp(vtmp, "_nodal") == 0);

#ifdef PARALLEL
    int rank = PAR_Rank();
    int nprocs = PAR_Size();
    int stepsPerProc = myStop / nprocs;
    if ((myStop % nprocs) != 0)
        stepsPerProc += 1;
    int startImg = rank*stepsPerProc;
    if (startImg >= myStop)
        startImg = myStop;
    int endImg = (rank+1)*stepsPerProc;
    if (endImg >= myStop)
        endImg = myStop;

    if (isNodal)
    {
        //
        // We want to make sure this mesh matches up with that of the previous
        // processor.  If we don't do this, we will get a gap.
        //
        if (startImg > 0)
            startImg--;
    }

    myStart = startImg;
    myStop = endImg;
#endif
   
    vtkFloatArray *arr = vtkFloatArray::New();
    bool haveInitialized = false;
    int  valsPerSlice = 0;
    for (int i = myStart ; i < myStop ; i++)
    {
        indexOfImageToRead = i;
        vtkDataArray *one_slice = GetOneVar(varname);

        //
        // This shouldn't happen, but it is always good to check assumptions.
        //
        if (one_slice == NULL)
        {
            debug1 << "Return value from avtImageFileFormat::GetOneVar was "
                   << "NULL, returning early" << endl;
            return NULL;
        }
        if (one_slice->GetDataType() != VTK_FLOAT)
        {
            debug1 << "Return value from avtImageFileFormat::GetOneVar was "
                   << "not floating point, returning early" << endl;
            return NULL;
        }
        if (one_slice->GetNumberOfComponents() != 1)
        {
            debug1 << "Return value from avtImageFileFormat::GetOneVar had "
                   << "more than 1 component.  Not supported." << endl;
            return NULL;
        }

        if (!haveInitialized)
        {
            valsPerSlice = one_slice->GetNumberOfTuples();
            int ntups = valsPerSlice*(myStop - myStart);
            arr->SetNumberOfTuples(ntups);
            haveInitialized = true;
        }
       
        float *p1 = (float *) one_slice->GetVoidPointer(0);
        float *p2 = (float *) arr->GetVoidPointer(0);
        p2 += valsPerSlice*(i-myStart);
        memcpy(p2, p1, valsPerSlice*sizeof(float));
        one_slice->Delete();
    }

    return arr;
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetOneVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    I made it get xdim/ydim from vtkImageData object
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Added code to return float data directly from vtkImageData object
//    instead of through intermediary float vectors
//
//    Mark C. Miller, Wed Dec 15 10:26:07 PST 2004
//    Added support for node-centered representation of image
//
//    Hank Childs, Wed Mar 23 15:30:41 PST 2005
//    Renamed to GetOneVar from GetVar.
//
//    Hank Childs, Fri May 20 11:03:10 PDT 2005
//    Fixed flipping of green and blue channels.  Also added fast track for
//    float and unsigned char data.
//
// ****************************************************************************

vtkDataArray *
avtImageFileFormat::GetOneVar(const char *varname)
{
    ReadInImage();

    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    int extents[6];
    image->GetExtent(extents);
    int xmin = extents[0];
    int xmax = extents[1];
    int ymin = extents[2];
    int ymax = extents[3];

    int channel = -2;
    if (strncmp(varname, "red", 3) == 0)
        channel = 0;
    else if (strncmp(varname, "green", 5) == 0)
        channel = 1;
    else if (strncmp(varname, "blue", 4) == 0)
        channel = 2;
    else if (strncmp(varname, "alpha", 5) == 0)
        channel = 3;
    else if (strncmp(varname, "intensity", 9) == 0)
        channel = -1;

    if (channel == -2)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    if (channel > image->GetNumberOfScalarComponents())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples((xdim)*(ydim));
    float *ptr = (float *)scalars->GetVoidPointer(0);

    int i, j;
    int nChannels = image->GetNumberOfScalarComponents();
    if (channel < 0 && nChannels < 3)
        channel = 0;  // Treat the 0th channel as intensity in this case.
    if (nChannels == 1)
        channel = 0;
    if (channel >= 0)
    {
        if (image->GetScalarType() == VTK_FLOAT)
        {
            float *data = (float *) image->GetScalarPointer();
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                {
                    int index = j*xdim + i;
                    ptr[index] = data[nChannels*index + channel];
                }
            }
        }
        else if (image->GetScalarType() == VTK_UNSIGNED_CHAR)
        {
            unsigned char *data = (unsigned char *) image->GetScalarPointer();
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                {
                    int index = j*xdim + i;
                    ptr[index] = (float) (data[nChannels*index + channel]);
                }
            }
        }
        else
        {
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                    ptr[j*xdim + i] = 
                     image->GetScalarComponentAsDouble(i+xmin,j+ymin,0,channel);
            }
        }
    }
    else
    {
        if (image->GetScalarType() == VTK_FLOAT)
        {
            float *data = (float *) image->GetScalarPointer();
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                {
                    int index = j*xdim + i;
                    float r = data[nChannels*index + 0];
                    float g = data[nChannels*index + 1];
                    float b = data[nChannels*index + 2];
                    ptr[j*xdim + i] = (r + g + b) / 3.0;
                }
            }
        }
        if (image->GetScalarType() == VTK_UNSIGNED_CHAR)
        {
            unsigned char *data = (unsigned char *) image->GetScalarPointer();
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                {
                    int index = j*xdim + i;
                    unsigned char r = data[nChannels*index + 0];
                    unsigned char g = data[nChannels*index + 1];
                    unsigned char b = data[nChannels*index + 2];
                    ptr[j*xdim + i] = (r + g + b) / 3.0;
                }
            }
        }
        else
        {
            for (j = 0; j < ydim; j++)
            {
                for (i = 0; i < xdim; i++)
                {
                    float r, g, b;
                    r = image->GetScalarComponentAsDouble(i+xmin,j+ymin,0,0);
                    g = image->GetScalarComponentAsDouble(i+xmin,j+ymin,0,1);
                    b = image->GetScalarComponentAsDouble(i+xmin,j+ymin,0,2);
                    ptr[j*xdim + i] = (r + g + b) / 3.0;
                }
            }
        }
    }

    return scalars;

}


// ****************************************************************************
//  Method: avtImageFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//      Chris Wojtan Mon Jun 21 15:41 PDT 2004
//      Changed the vector dimension from 3D to 4D
//
//      Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//      I made it get xdim/ydim from vtkImageData object
//
//      Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//      Added code to return float data directly from vtkImageData object
//      instead of through intermediary float vectors
// ****************************************************************************


vtkDataArray *
avtImageFileFormat::GetVectorVar(const char *varname)
{
    ReadInImage();

    int imgcomps = image->GetNumberOfScalarComponents();
    if (imgcomps < 3)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    int extents[6];
    image->GetExtent(extents);
    int xmin = extents[0];
    int xmax = extents[1];
    int ymin = extents[2];
    int ymax = extents[3];

    int ncomps = 4;
    int ntuples = xdim*ydim;   // this is the number of entries in the variable.
    vtkFloatArray *rv = vtkFloatArray::New();

    int ucomps = 4;

    rv->SetNumberOfComponents(ucomps);
    rv->SetNumberOfTuples(ntuples);
    float *one_entry = new float[ucomps];
    for (int i = 0 ; i < ntuples ; i++)
    {
        int j;
        for (j = 0 ; j < imgcomps; j++)
        {
            int ii = i % xdim;
            int jj = i / xdim;
            one_entry[j] = image->GetScalarComponentAsDouble(ii+xmin,jj+ymin,0,j);
        }
        for (j = imgcomps ; j < ucomps; j++)
            one_entry[j] = (j == 3 ? 255.0 : 0.0);
        rv->SetTuple(i, one_entry); 
    }
    
    delete [] one_entry;
    return rv;
}
