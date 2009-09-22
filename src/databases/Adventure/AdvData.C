/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include "AdvData.h"
#include <visitstream.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: AdvData::AdvData
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:53:37 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvData::AdvData() : domainRecords(), domainToRecord(),
    elementType(AdvDataSet::ADVENTURE_ELEMENT_TET4), modelVars(), resultVars()
{
    memset(advFiles, 0, MAX_ADV_FILES * sizeof(AdvDocFile *));
}

// ****************************************************************************
// Method: AdvData::~AdvData
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:53:48 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvData::~AdvData()
{
    ReleaseData();
}

// ****************************************************************************
// Method: AdvData::ReleaseData
//
// Purpose: 
//   Releases data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:54:00 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AdvData::ReleaseData()
{
    domainRecords.clear();
    domainToRecord.clear();
    modelVars.clear();
    resultVars.clear();

    for(int i = 0; i < MAX_ADV_FILES; ++i)
    {
        if(advFiles[i] != 0)
        {
            adv_dio_file_close(advFiles[i]);
            advFiles[i] = 0;
        }
    }
}

// ****************************************************************************
// Method: AdvData::GetNumDomains
//
// Purpose: 
//   Return the number of domains.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:54:28 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int
AdvData::GetNumDomains() const
{
    return domainToRecord.size();
}

// ****************************************************************************
// Method: AdvData::OpenFile
//
// Purpose: 
//   Opens the specified file, caching it. If the file is already open in the
//   cache then that file handle is returned. If too many files are open then
//   the cache is purged LRU style.
//
// Arguments:
//   filename : The name of the file to open.
//
// Returns:    The pointer to the open file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 16 14:15:14 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvDocFile *
AdvData::OpenFile(const std::string &filename)
{
    AdvDocFile *retval = 0;

    if(filename.size() < 1)
        return retval;

    // Look through the advFiles for a match.
    int index = -1;
    int firstEmptySlot = -1;
    for(int i = 0; i < MAX_ADV_FILES; ++i)
    {
        if(advFiles[i] == 0 && firstEmptySlot == -1)
            firstEmptySlot = i;

        if(advFiles[i] != 0)
        {
            // remove the advdoc: string from the front of the filename.
            std::string locator(adv_dio_file_get_locator(advFiles[i]) + 7);
            if(filename == locator)
                index = i;
        }
    }

    if(index == -1)
    {
        // We did not find a match so we need to open the file.
        retval = adv_dio_file_open(filename.c_str(), "r");

        if(retval != 0)
        {
            if(firstEmptySlot == -1)
            {
                // There's no room for the file. Make room by closing the 1st file.
                adv_dio_file_close(advFiles[0]);
                for(int i = 0; i < MAX_ADV_FILES-1; ++i)
                    advFiles[i] = advFiles[i+1];
                advFiles[MAX_ADV_FILES-1] = retval;
            }
            else
            {
                // There's room for the file so cache it.
                advFiles[firstEmptySlot] = retval;
            }
        }
    }
    else
    {
        // We found an existing file in advFiles so let's move it to last in
        // the list.
        retval =  advFiles[index];
        int id = index;
        for(int i = index+1; i < MAX_ADV_FILES; ++i)
        {
            if(advFiles[i] != 0)
                advFiles[id++] = advFiles[i];
        }
        advFiles[id] = retval;
        for(int i = id+1; i < MAX_ADV_FILES; ++i)
            advFiles[i] = 0;
    }

    return retval;
}

// ****************************************************************************
// Method: FixupPath
//
// Purpose: 
//   If the path to the adv file is relative then the path to the inp file is
//   prepended to make the adv file path absolute.
//
// Arguments:
//   inpname : The name of the inp file as an absolute path.
//   advname : The name of the adv file from the inp file.
//
// Returns:    The absolute path to the adv file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:55:24 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

std::string
FixupPath(const std::string &inpname, const char *advname)
{
    std::string retval;

    if(advname[0] == '/')
        retval = std::string(advname);
    else
    {
        int slashPos = inpname.rfind('/');
        if(slashPos < 0)
            retval = std::string(advname);
        else
            retval = inpname.substr(0, slashPos+1) + advname;
    }

    return retval;
}

