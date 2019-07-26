// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ADIOS_FILE_OBJECT_H
#define ADIOS_FILE_OBJECT_H
#include <string>
#include <vector>
#include <map>
#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

//NOTE: #include <mpi.h> *MUST* become before the adios includes.
#ifdef PARALLEL
#include <mpi.h>
#else
extern "C"
{
#include <mpidummy.h>
}
#define _NOMPI
#endif

extern "C"
{
#include <adios_read.h>

extern void adios_read_bp_reset_dimension_order (const ADIOS_FILE *fp, int is_fortran);
}

class vtkDataArray;
class vtkPoints;

// ****************************************************************************
//  Class: ADIOSFileObject
//
//  Purpose:
//      Wrapper around an ADIOS file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:15:32 EST 2010
//
//  Modifications:
//
//  Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//  Major overhaul. Added scalars, attributes, and reorganized the class.
//
//  Dave Pugmire, Wed Mar 17 15:29:24 EDT 2010
//  Add a vtkFloatArray ReadVariable method.
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Add GetStringScalar.
//
// ****************************************************************************

class ADIOSFileObject
{
  public:
    ADIOSFileObject(const char *fname);
    ADIOSFileObject(const std::string &fname);
    virtual ~ADIOSFileObject();

    bool Open();
    void Close();
    bool IsOpen() const {return fp != NULL;}
    std::string Filename() const {return fileName;}
    int NumTimeSteps();
    void GetCycles(std::vector<int> &cycles);

    vtkPoints *ReadCoordinates(const std::string &nm, int ts, int dim, int nPts);
    bool ReadScalarData(const std::string &nm, int ts, vtkDataArray **arr)
    {
        return ReadScalarData(nm, ts, (ADIOS_SELECTION*)NULL, arr);
    }
    bool ReadScalarData(const std::string &nm, int ts, int block, vtkDataArray **arr);
    bool ReadScalarData(const std::string &nm, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr);
    
    //Handle complex variables.
    bool ReadComplexData(const std::string &nm, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr, int idx);
    bool ReadComplexRealData(const std::string &nm, int ts, vtkDataArray **arr)
    {
        return ReadComplexData(nm, ts, (ADIOS_SELECTION*)NULL, arr, 0);
    }
    bool ReadComplexRealData(const std::string &nm, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr)
    {
        return ReadComplexData(nm, ts, sel, arr, 0);
    }

    bool ReadComplexImagData(const std::string &nm, int ts, vtkDataArray **arr)
    {
        return ReadComplexData(nm, ts, (ADIOS_SELECTION*)NULL, arr, 1);
    }
    bool ReadComplexImagData(const std::string &nm, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr)
    {
        return ReadComplexData(nm, ts, sel, arr, 1);
    }

    //Attributes
    bool GetAttr(const std::string &nm, int &val);
    bool GetAttr(const std::string &nm, double &val);
    bool GetAttr(const std::string &nm, std::string &val);
    
    //Scalars.
    bool GetScalar(const std::string &nm, int &val);
    bool GetScalar(const std::string &nm, double &val);
    bool GetScalar(const std::string &nm, std::string &val);

    ADIOS_MESH * GetMeshInfo(ADIOS_VARINFO *avi);

    std::map<std::string, ADIOS_VARINFO*> variables, scalars;
    std::map<std::string, int> attributes;

    void SetResetDimensionOrder() {resetDimensionOrder = true;}
    void UnsetResetDimensionOrder() {resetDimensionOrder = false;}

  protected:
    std::string fileName;

    ADIOS_FILE *fp;
    int numTimeSteps;
    bool resetDimensionOrder;

    ADIOS_SELECTION *CreateSelection(ADIOS_VARINFO *avi) {return CreateSelection(avi,-1);}
    ADIOS_SELECTION *CreateSelection(ADIOS_VARINFO *avi, int block);
    bool ReadScalarData(ADIOS_VARINFO *avi, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr);
    vtkDataArray *AllocateScalarArray(ADIOS_VARINFO *avi, ADIOS_SELECTION *sel);
    vtkDataArray *AllocateTypedArray(ADIOS_VARINFO *avi);
    
    void ResetDimensionOrder() {adios_read_bp_reset_dimension_order(fp, 0);}
    
    static bool SupportedVariable(ADIOS_VARINFO *avi);
};

#endif
