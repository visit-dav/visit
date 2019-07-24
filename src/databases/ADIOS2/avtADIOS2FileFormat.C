// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtFileFormatInterface.h>
#include <ADIOS2HelperFuncs.h>
#include <DebugStream.h>
#include <VisItException.h>

#include <avtADIOS2BaseFileFormat.h>
#include <avtGTCFileFormat.h>
#include <avtLAMMPSFileFormat.h>
#include <avtSpecFEMFileFormat.h>
#include <avtMEUMMAPSFileFormat.h>
#include <memory>
#include <string>
#include <avtParallel.h>

// ****************************************************************************
// Method: ADIOS2_CreateFileFormatInterface
//
// Purpose:
//   Opens the first ADIOS file in the list and attempts to use the various
//   file formats to create a file format interface.
//
// Arguments:
//   list   : The list of filenames.
//   nList  : The number of filenames in the list.
//   nBlock : The number of files in a timestep.
//
// Returns:    A file format interface or 0 if no file format interface
//             was created.
//
// Programmer: Dave Pugmire
// Creation:   Wed Feb  3 13:10:35 EST 2010
//
// Modifications:
//
//  Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//  Added XGC reader.
//
//   Dave Pugmire, Wed Apr  9 13:39:04 EDT 2014
//   Added specFM and schema readers.
//
//   Dave Pugmire, Thu Oct 30 11:59:40 EDT 2014
//   Added a LAMMPS reader. Modified the flavor flag to an enum for clarity.
//
// ****************************************************************************

avtFileFormatInterface *
ADIOS2_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = NULL;

    if (list == NULL && nList == 0)
        return NULL;

    enum Flavor {GTC, BASIC, MEUMMAPS, LAMMPS, SPECFEM, FAIL};
    Flavor flavor = FAIL;

#ifdef PARALLEL
    std::shared_ptr<adios2::ADIOS> adios(std::make_shared<adios2::ADIOS>(VISIT_MPI_COMM, adios2::DebugON));
#else
    std::shared_ptr<adios2::ADIOS> adios(std::make_shared<adios2::ADIOS>(adios2::DebugON));
#endif

    adios2::IO io(adios->DeclareIO("ReadBP"));
    adios2::Engine reader;
    std::map<std::string, adios2::Params> variables, attributes;

    std::string engineName = ADIOS2Helper_GetEngineName(list[0]);
    std::string fileName = ADIOS2Helper_GetFileName(list[0]);
    bool stagingMode = ADIOS2Helper_IsStagingEngine(engineName);

    io.SetEngine(engineName);
    reader = io.Open(fileName, adios2::Mode::Read);

    if (stagingMode)
    {
        adios2::StepStatus status = reader.BeginStep(adios2::StepMode::NextAvailable, -1.0f);
        if (status == adios2::StepStatus::OK)
        {
            variables = io.AvailableVariables();
            attributes = io.AvailableAttributes();
        }
    }
    else
    {
        variables = io.AvailableVariables();
        attributes = io.AvailableAttributes();
    }

    TRY
    {
    //See what flavor we have:
    if (avtGTCFileFormat::Identify(fileName, variables, attributes))
        flavor = GTC;
    else if (avtLAMMPSFileFormat::Identify(fileName, variables,attributes))
        flavor = LAMMPS;
    else if (avtMEUMMAPSFileFormat::Identify(fileName, variables, attributes))
        flavor = MEUMMAPS;
    else if (avtSpecFEMFileFormat::Identify(fileName, variables, attributes))
        flavor = SPECFEM;
    else
        flavor = BASIC;
    }
    CATCH(VisItException)
    {
        RETHROW;
    }
    ENDTRY

    switch(flavor)
    {
    case GTC:
        ffi = avtGTCFileFormat::CreateInterface(list, nList, nBlock, adios, reader, io, variables, attributes);
        break;
    case LAMMPS:
        ffi = avtLAMMPSFileFormat::CreateInterface(list, nList, nBlock, adios, reader, io, variables, attributes);
        break;
    case MEUMMAPS:
        ffi = avtMEUMMAPSFileFormat::CreateInterface(list, nList, nBlock, adios, reader, io, variables, attributes);
        break;
    case SPECFEM:
        ffi = avtSpecFEMFileFormat::CreateInterface(list, nList, nBlock, adios, reader, io, variables, attributes);
        break;
    case BASIC:
        ffi = avtADIOS2BaseFileFormat::CreateInterface(list, nList, nBlock, adios, reader, io, variables, attributes);
        break;
    default:
        return NULL;
    }

    return ffi;
}
