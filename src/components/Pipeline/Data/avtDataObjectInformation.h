// ************************************************************************* //
//                           avtDataObjectInformation.h                      //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_INFORMATION_H
#define AVT_DATA_OBJECT_INFORMATION_H
#include <pipeline_exports.h>
#include <ref_ptr.h>

#include <avtDataAttributes.h>
#include <avtDataValidity.h>

class     avtDataObjectString;
class     avtDataObjectWriter;

// ****************************************************************************
//  Class: avtDataObjectInformation
//
//  Purpose:
//      An auxiliary class intended only to be used by avt data objects.  Its
//      purpose is to information about a data object in an encapsulated way
//      that translates across data objects.  It is divided into two classes,
//      one for describing the attributes of the dataset, the other for
//      describing the validity of the dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2000
//  
//  Modifications:
//
//    Hank Childs, Sat Mar 24 15:14:42 PST 2001
//    Split class into two classes, blew away previous comments since they
//    now apply to avtDataAttributes and avtDataValidity.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectInformation
{
  public:
                             avtDataObjectInformation();
    virtual                 ~avtDataObjectInformation();

    void                     Copy(const avtDataObjectInformation &);
 
    avtDataAttributes       &GetAttributes(void)       { return atts; };
    const avtDataAttributes &GetAttributes(void) const { return atts; };
    avtDataValidity         &GetValidity(void)       { return validity; };
    const avtDataValidity   &GetValidity(void) const { return validity; };

    void                     Merge(const avtDataObjectInformation &);
    void                     ParallelMerge(const ref_ptr<avtDataObjectWriter>);

    void                     Write(avtDataObjectString &, 
                                   const avtDataObjectWriter *);
    int                      Read(char *);

  protected:
    avtDataAttributes        atts;
    avtDataValidity          validity;

  private:
    void                     SwapAndMerge(const ref_ptr<avtDataObjectWriter> dobw,
                                          int swapWithProc);
    void                     RecvResult(const ref_ptr<avtDataObjectWriter> dobw,
                                        int swapWithProc);
    void                     SendResult(const ref_ptr<avtDataObjectWriter> dobw,
                                        int swapWithProc);
};

#endif
