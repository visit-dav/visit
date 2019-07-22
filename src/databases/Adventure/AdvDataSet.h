// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ADV_DATA_SET_H
#define ADV_DATA_SET_H
#include <string>
#include <map>
#include <vector>
#include <Adv/AdvDocument.h>

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: AdvDataSet
//
// Purpose:
//   This class manages the conversion of ADV data to VTK data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:31:10 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class AdvDataSet
{
public:
    struct VarInfo
    {
        std::string label;
        std::string fega_type;
        std::string format;

        bool CheckFormat(int &ncomps) const;
    };

    typedef std::vector<VarInfo> VarInfoVector;

    typedef enum
    { 
        ADVENTURE_ELEMENT_TET4,
        ADVENTURE_ELEMENT_TET10,
        ADVENTURE_ELEMENT_HEX8
    } AdvElementType;

    AdvDataSet();
    ~AdvDataSet();

    void SetDomainToSubDomain(int domainID, int subDomain);
    int  GetNumSubDomains() const;

    vtkDataSet   *GetMesh(AdvDocFile *f, int domainID, AdvElementType);
    vtkDataArray *GetNodeVar(AdvDocFile *f, AdvDocument *nodeIndexDoc, 
                             int domainID, const VarInfo &var);
    vtkDataArray *GetElementVar(AdvDocFile *f, int domainID, 
                                const VarInfo &var);

    std::string        modelFile;
    std::string        resultFile;
private:
    vtkDataArray *GetAllElementVariable(AdvDocFile *f, int subDomain,
                                        const VarInfo &var);
    vtkDataArray *GetAllConstant(AdvDocFile *f, int subDomain, 
                                 const VarInfo &var, int ntuples);
    vtkDataArray *GetAllNodeVariable(AdvDocFile *f, AdvDocument *nodeIndexDoc, 
                                     int subDomain, const VarInfo &var);
    void AddGlobalNodeIds(vtkDataSet *ds, AdvDocFile *f);

    std::map<int,int>  domainToSubDomain;
    int                nnodes;
    int                ncells;
};

#endif
