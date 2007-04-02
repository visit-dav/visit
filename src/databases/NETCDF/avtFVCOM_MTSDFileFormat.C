// ************************************************************************* //
//                            avtFVCOM_MTSDFileFormat.C                      //
// ************************************************************************* //

#include <avtFVCOM_MTSDFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>

#include <NETCDFFileObject.h>
#include <avtFVCOMReader.h>
#include <netcdf.h>

using     std::string;

// ****************************************************************************
// Method: avtFVCOMFileFormat::Identify
//
// Purpose:
//   This method checks to see if the file is an FVCOM file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a particle file; False otherwise.
//
// Note:
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
avtFVCOM_MTSDFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool isFVCOM = false;
    
    //
    // Use the fileObject to look for something in the file that will make us
    // believe that it is a FVCOM file. The check can be as complex as you want
    // but it should not return true unless this really is an FVCOM file.
    //
    std::string source;
    if(fileObject->ReadStringAttribute("source", source))
    {
        isFVCOM = strncmp("FVCOM",source.c_str(),5)==0;
    }

    debug4 << "FVCOM MTSD:Identify source= " << source << endl;
    // Make sure it is MTSD.

    if(isFVCOM)
    {
        size_t ntimesteps;
        int status, time_id, ncid;
        ncid=fileObject->GetFileHandle();
      
        status = nc_inq_dimid(ncid, "time", &time_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        debug4 << "FVCOM MTSD:Identify ntimesteps= " << ntimesteps << endl;

        if(ntimesteps==1)
        {
            isFVCOM= false;
        } 
        else
        {
            isFVCOM= true;
        }
    } // end if is FVCOM

    return isFVCOM;
}


// ****************************************************************************
// Method: avtFVCOMFileFormat::CreateInterface
//
// Purpose:
//   Creates the file format interface for this reader.
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//
// ****************************************************************************


avtFileFormatInterface *
avtFVCOM_MTSDFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        if(f != 0)
        {
            ffl[i] = new avtFVCOM_MTSDFileFormat(list[i], f);
            f = 0;
        }
        else
            ffl[i] = new avtFVCOM_MTSDFileFormat(list[i]);
    }

    return new avtMTSDFileFormatInterface(ffl, nList);
}

// ****************************************************************************
//  Method: avtFVCOM_MTSD constructor
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

avtFVCOM_MTSDFileFormat::avtFVCOM_MTSDFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1)
{
    reader = new avtFVCOMReader(filename);
}

avtFVCOM_MTSDFileFormat::avtFVCOM_MTSDFileFormat(const char *filename,
    NETCDFFileObject *f) : avtMTSDFileFormat(&filename, 1)
{
    reader = new avtFVCOMReader(filename,f);
}

// ****************************************************************************
//  Method: avtFVCOM_MTSD destructor
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

avtFVCOM_MTSDFileFormat::~avtFVCOM_MTSDFileFormat()
{
    delete reader;
}

// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

int
avtFVCOM_MTSDFileFormat::GetNTimesteps(void)
{
    return reader->GetNTimesteps();
}


// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

void
avtFVCOM_MTSDFileFormat::FreeUpResources(void)
{
    debug4 << "avtFVCOM_MTSDFileFormat::FreeUpResources: " << endl;
    reader->FreeUpResources();
}


// ****************************************************************************
// Method: avtFVCOMReader::GetCycles
//
// Purpose: 
//   Returns the time cycle in the file.
//
// Arguments:
//   cyc : The times cycle to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications:  Ref to FVCOM Reader class!
//   
// ****************************************************************************

void
avtFVCOM_MTSDFileFormat::GetCycles(std::vector<int> &cycles)
{
    reader->GetCycles(cycles);
}


// ****************************************************************************
// Method: avtFVCOMReader::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   t : The times to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************


void
avtFVCOM_MTSDFileFormat::GetTimes(std::vector<double> &times)
{
    reader->GetTimes(times);
}

// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

void
avtFVCOM_MTSDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    reader->PopulateDatabaseMetaData(md, timeState, GetType());
}


// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

vtkDataSet *
avtFVCOM_MTSDFileFormat::GetMesh(int timestate, const char *meshname)
{
    return reader->GetMesh(timestate, meshname);
}


// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOM_MTSDFileFormat::GetVar(int timestate, const char *varname)
{
    return reader->GetVar(timestate, varname);
}


// ****************************************************************************
//  Method: avtFVCOM_MTSDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:52 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOM_MTSDFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return reader->GetVectorVar(timestate, varname);
}
