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
    enum Flavor {GTC, BASIC, MEUMMAPS, LAMMPS, SPECFEM, FAIL};
    bool isSST = (std::string(list[0]).find(".sst") != std::string::npos);
    bool isHDF5 = (std::string(list[0]).find(".h5") != std::string::npos);
    bool isWDM = (std::string(list[0]).find(".ssc") != std::string::npos);
    bool stagingMode = isSST || isWDM;

    cout<<__FILE__<<" "<<__LINE__<<" isSST "<<isSST<<endl;
    cout<<__FILE__<<" "<<__LINE__<<" isWDM "<<isWDM<<endl;
    cout<<__FILE__<<" "<<__LINE__<<" isHDF5 "<<isHDF5<<endl;
    Flavor flavor = FAIL;
    if (list != NULL || nList > 0)
    {
        std::shared_ptr<adios2::ADIOS> adios(std::make_shared<adios2::ADIOS>(adios2::DebugON));
        adios2::IO io(adios->DeclareIO("ReadBP"));
        adios2::Engine reader;
        std::map<std::string, adios2::Params> variables, attributes;
        // Determine the type of reader that we want to use.
        TRY
        {
            flavor = BASIC;

            std::string engineName = ADIOS2Helper_GetEngineName(list[0]);
            std::string fileName   = ADIOS2Helper_GetFileName(list[0]);
            bool stagingMode  = ADIOS2Helper_IsStagingEngine(engineName);

            io.SetEngine(engineName);
            //cout<<__FILE__<<" "<<__LINE__<<" Connect to stream "<<fileName<<" ..."
            //    <<" engine "<<engineName<<" ..."<<endl;
            reader = io.Open(fileName, adios2::Mode::Read);
            if (stagingMode)
            {
                cout<<__FILE__<<" "<<__LINE__<<" Get first step "<<endl;
                adios2::StepStatus status =
                    reader.BeginStep(adios2::StepMode::Read, -1.0f);
                if (status == adios2::StepStatus::OK)
                {
                    std::cout<<" Identifier received streaming step = "
                        <<reader.CurrentStep()<<endl;
                    variables = io.AvailableVariables();
                    attributes = io.AvailableAttributes();

                    if (false)
                        ;
                    // add formats here that support staging and HDF5
                    else if (avtGTCFileFormat::IdentifyADIOS2(variables,attributes))
                        flavor = GTC;
                    else if (avtLAMMPSFileFormat::IdentifyADIOS2(variables,attributes))
                        flavor = LAMMPS;
                    else if (avtMEUMMAPSFileFormat::IdentifyADIOS2(variables,attributes))
                        flavor = MEUMMAPS;

                    // generic with staging engines
                    else
                        flavor = BASIC;

                    //reader.EndStep();
                }
            }
            else
            {
                variables = io.AvailableVariables();
                attributes = io.AvailableAttributes();
#if MDSERVER
                std::cout<<" MDSERVER Identifier has " << variables.size() << " variables"<<endl;
#else
                std::cout<<" Identifier has " << variables.size() << " variables"<<endl;
#endif
                // add formats here that support reading from HDF5
                if (avtGTCFileFormat::Identify(fileName.c_str()))
                    flavor = GTC;
                else if (avtLAMMPSFileFormat::Identify(fileName.c_str()))
                    flavor = LAMMPS;
                else if (avtMEUMMAPSFileFormat::Identify(fileName.c_str()))
                    flavor = MEUMMAPS;
                else if (avtSpecFEMFileFormat::Identify(fileName.c_str()))
                    flavor = SPECFEM;

                // generic with HDF5 file
                else if (isHDF5)
                    flavor = BASIC;

                // add formats here that only support BP files

                // generic (Base returns true for anything)
                else if (avtADIOS2BaseFileFormat::Identify(list[0]))
                    flavor = BASIC;
            }
        }
        CATCH(VisItException)
        {
            RETHROW;
        }
        ENDTRY

        cout<<"FLAVOR= "<<flavor<<endl;
        switch(flavor)
        {
          case GTC:
            ffi = avtGTCFileFormat::CreateInterfaceADIOS2(
                    list, nList, nBlock, adios, reader, io, variables, attributes);
            break;
          case LAMMPS:
            ffi = avtLAMMPSFileFormat::CreateInterface(
                    list, nList, nBlock);
            break;
          case MEUMMAPS:
            ffi = avtMEUMMAPSFileFormat::CreateInterfaceADIOS2(
                    list, nList, nBlock, adios, reader, io, variables, attributes);
            break;
          case SPECFEM:
            reader.Close();
            ffi = avtSpecFEMFileFormat::CreateInterface(
                    list, nList, nBlock);
            break;
          case BASIC:
              cout<<"OPEN A BASIC READER"<<endl;
            ffi = avtADIOS2BaseFileFormat::CreateInterfaceADIOS2(
                    list, nList, nBlock, adios, reader, io, variables, attributes);
            break;
          default:
            return NULL;
        }
    }

    //cout<<"RETURN ADIOS READER"<<endl;
    return ffi;
}
