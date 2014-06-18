/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#ifndef AVT_XDB_WRITER_H
#define AVT_XDB_WRITER_H
#include <avtDatabaseWriter.h>
#include <avtDatabaseMetaData.h>
#include <string>
#include <vector>

class avtFieldViewXDBWriterInternal;

// ****************************************************************************
//  Class: avtFieldViewXDBWriter
//
//  Purpose:
//      A module that writes out XDB files.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
//  Modifications:
//
// ****************************************************************************

class avtFieldViewXDBWriter : public avtDatabaseWriter
{
public:
                          avtFieldViewXDBWriter();
    virtual              ~avtFieldViewXDBWriter();

protected:
    virtual void          CheckCompatibility(const std::string &plotName);
    virtual void          OpenFile(const std::string &filename, int nb);
    virtual void          WriteHeaders(const avtDatabaseMetaData *,
                                       std::vector<std::string> &, 
                                       std::vector<std::string> &,
                                       std::vector<std::string> &);
    virtual void          BeginPlot(const std::string &plotName);
    virtual void          WriteChunk(vtkDataSet *, int);
    virtual void          CloseFile(void);

    virtual CombineMode   GetCombineMode(const std::string &plotName) const;
    virtual bool          CreateNormals() const;

    virtual std::vector<std::string> GetDefaultVariables(avtDataRequest_p ds);

    virtual bool          CanHandleMaterials(void);

    virtual void          GetMaterials(bool needsExecute,
                                       const std::string &meshname,
                                       const avtDatabaseMetaData *md,
                                       std::vector<std::string> &materialList);

    virtual avtContract_p ApplyMaterialsToContract(avtContract_p c0, 
                                       const std::string &meshname,
                                       const std::vector<std::string> &mats,
                                       bool &changed);
private:
    avtFieldViewXDBWriterInternal *impl;
};

#endif
