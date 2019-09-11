// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDataRepresentation.h                             //
// ************************************************************************* //

#ifndef AVT_DATAREPRESENTATION_H
#define AVT_DATAREPRESENTATION_H
#include <pipeline_exports.h>


#include <array_ref_ptr.h>
#include <string>


class  avtWebpage;
class  vtkDataSet;
namespace vtkh
{
class  DataSet;
}


typedef enum
{
    DATA_REP_TYPE_VTK,
    DATA_REP_TYPE_VTKM
} DataRepType;

typedef enum
{
    DATASET_TYPE_RECTILINEAR,
    DATASET_TYPE_CURVILINEAR,
    DATASET_TYPE_UNSTRUCTURED,
    DATASET_TYPE_POLYDATA,
    DATASET_TYPE_NULL,
    DATASET_TYPE_UNKNOWN
}  DataSetType;


// ****************************************************************************
//  Class: avtDataRepresentation
//
//  Purpose:
//      Buffers how data is being stored - as a vtkDataSet or as a char
//      string.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
//    Jeremy Meredith, Tue Sep 26 09:28:46 PDT 2000
//    Added constructors from VTK datasets and strings.  Added copy 
//    constructor.  Removed SetDomain/SetDomainString and Merge.
//    Added assignment operator.
//
//    Kathleen Bonnell, Thu Apr  5 13:27:02 PDT 2001 
//    Changed class name to avtDataRepresentation.
//
//    Hank Childs, Mon Jun  4 14:02:19 PDT 2001
//    Added domain identifiers with the data representation.
//
//    Hank Childs, Mon Sep 17 09:11:23 PDT 2001
//    Changed way the string representation is managed to prevent unnecessary
//    copies.  Removed unused method CreateDataString.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Added member 'label' and retrieval method. Modifed constructor to
//    include the label.
//    
//    Hank Childs, Wed Mar 17 19:16:48 PST 2004
//    Added a static null dataset to prevent unnecessary memory usage with SR.
//
//    Mark C. Miller, Wed Nov 16 13:31:19 PST 2005
//    Added compression
//
//    Hank Childs, Thu Dec 21 10:07:40 PST 2006
//    Add support for debug dumps
//
//    Cyrus Harrison, Tue Mar 13 11:37:30 PDT 2007
//    Add support for debug dumps without full vtk dataset dump (-info-dump)
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Remove const return type of assignment operator, to prevent the compiler
//    from defining a second assignment operator for the non-const case.
//
//    Cyrus Harrison, Wed Feb 13 10:25:48 PST 2008
//    Removed DatasetDump option b/c it was migrated to avtDebugDumpOptions.
//
//    Cameron Christensen, Thursday, May 22, 2014
//    Added support for EAVL.
//
//    Eric Brugger, Tue Sep 30 15:05:02 PDT 2014
//    I modified the EAVL version of the avtDataRepresentation constructor
//    to also have domain and label arguments.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
//    Eric Brugger, Thu Dec 10 11:47:06 PST 2015
//    Added support for VTKm.
//
//    Kathleen Biagas, Wed Jan 30 10:41:55 PST 2019
//    Removed support for EAVL.
//
// ****************************************************************************

class PIPELINE_API avtDataRepresentation
{
  public:
                        avtDataRepresentation();
                        avtDataRepresentation(vtkDataSet *, int, std::string,
                                              bool dontCopyData = false);
                        avtDataRepresentation(vtkh::DataSet *, int, std::string,
                                              bool dontCopyData = false);
                        avtDataRepresentation(char *, int, int, std::string,
                                              CharStrRef &, DataSetType);
                        avtDataRepresentation(const avtDataRepresentation &);

    virtual            ~avtDataRepresentation();

    avtDataRepresentation    &operator=(const avtDataRepresentation &);

    DataRepType         GetDataRepType() const { return dataRepType; }
    vtkDataSet         *GetDataVTK(void);
    vtkh::DataSet      *GetDataVTKm(void);
    unsigned char      *GetDataString(int &, DataSetType &);
    unsigned char      *GetCompressedDataString(int &, DataSetType &);
    long long           GetNumberOfCells(int topoDim, bool polysOnly) const;

    float               GetCompressionRatio() const;
    float               GetTimeToCompress() const;
    float               GetTimeToDecompress() const;

    bool                Valid(void);

    int                 GetDomain(void) { return domain; };
    std::string         GetLabel(void)  { return label; };

    const char         *DebugDump(avtWebpage *, const char *);

  protected:
    vtkDataSet         *asVTK;
    vtkh::DataSet      *asVTKm;
    unsigned char      *asChar;
    int                 asCharLength;
    CharStrRef          originalString;
    DataSetType         datasetType;
    DataRepType         dataRepType;

    float               compressionRatio;
    float               timeToCompress;
    float               timeToDecompress;

    int                 domain;
    std::string         label;

    static bool           initializedNullDatasets;
    static vtkDataSet    *nullVTKDataset;
    static vtkh::DataSet *nullVTKmDataset;

    unsigned char      *GetDataString(int &, DataSetType &, bool);
    unsigned char      *vtkToString(bool compress);
    static void         InitializeNullDatasets(void);
    static void         DeleteNullDatasets(void);
    static DataSetType  DatasetTypeForVTK(vtkDataSet *);

 private:
    vtkDataSet*         VTKmToVTK(vtkh::DataSet *data);
    vtkh::DataSet*      VTKToVTKm(vtkDataSet *data);
};

#endif
