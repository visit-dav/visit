// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ADV_DATA_H
#define ADV_DATA_H
#include <Adv/AdvDocument.h>
#include <map>
#include <vector>
#include <string>

#include "AdvDataSet.h"

#define MAX_ADV_FILES 10

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: AdvData
//
// Purpose:
//   This class manages access to a set of ADV files as a multidomain object.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:08:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class AdvData
{
public:
    AdvData();
    ~AdvData();

    void ReleaseData();
    bool Open(const std::string &filename);
    int  GetNumDomains() const;
    void GetMetaData(AdvDataSet::VarInfoVector &vars, AdvDataSet::AdvElementType &et);

    vtkDataSet   *GetMesh(int domain);
    vtkDataArray *GetVar(int domain, const std::string &var);

private:
    AdvDocFile *OpenFile(const std::string &filename);
    void ReadMetaDataFromFile(AdvDocFile *f, AdvDataSet::VarInfoVector &, 
                              AdvDataSet::AdvElementType &);

    std::vector<AdvDataSet> domainRecords;
    std::map<int,int>       domainToRecord;
    AdvDocFile             *advFiles[MAX_ADV_FILES];

    AdvDataSet::AdvElementType elementType;
    AdvDataSet::VarInfoVector  modelVars;
    AdvDataSet::VarInfoVector  resultVars;
};

#endif
