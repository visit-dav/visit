// ************************************************************************* //
//                            avtSimDBFileFormat.C                           //
// ************************************************************************* //

#include <avtSimDBFileFormat.h>

#include <string>

#include <avtDatabaseMetaData.h>
#include <avtSimulationInformation.h>

#include <InvalidFilesException.h>

#include <visitstream.h>

using     std::string;

// ****************************************************************************
//  Method: avtSimDB constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 30 18:01:57 PDT 2004
//    Using icc, "ifstream >> string" seemed to fail.  (Note that this may
//    have been using mixed gcc-icc libraries, but nevertheless.....)  So I
//    made the reading of "host" go through a char* to get to a string.
//
// ****************************************************************************

avtSimDBFileFormat::avtSimDBFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    ifstream in(filename);

    char buff[256];
    in >> buff;
    if (string(buff) != "host")
    {
        EXCEPTION2(InvalidFilesException,filename,
                   "expected the first keyword to be 'host'");
    }
    in >> buff;
    host = buff;

    in >> buff;
    if (string(buff) != "port")
    {
        EXCEPTION2(InvalidFilesException,filename,
                   "expected the second keyword to be 'port'");
    }
    in >> port;

    in.close();
}

// ****************************************************************************
//  Method: avtSimDBFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

void
avtSimDBFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fill the simulation metadata with the parameters from the file.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

void
avtSimDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    md->SetIsSimulation(true);
    avtSimulationInformation simInfo;
    simInfo.SetHost(host);
    simInfo.SetPort(port);
    md->SetSimInfo(simInfo);
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetMesh
//
//  Purpose:
//      This is an mdserver-only plugin.  Do nothing.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************
vtkDataSet *
avtSimDBFileFormat::GetMesh(int domain, const char *meshname)
{
    return NULL;
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetVar
//
//  Purpose:
//      This is an mdserver-only plugin.  Do nothing.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

vtkDataArray *
avtSimDBFileFormat::GetVar(int domain, const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetVectorVar
//
//  Purpose:
//      This is an mdserver-only plugin.  Do nothing.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

vtkDataArray *
avtSimDBFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}
