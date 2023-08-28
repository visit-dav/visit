// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//  avtPVDFileReader.C
// ***************************************************************************

#include <avtPVDFileReader.h>
#include <VTMParser.h>

#include <vtkDataSet.h>
#include <vtkNew.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>
#include <vtkVisItXMLPDataReader.h>

#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <InvalidFilesException.h>
#include <visit-config.h>

#include <set>

using std::array;
using std::map;
using std::set;
using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtPVDFileReader constructor
//
//  Arguments:
//      fname    The file name.
//      opt      Read options
//
//  Programmer:  Kathleen Biagas
//  Creation:    August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

avtPVDFileReader::avtPVDFileReader(const char *fname,
    const DBOptionsAttributes *opt) : avtVTKFileReader(fname, opt), pvdFile(fname)
{
    currentTS = -1;
}


// ****************************************************************************
//  Method: avtPVDFileReader destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

avtPVDFileReader::~avtPVDFileReader()
{
    avtVTKFileReader::FreeUpResources();
}


// ****************************************************************************
//  Method: avtPVDFileReader::FreeUpResources
//
//  Purpose:
//      Frees up resources.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtPVDFileReader::FreeUpResources(void)
{
    avtVTKFileReader::FreeUpResources();
    // base class sets this to false, but we don't want to read the
    // metafile again, so reset it to true
    readInDataset = true;
}


// ****************************************************************************
//  Method: avtPVDFileReader::ReadInFile
//
//  Purpose:
//    Parses the PVD file.
//    Throws an exception if it enounters a problem indicating
//    the pvdFile may not be formatted as valid pvd.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtPVDFileReader::ReadInFile(int _domain)
{
    int domain = _domain == -1 ? 0 : _domain;
    if (fileExtension == "pvd")
    {
        // parse the file
        vtkNew<vtkXMLDataParser> parser;

        string baseDir = FileFunctions::Dirname(string(filename)) +
                         VISIT_SLASH_STRING;
        string errorMessage;

        parser->SetFileName(filename);

        if (!parser->Parse())
        {
            errorMessage = "Error parsing file: " + pvdFile;
            EXCEPTION2(InvalidFilesException, pvdFile, errorMessage);
        }

        vtkXMLDataElement *root = parser->GetRootElement();
        if (!root)
        {
            errorMessage = "Error retrieving RootElement after parsing file.";
            EXCEPTION2(InvalidFilesException, pvdFile, errorMessage);
        }
        string rootName(root->GetName());
        if (rootName != "VTKFile")
        {
            errorMessage = pvdFile + "does not contain 'VTKFile' tag.";
            EXCEPTION2(InvalidFilesException, pvdFile, errorMessage);
        }

        string fType = root->GetAttribute("type");
        if (fType != "Collection")
        {
            errorMessage = string("VTKFile type: ") +
                          fType + string(" currently not supported.");
            EXCEPTION2(InvalidFilesException, pvdFile, errorMessage);
        }

        vtkXMLDataElement *collectionNode = root->GetNestedElement(0);
        if (collectionNode->GetName() != fType)
        {
            errorMessage = "First nested element doesn't match " + fType;
            EXCEPTION2(InvalidFilesException, pvdFile, errorMessage);
        }

        // to keep track of unique time states and parts
        std::set<int> parts;
        std::set<double> stimes;

        // Allow for poorly formed XML, if `timestep` or `part` aren't defined
        // for every DataSet these will be set to false, ignore them entirely.
        bool processParts = true;
        bool processTimes = true;

        string blockExtension;
        for (int i = 0; i < collectionNode->GetNumberOfNestedElements(); ++i)
        {
            vtkXMLDataElement *el = collectionNode->GetNestedElement(i);
            if (string(el->GetName()) == "DataSet")
            {
                if (el->GetAttribute("file") == 0)
                {
                    debug3 << "DataSet element " << i << " does not have `file`"
                           << " element, ignoring." << endl;
                    continue;
                }
                string datafile(el->GetAttribute("file"));
                double t =  -1.;
                int partId = -1;
                if (processTimes)
                {
                    if (el->GetScalarAttribute("timestep", t) == 0)
                    {
                        debug3 << "Dataset element " << i << "(" << datafile
                               << ") did not have `timestep` element, will not"
                               << " be processing as multi-time collection."
                               << endl;
                        processTimes = false;
                    }
                    else
                    {
                        stimes.insert(t);
                    }
                }
                if (processParts)
                {
                    if (el->GetScalarAttribute("part", partId) == 0)
                    {
                        debug3 << "Dataset element " << i << "(" << datafile
                               << ") did not have `part` element, will not"
                               << " be processing as multi-part collection."
                               << endl;
                        processParts = false;
                    }
                    else
                    {
                        parts.insert(partId);
                    }
                }
                size_t pos = datafile.find_last_of('.');
                pieceExtension = datafile.substr(pos+1);
                // all subfiles are relative to location of this .pvd
                // file so prepend baseDir
                datafile = baseDir + datafile;
                if(pieceExtension == "vtm")
                {
                    VTMParser *parser = new VTMParser();
                    parser->SetFileName(datafile.c_str());
                    if (!parser->Parse())
                    {
                        string em = parser->GetErrorMessage();
                        delete parser;
                        EXCEPTION2(InvalidFilesException, datafile.c_str(), em);
                        return;
                    }
                    blockExtension = parser->GetBlockExtension();
                    string blockFileName;
                    for (int i = 0; i < parser->GetNumberOfBlocks(); ++i)
                    {
                        blockFileName  = parser->GetBlockFileName(i);
                        dataFileNames.push_back(blockFileName);
                        // keep track of this block's time and partId
                        if (processTimes)
                        {
                            dataFileTimes.push_back(t);
                            timeBlockMap[t].push_back(blockFileName);
                        }
                        if(processParts)
                            dataFilePartIds.push_back(partId);
                    }
                    delete parser;
                }
                else if (pieceExtension[0] == 'p')
                {
                    vtkNew<vtkVisItXMLPDataReader> xmlpReader;
                    xmlpReader->SetFileName(datafile.c_str());
                    xmlpReader->ReadXMLInformation();
                    string pieceFileName;
                    bool processExtents = true;
                    for (int i = 0; i < xmlpReader->GetNumberOfPieces(); ++i)
                    {
                        pieceFileName  = xmlpReader->GetPieceFileName(i);
                        dataFileNames.push_back(pieceFileName);
                        if(processExtents)
                        {
                            int *pe = xmlpReader->GetExtent(i);
                            if(pe == NULL)
                            {
                                processExtents = false;
                                allPieceExtents.clear();
                            }
                            else
                            {
                                array<int,6> ext={pe[0], pe[1], pe[2], pe[3], pe[4], pe[5]};
                                allPieceExtents.push_back(ext);
                                if (processTimes)
                                {
                                    timePieceMap[t].push_back(ext);
                                }
                            }
                        }
                        // keep track of this block's time and partId
                        if (processTimes)
                        {
                            dataFileTimes.push_back(t);
                            timeBlockMap[t].push_back(pieceFileName);
                        }
                        if(processParts)
                            dataFilePartIds.push_back(partId);
                    }
                }
                else
                {
                    dataFileNames.push_back(datafile);
                    // keep track of this block's time and partId
                    if(processTimes)
                    {
                        timeBlockMap[t].push_back(datafile);
                        dataFileTimes.push_back(t);
                    }
                    if(processParts)
                        dataFilePartIds.push_back(partId);
                }
            }
        }
        // We couldn't process a time for every data file,
        // so ignore time altogether (Probably a very rare case)
        if (dataFileTimes.size() != dataFileNames.size())
        {
            stimes.clear();
            dataFileTimes.clear();
            timeBlockMap.clear();
        }
        // We couldn't process a part for every data file,
        // so ignore parts altogether (Probably a very rare case)
        if (dataFilePartIds.size() != dataFileNames.size())
        {
            parts.clear();
            dataFilePartIds.clear();
        }
        times.resize(stimes.size());
        if(stimes.size() > 0)
            std::copy(stimes.begin(), stimes.end(), times.begin());

        ngroups = int(parts.size());
        nblocks = GetNBlocks();
        if (ngroups > 1)
        {
            groupPieceName = "part";
            groupIds   = dataFilePartIds;
        }

        blockPieceName = "block";

        if (times.empty())
        {
            pieceFileNames = dataFileNames;
            pieceExtents = allPieceExtents;
        }
        else
        {
            pieceFileNames = timeBlockMap[times[0]];
            pieceExtents = timePieceMap[times[0]];
        }

        pieceDatasets = new vtkDataSet*[nblocks];
        for (int i = 0; i < nblocks; ++i)
        {
            pieceDatasets[i] = NULL;
        }

        // Have already extracted all the filenames from 'pvt?' or vtm files
        // so need to adjust the extension
        if (pieceExtension[0] == 'p')
            pieceExtension = pieceExtension.substr(1);
        else if (!blockExtension.empty())
            pieceExtension = blockExtension;

        ReadInDataset(domain);
        readInDataset = true;
    }
}


