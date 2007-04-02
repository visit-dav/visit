// ************************************************************************* //
//                            avtFVCOM_STSDFileFormat.C                      //
// ************************************************************************* //

#include <avtFVCOM_STSDFileFormat.h>


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
#include <avtMaterial.h>
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
avtFVCOM_STSDFileFormat::Identify(NETCDFFileObject *fileObject)
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

    // Make sure it is STSD.
    
    if(isFVCOM)
    {
        size_t ntimesteps;
        int status, time_id, ncid;
        ncid=fileObject->GetFileHandle();
      
        status = nc_inq_dimid(ncid, "time", &time_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        if(ntimesteps==1)
        {
            isFVCOM= true;
        } 
        else
        {
            isFVCOM=false;
        }
    }

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
avtFVCOM_STSDFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];
    int nTimestep = nList / nBlock;
    for (int i = 0 ; i < nTimestep ; i++)
    {
        ffl[i] = new avtSTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            if(f != 0)
            {
                ffl[i][j] = new avtFVCOM_STSDFileFormat(list[i*nBlock + j], f);
                f = 0;
            }
            else
                ffl[i][j] = new avtFVCOM_STSDFileFormat(list[i*nBlock + j]);
        }
    }

    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}


// ****************************************************************************
//  Method: avtFVCOM_STSD constructor
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
  debug4 << "avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat" << endl;

    reader = new avtFVCOMReader(filename);
  debug4 << "avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat: end" << endl;
}

avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat(const char *filename,
   NETCDFFileObject *f)   : avtSTSDFileFormat(filename)
{
  debug4 << "avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat(name f)" << endl;
    reader = new avtFVCOMReader(filename,f);
  debug4 << "avtFVCOM_STSDFileFormat::avtFVCOM_STSDFileFormat(name f): end" << endl;
}


// ****************************************************************************
//  Method: avtFVCOM_STSD destructor
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

avtFVCOM_STSDFileFormat::~avtFVCOM_STSDFileFormat()
{
  debug4 << "avtFVCOM_STSDFileFormat::~avtFVCOM_STSDFileFormat" << endl;
  // debug4 << "reader: " << reader << endl;

  delete reader;
  debug4 << "avtFVCOM_STSDFileFormat::~avtFVCOM_STSDFileFormat: end" << endl;

}


// ****************************************************************************
//  Method: avtFVCOM_STSDFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

void
avtFVCOM_STSDFileFormat::FreeUpResources(void)
{
  debug4 << "avtFVCOM_STSDFileFormat::FreeUpResources" << endl;

    reader->FreeUpResources();
  debug4 << "avtFVCOM_STSDFileFormat::FreeUpResources: end" << endl;

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

int
avtFVCOM_STSDFileFormat::GetCycle(void)
{

  //  reader->GetNTimesteps();
  
  std::vector<int> cycles;
  reader->GetCycles(cycles);
  debug4 << "Cycle size=" << cycles.size() << endl;
  debug4 << "Cycles=" << cycles[0] << endl;
  return (cycles.size() > 0) ? cycles[0] : 0;
}


// ****************************************************************************
// Method: avtFVCOM_STSDFileFormat::GetTime
//
// Purpose: 
//   Returns the time for the dataset.
//
// Programmer: David Stuebe
// Creation:    Wed May 31 15:50:45 PST 2006
//
// Modifications:
//   
// ****************************************************************************


double
avtFVCOM_STSDFileFormat::GetTime(void)
{
    std::vector<double> time;
    reader->GetTimes(time);
    debug4 << "Time size=" << time.size() << endl;
    debug4 << "TIME=" << time[0] << endl;
    return (time.size() > 0) ? time[0] : 0;
}



// ****************************************************************************
//  Method: avtFVCOM_STSDFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

void
avtFVCOM_STSDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4<< "Should run GetCycle" << endl;
    reader->PopulateDatabaseMetaData(md, 0, GetType());
}


// ****************************************************************************
//  Method: avtFVCOM_STSDFileFormat::GetMesh
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
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

vtkDataSet *
avtFVCOM_STSDFileFormat::GetMesh(const char *meshname)
{
    reader->SetDomainIndexForCaching(0);
    return reader->GetMesh(0, meshname, cache);
}




// ****************************************************************************
// Method: avtFVCOM_STSDFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets the material object for the particles.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: David Stuebe
// Creation:   Mon Jul 17 2006
//
// Modifications:
//   
// ****************************************************************************

void *
avtFVCOM_STSDFileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *args, DestructorFunction &df)
{
  return reader->GetAuxiliaryData(var, 0, type, args, df);
}

// ****************************************************************************
//  Method: avtFVCOM_STSDFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOM_STSDFileFormat::GetVar(const char *varname)
{
   reader->SetDomainIndexForCaching(0);
   return reader->GetVar(0, varname, cache);
}


// ****************************************************************************
//  Method: avtFVCOM_STSDFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOM_STSDFileFormat::GetVectorVar(const char *varname)
{
  reader->SetDomainIndexForCaching(0);
  return reader->GetVectorVar(0, varname, cache);
}
