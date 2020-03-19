// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtOpenEXRFileFormat.C                         //
// ************************************************************************* //

#include <avtOpenEXRFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>

#ifdef _WIN32
// We built OpenEXR as a DLL on Windows.
#define OPENEXR_DLL
#endif

#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>

// ****************************************************************************
// Class: avtOpenEXRFileFormat::Internal
//
// Purpose:
//   Reads OpenEXR files.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 27 18:00:24 PDT 2017
//
// Modifications:
//
// ****************************************************************************

class avtOpenEXRFileFormat::Internal
{
public:
    Internal() : file(NULL), filename()
    {
    }

    ~Internal()
    {
        FreeResources();
    }

    void FreeResources()
    {
        if(file != NULL)
        {
            delete file;
            file = NULL;
        }
    }

    void SetFileName(const std::string &fn)
    {
        if(fn != filename && file != NULL)
        {
            delete file;
            file = NULL;
        }
        filename = fn;
    }

    Imf::InputFile *GetFile()
    {
        if(file == NULL)
        {
            try
            {
                file = new Imf::InputFile(filename.c_str());
            }
            catch(...)
            {
                file = NULL;
            }
        }
        return file;
    }

    void GetSize(int &width, int &height)
    {
        // Read the channel names from the file. Get the image size.
        if(GetFile() != NULL)
        {
            Imath::Box2i dw = GetFile()->header().dataWindow();
            width  = dw.max.x - dw.min.x + 1;
            height = dw.max.y - dw.min.y + 1;
        }
        else
        {
            width = height = 0;
        }
    }

    bool GetChannelNames(std::vector<std::string> &names)
    {
        names.clear();
        if(GetFile() != NULL)
        {
            const Imf::ChannelList &channels = GetFile()->header().channels();
            Imf::ChannelList::ConstIterator it = channels.begin();
            for( ; it != channels.end(); ++it)
            {
                names.push_back(it.name());
            }
        }
        return !names.empty();
    }

    vtkDataArray *ReadChannel(const std::string &name)
    {
        vtkDataArray *retval = NULL;

        if(GetFile() != NULL)
        {
            int width, height;
            GetSize(width, height);
            debug5 << "OpenEXR: width=" << width << ", height=" << height << endl;

            const Imf::ChannelList &channels = GetFile()->header().channels();
            const Imf::Channel *channel = channels.findChannel(name);
            if(channel != NULL)
            {
                Imf::FrameBuffer fb;

                if(channel->type == Imf::HALF)
                {
                    debug5 << "Reading half data for " << name << endl;
                    half *data = new half[width*height];
                    fb.insert(name,
                        Imf::Slice(Imf::HALF,
                              (char *)data,
                              sizeof(half),
                              sizeof(half) * width,
                              1, 1, 
                              0.));
                    GetFile()->setFrameBuffer(fb);
                    Imath::Box2i dw = GetFile()->header().dataWindow();
                    GetFile()->readPixels(dw.min.y, dw.max.y);

                    // Convert to float.
                    vtkFloatArray *f = vtkFloatArray::New();
                    f->SetNumberOfTuples(width*height);
                    f->SetName(name.c_str());
                    float *dest = (float *)f->GetVoidPointer(0);
                    // Flip
                    for(int j = 0; j < height; ++j)
                    {
                        half *src = data + (height-j-1)*width; // flip
                        for(int i = 0; i < width; ++i)
                            *dest++ = float(*src++);
                    }

                    delete [] data;
                    retval = f;
                }
                else if(channel->type == Imf::FLOAT)
                {
                    debug5 << "Reading float data for " << name << endl;
                    float *data = new float[width*height];
                    fb.insert(name,
                        Imf::Slice(Imf::FLOAT,
                              (char *)data,
                              sizeof(float),
                              sizeof(float) * width,
                              1, 1, 
                              0.));
                    GetFile()->setFrameBuffer(fb);
                    Imath::Box2i dw = GetFile()->header().dataWindow();
                    GetFile()->readPixels(dw.min.y, dw.max.y);

                    // Flip
                    vtkFloatArray *f = vtkFloatArray::New();
                    f->SetNumberOfTuples(width*height);
                    f->SetName(name.c_str());
                    float *dest = (float *)f->GetVoidPointer(0);
                    for(int j = 0; j < height; ++j)
                    {
                        float *src = data + (height-j-1)*width; // flip
                        memcpy(dest, src, sizeof(float)*width);
                        dest += width;
                    }

                    delete [] data;
                    retval = f;
                }
                else
                {
                    debug5 << "Unsupported data type." << endl;
                }
            }
        }
        return retval;
    }

private:
    Imf::InputFile *file;
    std::string     filename;
};

// ****************************************************************************
//  Method: avtOpenEXRFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

avtOpenEXRFileFormat::avtOpenEXRFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    d = new Internal;
    d->SetFileName(filename);
}

avtOpenEXRFileFormat::~avtOpenEXRFileFormat()
{
    delete d;
}

// ****************************************************************************
//  Method: avtOpenEXRFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

void
avtOpenEXRFileFormat::FreeUpResources(void)
{
    d->FreeResources();
}


// ****************************************************************************
//  Method: avtOpenEXRFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

void
avtOpenEXRFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    std::string meshname = "mesh";
    avtMeshType mt = AVT_RECTILINEAR_MESH;
    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = 2;
    int topological_dimension = 2;
    double *extents = NULL;
    AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                      spatial_dimension, topological_dimension);

    std::vector<std::string> names;
    d->GetChannelNames(names);
    for(size_t i = 0; i < names.size(); ++i)
        AddScalarVarToMetaData(md, names[i], "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtOpenEXRFileFormat::GetMesh
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
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

vtkDataSet *
avtOpenEXRFileFormat::GetMesh(const char *meshname)
{
    int ndims = 2;
    int dims[3] = {1,1,1};
    vtkFloatArray *coords[3] = {0,0,0};

    if(d->GetFile() == NULL)
        return NULL;

    d->GetSize(dims[0], dims[1]);
    coords[0] = vtkFloatArray::New();
    coords[0]->SetNumberOfTuples(dims[0]);
    float *xarray = (float *)coords[0]->GetVoidPointer(0);
    for(int i = 0; i < dims[0]; ++i)
        xarray[i] = i;
    coords[1] = vtkFloatArray::New();
    coords[1]->SetNumberOfTuples(dims[1]);
    float *yarray = (float *)coords[1]->GetVoidPointer(0);
    for(int i = 0; i < dims[1]; ++i)
        yarray[i] = i;
    coords[2] = vtkFloatArray::New();
    coords[2]->SetNumberOfTuples(1);
    coords[2]->SetComponent(0, 0, 0.);

    //
    // Create the vtkRectilinearGrid object and set its dimensions
    // and coordinates.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
}


// ****************************************************************************
//  Method: avtOpenEXRFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

vtkDataArray *
avtOpenEXRFileFormat::GetVar(const char *varname)
{
    return d->ReadChannel(varname);
}


// ****************************************************************************
//  Method: avtOpenEXRFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

vtkDataArray *
avtOpenEXRFileFormat::GetVectorVar(const char *varname)
{
    return GetVar(varname);
}