// ****************************************************************************
// Method: AdvData::Open
//
// Purpose: 
//   Opens either an inp file or an adv file. If the file is not one of those
//   types then we throw an exception.
//
// Arguments:
//   filename : The name of the file to open.
//
// Returns:    True on success; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:56:56 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
AdvData::Open(const std::string &filename)
{
    const char *mName = "AdvData::Open: ";

    ReleaseData();

    // If the file is an ".inp" file then we may have more than 1 domain file.
    if(filename.size() >= 3 &&
       filename.substr(filename.size()-3, filename.size()) == "inp")
    {
        // Open the file.
        ifstream ifile(filename.c_str());
        if (ifile.fail())
        {
            return false;
        }

        // Process the .inp file
        char line[1024];
        ifile.getline(line, 1024);
        int nDomains = 0;
        int tmp[4];
        if(sscanf(line, "%d %d %d %d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4)
        {
            // This is not a .inp file. Assume it is PATRAN and skip it.
            EXCEPTION1(InvalidDBTypeException, "This is not an Adventure \".inp\" file");
        }
        if(sscanf(line, "%d", &nDomains) != 1)
        {
            // This is not a .inp file
            EXCEPTION1(InvalidDBTypeException, "This is not an Adventure \".inp\" file");
        }

        domainRecords.reserve(nDomains);
        for(int i = 0; i < nDomains; ++i)
        {
            AdvDataSet domain;

            ifile.getline(line, 1024);
            domain.modelFile = FixupPath(filename, line);

            ifile.getline(line, 1024);
            domain.resultFile = FixupPath(filename, line);

            debug4 << "Domain " << i << endl;
            debug4 << "\tmodelFile=" << domain.modelFile << endl;
            debug4 << "\tresultFile=" << domain.resultFile << endl;

            domainRecords.push_back(domain);
        }
        ifile.close();
    }
    else
    {
        AdvDataSet domain;
        domain.modelFile = filename;
        domainRecords.push_back(domain);
    }

    // Now that we have a set of model and result files, open each of the
    // model files and determine how many subdomains are in them.
    int domainID = 0;
    for(size_t i = 0; i < domainRecords.size(); ++i)
    {
        AdvDocFile *f = adv_dio_file_open(domainRecords[i].modelFile.c_str(), "r");
        if(f == 0)
        {
            debug4 << mName << "Could not open " << domainRecords[i].modelFile
                   << " to determine its domain count." << endl;
            EXCEPTION1(InvalidDBTypeException, "Could not open Adventure file");
        }

        AdvDocument *doc = adv_dio_open_by_property(f, 0, "label", "HDDM_FEA_Model", 0);
        if(doc == 0)
        {
            debug4 << mName << "Could not get HDDM_FEA_Model" << endl;
            EXCEPTION1(InvalidDBTypeException, "Adventure file missing HDDM_FEA_Model");
        }

#ifdef TREAT_SUBDOMAINS_INDIVIDUALLY
        // We're not currently doing this but if we were, we'd need to change how 
        // we read data in AdvDataSet.
        int nSubDomains = 0;
        if(!adv_dio_get_property_int32(doc, "num_subdomains", &nSubDomains))
        {
            debug4 << mName << "Could not get num_subdomains" << endl;
            EXCEPTION1(InvalidDBTypeException, "Adventure file missing "
                "num_subdomains property");
        }

        for(int s = 0; s < nSubDomains; ++s)
        {
            domainToRecord[domainID] = i;
            domainRecords[i].SetDomainToSubDomain(domainID, s);
            domainID++;
        }
#else
        // Treat all subdomains as one domain
        domainToRecord[domainID] = i;
        domainRecords[i].SetDomainToSubDomain(domainID, 0);
        domainID++;
#endif
        adv_dio_close(doc);
        adv_dio_file_close(f);
    }

    // Read the file metadata.
    AdvDocFile *f = OpenFile(domainRecords[0].modelFile);
    if(f != 0)
    {
        ReadMetaDataFromFile(f, modelVars, elementType);

        f = OpenFile(domainRecords[0].resultFile);
        if(f != 0)
        {
            AdvDataSet::AdvElementType et;
            ReadMetaDataFromFile(f, resultVars, et);
        }
    }

    return true;
}

// ****************************************************************************
// Method: AdvData::ReadMetaDataFromFile
//
// Purpose: 
//   Reads metadata from the file.
//
// Arguments:
//   f           : The file from which we're reading metadata.
//   vars        : The list of vars in the file. The method sets this.
//   elementType : The element type in the file. The method sets this.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 15:59:30 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AdvData::ReadMetaDataFromFile(AdvDocFile *f, 
    AdvDataSet::VarInfoVector &vars, AdvDataSet::AdvElementType &elementType)
{
    // Now, let's iterate over the documents and add ones that look like variables.
    AdvDocument *doc = 0;
    int i = 0;
    while( (doc = adv_dio_open_nth(f, i++)) != 0)
    {
        const char *content_type = adv_dio_get_property(doc, "content_type");
        if(content_type != 0)
        {
            if(strcmp(content_type, "FEGenericAttribute") == 0 ||
               strcmp(content_type, "HDDM_FEGenericAttribute") == 0)
            {
                const char *fega_type = adv_dio_get_property(doc, "fega_type");
                const char *label = adv_dio_get_property(doc, "label");
                const char *format = adv_dio_get_property(doc, "format");
                if(fega_type != 0 && label != 0 && format != 0)
                {
                    AdvDataSet::VarInfo v;
                    v.label = label;
                    v.fega_type = fega_type;
                    v.format = format;
                    vars.push_back(v);
                }
            }
            else if(strcmp(content_type, "HDDM_Element") == 0)
            {
                const char *element_type = adv_dio_get_property(doc, "element_type");
                if(element_type != 0)
                {
                    if(strcmp("3DLinearTetrahedron", element_type) == 0 )
                        elementType = AdvDataSet::ADVENTURE_ELEMENT_TET4;
                    else if ( strcmp("3DQuadraticTetrahedron", element_type) == 0 ) 
                        elementType = AdvDataSet::ADVENTURE_ELEMENT_TET10;
                    else if ( strcmp("3DLinearHexahedron", element_type) == 0 ) 
                        elementType = AdvDataSet::ADVENTURE_ELEMENT_HEX8;
                }
            }
        }
        adv_dio_close(doc);
    }
}

// ****************************************************************************
// Method: AdvData::GetMetaData
//
// Purpose: 
//   Gets metadata about the file that we read when the file was opened.
//
// Arguments:
//   vars : The list of variables to populate.
//   et   : The return element type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:01:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AdvData::GetMetaData(AdvDataSet::VarInfoVector &vars, AdvDataSet::AdvElementType &et)
{
    for(size_t i = 0; i < modelVars.size(); ++i)
        vars.push_back(modelVars[i]);
    for(size_t i = 0; i < resultVars.size(); ++i)
        vars.push_back(resultVars[i]);

    et = elementType;
}

// ****************************************************************************
// Method: AdvData::GetMesh
//
// Purpose: 
//   Get the VTK dataset for the specified domain.
//
// Arguments:
//   domain : The domain for which we want the dataset.
//
// Returns:    A VTK dataset.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:02:12 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
AdvData::GetMesh(int domain)
{
    vtkDataSet *retval = 0;

    if(domainToRecord.find(domain) != domainToRecord.end())
    {
        int recordIndex = domainToRecord[domain];

        // Okay, we need to open the model file
        AdvDocFile *f = OpenFile(domainRecords[recordIndex].modelFile);

        // Get the mesh
        if(f != 0)
            retval = domainRecords[recordIndex].GetMesh(f, domain, elementType);
    }

    return retval;
}

// ****************************************************************************
// Method: AdvData::GetVar
//
// Purpose: 
//   Get the named variable for the specified domain.
//
// Arguments:
//   domain : The domain number for which we want data.
//   var    : The variable for which we want data.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:02:45 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvData::GetVar(int domain, const std::string &var)
{
    vtkDataArray *retval = 0;

    if(domainToRecord.find(domain) != domainToRecord.end())
    {
        int recordIndex = domainToRecord[domain];

        // Determine the centering.
        bool nodal = false;
        for(size_t i = 0; i < modelVars.size(); ++i)
        {
            if(modelVars[i].label == var)
                nodal |= modelVars[i].fega_type.find("NodeVar") != std::string::npos;
        }
        for(size_t i = 0; i < resultVars.size(); ++i)
        {
            if(resultVars[i].label == var)
                nodal |= resultVars[i].fega_type.find("NodeVar") != std::string::npos;
        }

        // For nodal variables, we need to also pass the NodeIndex_SubdomainToPart
        // document so we can map from subdomain to part.
        AdvDocument *nodeIndexDoc = 0;
        if(nodal)
        {
            AdvDocFile *f = OpenFile(domainRecords[recordIndex].modelFile);
            if (f == 0 || 
               (nodeIndexDoc = adv_dio_open_by_property(f, 0, 
                "content_type", "HDDM_FEGenericAttribute", 
                "label", "NodeIndex_SubdomainToPart", 0)) == 0)
            {
                EXCEPTION1(InvalidVariableException, var.c_str());
            }
        }

        TRY
        {
            for(size_t i = 0; i < modelVars.size() && retval == 0; ++i)
            {
                if(modelVars[i].label == var)
                {
                    // Okay, we need to open the model file
                    AdvDocFile *f = OpenFile(domainRecords[recordIndex].modelFile);
                
                    // Get the var
                    if(f != 0)
                    {
                        if(nodal)
                            retval = domainRecords[recordIndex].GetNodeVar(f, nodeIndexDoc, domain, modelVars[i]);
                        else
                            retval = domainRecords[recordIndex].GetElementVar(f, domain, modelVars[i]);
                    }
                }
            }

            for(size_t i = 0; i < resultVars.size() && retval == 0; ++i)
            {
                if(resultVars[i].label == var)
                {
                    // Okay, we need to open the result file
                    AdvDocFile *f = OpenFile(domainRecords[recordIndex].resultFile);
                
                    // Get the var
                    if(f != 0)
                    {
                        if(nodal)
                            retval = domainRecords[recordIndex].GetNodeVar(f, nodeIndexDoc, domain, resultVars[i]);
                        else
                            retval = domainRecords[recordIndex].GetElementVar(f, domain, resultVars[i]);
                    }
                }
            }

            if(nodal)
                adv_dio_close(nodeIndexDoc);
        }
        CATCHALL
        {
            if(nodal)
                adv_dio_close(nodeIndexDoc);
            RETHROW;
        }
        ENDTRY
    }

    return retval;
}
