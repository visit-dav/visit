// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class     avtWebpage;


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
//    Mark C. Miller, Thu Jun 10 10:05:09 PDT 2004
//    Added message tag args to private comm functions
//
//    Hank Childs, Thu Dec 21 10:10:09 PST 2006
//    Add support for debug dumps.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
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

    void                     DebugDump(avtWebpage *);

  protected:
    avtDataAttributes        atts;
    avtDataValidity          validity;

  private:
    void                     SwapAndMerge(const ref_ptr<avtDataObjectWriter> dobw,
                                          int swapWithProc, int lenTag, int strTag);
    void                     RecvResult(const ref_ptr<avtDataObjectWriter> dobw,
                                        int swapWithProc, int lenTag, int strTag);
    void                     SendResult(const ref_ptr<avtDataObjectWriter> dobw,
                                        int swapWithProc, int lenTag, int strTag);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                             avtDataObjectInformation(const 
                                               avtDataObjectInformation &) {;};
    avtDataObjectInformation      &operator=(const avtDataObjectInformation &) 
                                                             { return *this; };
};

#endif


