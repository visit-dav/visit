// ************************************************************************* //
//                            avtSTLFileFormat.C                             //
// ************************************************************************* //

#include <avtSTLFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkSTLReader.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


//
// Define the static const's
//

const char   *avtSTLFileFormat::MESHNAME = "STL_mesh";


// ****************************************************************************
//  Method: avtSTLFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    May 24, 2002
//
// ****************************************************************************

avtSTLFileFormat::avtSTLFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
}


// ****************************************************************************
//  Method: avtSTLFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtSTLFileFormat::~avtSTLFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtSTLFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

void
avtSTLFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from STL file " << filename << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (dataset != NULL)
    {
        dataset->Delete();
    }

    //
    // Create a file reader and set our dataset to be its output.
    //
    vtkSTLReader *reader = vtkSTLReader::New();
    reader->SetFileName(filename);
    dataset = reader->GetOutput();
    dataset->Register(NULL);

    //
    // Force the read and make sure that the reader is really gone, so we don't
    // eat up too many file descriptors.
    //
    dataset->Update();
    dataset->SetSource(NULL);
    reader->Delete();

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a STL dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jun 24 10:23:18 PDT 2004
//    Make sure to increment the reference count, since the caller will believe
//    that it owns the returned dataset.
//
// ****************************************************************************

vtkDataSet *
avtSTLFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from STL file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    dataset->Register(NULL);
    return dataset;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as STL scalars.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

vtkDataArray *
avtSTLFileFormat::GetVar(const char *var)
{
    EXCEPTION1(InvalidVariableException, var);
}


// ****************************************************************************
//  Method: avtSTLFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

void
avtSTLFileFormat::FreeUpResources(void)
{
    debug4 << "STL file " << filename 
           << " forced to free up resources." << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }

    readInDataset = false;
}


// ****************************************************************************
//  Method: avtSTLFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  7 09:21:53 PST 2003 
//    Do not bother reading in the dataset, since we learn nothing meaningful
//    from it anyway.  The VTK routines assume that the dataset being read
//    in is truly an STL file.
//
// ****************************************************************************

void
avtSTLFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int spat = 3;
    int topo = 2;

    avtMeshType type = AVT_SURFACE_MESH;;

    AddMeshToMetaData(md, MESHNAME, type, NULL, 1, 0, spat, topo);
}


