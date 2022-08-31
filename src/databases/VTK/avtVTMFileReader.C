// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTMFileReader.C
// ****************************************************************************

#include <avtVTMFileReader.h>

#include <VTMParser.h>

#include <InvalidFilesException.h>

using std::string;


// ****************************************************************************
//  Method: avtVTMFileReader constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Kathleen Biagas
//  Creation:    June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

avtVTMFileReader::avtVTMFileReader(const char *fname,
    const DBOptionsAttributes *opts) : avtVTKMultiGroupFileReader(fname, opts)
{
}


// ****************************************************************************
//  Method: avtVTMFileReader destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
// ****************************************************************************

avtVTMFileReader::~avtVTMFileReader()
{
}

// ****************************************************************************
//  Method: avtVTMFileReader::ReadInFile
//
//  Purpose:
//      Reads in the file.
//
//  Programmer: Eric Brugger
//  Creation:   June 18, 2012
//
//  Modifications:
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names, as read from 'vtm' file.
//
//    Kathleen Biagas, Thu Sep 21 14:59:31 MST 2017
//    Add support for pvtk files.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    pieceFileNames now a vector<string>. Combine 2 for-blocks into 1.
//    pieceExtents now vector<array<int,6>>.
//
// ****************************************************************************

void
avtVTMFileReader::ReadInFile(int _domain)
{
    int domain = _domain == -1 ? 0 : _domain;

    VTMParser *parser = new VTMParser;
    parser->SetFileName(filename);
    if (!parser->Parse())
    {
        string em = parser->GetErrorMessage();
        delete parser;
        EXCEPTION2(InvalidFilesException, filename, em);
        return;
    }

    nblocks = parser->GetNumberOfBlocks();
    ngroups = parser->GetNumberOfGroups();
    if (ngroups > 1)
    {
        groupNames = parser->GetGroupNames();
        groupPieceName = parser->GetGroupPieceName();
        groupIds   = parser->GetGroupIds();
    }

    blockNames = parser->GetBlockNames();
    blockPieceName = parser->GetBlockPieceName();

    pieceFileNames.resize(nblocks);
    for (int i = 0; i < nblocks; i++)
    {
        pieceFileNames[i] = parser->GetBlockFileName(i);
    }
    delete parser;


    pieceDatasets.resize(nblocks);
    for (int i = 0; i < nblocks; i++)
        pieceDatasets[i] = nullptr;

    ReadInDataset(domain);
    haveReadFile = true;
}

