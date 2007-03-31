#ifndef MIR_CONNECTIVITY
#define MIR_CONNECTIVITY

class vtkDataSet;

// ****************************************************************************
//  Class:  MIRConnectivity
//
//  Purpose:
//    Holds connectivity for material interface reconstruction.
//
//  Programmer:  Hank Childs
//  Creation:    October 7, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 09:49:10 PDT 2003
//    Put into its own file.
//
// ****************************************************************************
struct MIRConnectivity
{
    int *connectivity;
    int  ncells;
    int *celltype;
    int *cellindex;
       
    MIRConnectivity();
    ~MIRConnectivity();
    void SetUpConnectivity(vtkDataSet *);
};

#endif
