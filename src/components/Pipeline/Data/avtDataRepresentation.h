// ************************************************************************* //
//                       avtDataRepresentation.h                             //
// ************************************************************************* //

#ifndef AVT_DATAREPRESENTATION_H
#define AVT_DATAREPRESENTATION_H
#include <pipeline_exports.h>


#include <array_ref_ptr.h>
#include <string>

class  vtkDataSet;


typedef enum
{
    DATASET_TYPE_RECTILINEAR,
    DATASET_TYPE_CURVILINEAR,
    DATASET_TYPE_UNSTRUCTURED,
    DATASET_TYPE_POLYDATA,
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
// ****************************************************************************

class PIPELINE_API avtDataRepresentation
{
  public:
                        avtDataRepresentation();
                        avtDataRepresentation(vtkDataSet *, int, std::string);
                        avtDataRepresentation(char *, int, int, std::string,
                                              CharStrRef &, DataSetType);
                        avtDataRepresentation(const avtDataRepresentation &);
    virtual            ~avtDataRepresentation();

    const avtDataRepresentation    &operator=(const avtDataRepresentation &);

    vtkDataSet         *GetDataVTK(void);
    unsigned char      *GetDataString(int &, DataSetType &);

    bool                Valid(void);

    int                 GetDomain(void) { return domain; };
    std::string         GetLabel(void)  { return label; };

  protected:
    vtkDataSet         *asVTK;
    unsigned char      *asChar;
    int                 asCharLength;
    CharStrRef          originalString;
    DataSetType         datasetType;

    int                 domain;
    std::string         label;
};


#endif


