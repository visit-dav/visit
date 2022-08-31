// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtPVTKFileReader.C
// ****************************************************************************

#include <avtPVTKFileReader.h>

#include <PVTKParser.h>
#include <vtkVisItXMLPDataReader.h>

#include <InvalidFilesException.h>

#include <algorithm>

using std::array;
using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtPVTKFileReader constructor
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

avtPVTKFileReader::avtPVTKFileReader(const char *fname,
    const DBOptionsAttributes *dbo) : avtVTKMultiBlockFileReader(fname,dbo)
{
    // find the file extension
    fileExtension = "none";
    size_t i = filename.rfind('.', filename.length());
    if (i != string::npos)
        fileExtension = filename.substr(i+1, filename.length() -i);
}



// ****************************************************************************
//  Method: avtPVTKFileReader destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

avtPVTKFileReader::~avtPVTKFileReader()
{
}


// ****************************************************************************
//  Method: avtPVTKFileReader::ReadInFile
//
//  Purpose:
//      Reads in the .vt? file.
//
//  Notes:
//      Originally part of avtVTKFileReader.
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
avtPVTKFileReader::ReadInFile(int _domain)
{
    int domain = _domain == -1 ? 0 : _domain;

    if (fileExtension == "pvtu" || fileExtension == "pvts" ||
        fileExtension == "pvtr" || fileExtension == "pvti" ||
        fileExtension == "pvtp")
    {
        vtkVisItXMLPDataReader *xmlpReader = vtkVisItXMLPDataReader::New();
        xmlpReader->SetFileName(filename.c_str());
        xmlpReader->ReadXMLInformation();

        nblocks = xmlpReader->GetNumberOfPieces();
        pieceFileNames.resize(nblocks);
        pieceExtents.resize(nblocks);
        for (int i = 0; i < nblocks; i++)
        {
            pieceFileNames[i] = xmlpReader->GetPieceFileName(i);

            int *ext = xmlpReader->GetExtent(i);
            if (ext != NULL)
            {
                array<int, 6> pe = {ext[0], ext[1], ext[2], ext[3], ext[4], ext[5]};
                pieceExtents[i] = pe;
            }
        }

        xmlpReader->Delete();
    }
    else if (fileExtension == "pvtk")
    {
        PVTKParser *parser = new PVTKParser();
        parser->SetFileName(filename.c_str());
        if (!parser->Parse())
        {
            string em = parser->GetErrorMessage();
            delete parser;
            EXCEPTION2(InvalidFilesException, filename, em);
        }

        nblocks = int(parser->GetNumberOfPieces());
        pieceFileNames.resize(nblocks);
        for (int i = 0; i < nblocks; i++)
        {
            pieceFileNames[i] = parser->GetPieceFileName(i);
        }

        if (parser->HasExtents())
        {
            pieceExtents.resize(nblocks);
            for (int i = 0; i < nblocks; i++)
            {
                vector<int> &readerExtent = parser->GetPieceExtent(i);
                array<int,6> pe;
                std::copy_n(readerExtent.begin(), 6, pe.begin());
                pieceExtents[i] = pe;
            }
        }

        delete parser;
    }

    pieceDatasets.resize(nblocks);
    for (int i = 0; i < nblocks; i++)
        pieceDatasets[i] = nullptr;

    ReadInDataset(domain);

    haveReadFile = true;
}

