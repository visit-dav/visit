// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtDataObjectReader.h                         //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_READER_H
#define AVT_DATA_OBJECT_READER_H

#include <pipeline_exports.h>

#include <string>
#include <ref_ptr.h>

#include <avtDataSetReader.h>
#include <avtImageReader.h>
#include <avtNullDataReader.h>


// ****************************************************************************
//  Class: avtDataObjectReader
//
//  Purpose:
//      A front end that finds the appropriate image or dataset reader.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectReader
{
  public:
                                 avtDataObjectReader();
    virtual                     ~avtDataObjectReader();

    void                         Read(int size, char *input);

    bool                         InputIsDataset(void);
    bool                         InputIsImage(void);
    bool                         InputIsNullData(void);
    bool                         InputIs(const char *);

    avtDataObject_p              GetOutput(void);
    avtImage_p                   GetImageOutput(void);
    avtDataset_p                 GetDatasetOutput(void);
    avtNullData_p                GetNullDataOutput(void);

    avtDataObjectInformation    &GetInfo(void);

  protected:
    bool                         haveInput;

    bool                         datasetInput;
    bool                         imageInput;
    bool                         nullDataInput;

    avtDataSetReader_p           datasetReader;
    avtImageReader_p             imageReader;
    avtNullDataReader_p          nullDataReader;

  private:
    std::string                  inputTypeStr;
};


typedef  ref_ptr<avtDataObjectReader> avtDataObjectReader_p;


#endif