// ****************************************************************************
//  Method: avtVTKFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data. Base class does bulk of work.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtPVDFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Make sure md knows the times provided are good.
    if(!times.empty())
    {
        md->SetTimes(times);
        md->SetTimesAreAccurate(true);
    }
    avtVTKFileReader::PopulateDatabaseMetaData(md);
}


// ****************************************************************************
//  Method: avtPVDFileReader::GetNTimes
//
//  Purpose:
//    Returns number of timestates. Parses the PVD file if necessary.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

int
avtPVDFileReader::GetNTimes(void)
{
    if(!readInDataset)
    {
        ReadInFile(-1);
    }
    return int(times.size() ? times.size() : 1);
}


// ****************************************************************************
//  Method: avtPVDFileReader::GetNBlocks
//
//  Purpose:
//    Returns number of blocks for current timestate.
//
//  Arguments:
//    ts    The desired timestate.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************

int
avtPVDFileReader::GetNBlocks(int ts)
{
    if(!timeBlockMap.empty() && ts >= 0 && ts < int(times.size()))
    {
        return int(timeBlockMap[times[ts]].size());
    }
    else
    {
        return int(dataFileNames.size());
    }
}


// ****************************************************************************
//  Method: avtPVDFileReader::ActivateTimestep
//
//  Purpose:
//    Frees resources from old timestate, sets up for new timestate.
//    Does nothing if pvd file has not yet been processed (!readInDataset).
//
//  Arguments:
//    ts    The timestate to be activated.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//
// ****************************************************************************
void
avtPVDFileReader::ActivateTimestep(int ts)
{
    if (!readInDataset || (!times.empty() && (ts < 0 || ts >= int(times.size()))))
        return;

    if (currentTS != ts)
    {
        currentTS = ts;
        FreeUpResources();
    }

    if(pieceFileNames.empty())
    {
        if (times.empty())
            pieceFileNames = dataFileNames;
        else
            pieceFileNames = timeBlockMap[times[ts]];
    }

    if (pieceDatasets == NULL)
    {
        pieceDatasets = new vtkDataSet*[nblocks];
        for (int i = 0; i < nblocks; ++i)
        {
            pieceDatasets[i] = NULL;
        }
    }
    if (pieceExtents.empty())
    {
        if (times.empty())
            pieceExtents = allPieceExtents;
        else
            pieceExtents = timePieceMap[times[ts]];
    }
}
