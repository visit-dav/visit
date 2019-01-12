#ifndef MSH_DATA_SET_H
#define MSH_DATA_SET_H
#include <vector>
#include <string>
class vtkDataSet;

// ****************************************************************************
// Class: MshData
//
// Purpose:
//   Reads an Adventure MSH input file and serves up VTK data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 14:57:43 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class MshData
{
public:
    MshData();
    ~MshData();

    bool Open(const std::string &filename);
    int GetNumDomains() const;
    int GetCellType() const; // 0=Tet4, 1=Tet10, 2=Hex8

    void ReleaseData();

    vtkDataSet *GetMesh(int);
private:
    std::vector<vtkDataSet *> domains;
    int                       cellType;
};

#endif
