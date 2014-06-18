/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#ifndef AVT_FIELDVIEWXDB_WRITER_INTERNAL_H
#define AVT_FIELDVIEWXDB_WRITER_INTERNAL_H
#include "VXDB_exports.h"
#include <avtDatabaseMetaData.h>
#include <string>
#include <vector>

#include <avtDataObject.h>
#include <avtContract.h>

class vtkDataSet;

// ****************************************************************************
//  Class: avtFieldViewXDBWriterInternal
//
//  Purpose:
//    The "internal" class that writes out XDB files.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
//  Modifications:
//
// ****************************************************************************

class VXDB_API avtFieldViewXDBWriterInternal
{
public:
    avtFieldViewXDBWriterInternal(int rank);
    ~avtFieldViewXDBWriterInternal();

    static std::string GetCopyright();

    static int CombineNone;       // Do no combination of geometry before export.
    static int CombineNoneGather; // Do no combination of geometry but gather to rank 0
    static int CombineLike;       // Combine like geometry by label and convert to polydata
                                  // so rank 0 will get N polydata objects representing
                                  // data from all ranks (1 per label).
    static int CombineAll;        // Combine all geometry from all ranks onto rank 0 and
                                  // get a single polydata object.

    void          CheckCompatibility(avtDataObject_p input, const std::string &plotName);
    void          OpenFile(avtDataObject_p input, 
                           const std::string &filename, int nb);
    void          WriteHeaders(avtDataObject_p input,
                               const avtDatabaseMetaData *,
                               std::vector<std::string> &, 
                               std::vector<std::string> &,
                               std::vector<std::string> &);
    void          BeginPlot(avtDataObject_p input, const std::string &plotName);
    void          WriteChunk(avtDataObject_p input, vtkDataSet *, int);
    void          CloseFile(avtDataObject_p input);

    int           GetCombineMode(avtDataObject_p input, const std::string &plotName) const;
    bool          CreateNormals(avtDataObject_p input) const;

    std::vector<std::string> GetDefaultVariables(avtDataObject_p input, avtDataRequest_p ds);

    bool          CanHandleMaterials(avtDataObject_p input);

    void          GetMaterials(avtDataObject_p input,
                               bool needsExecute,
                               const std::string &meshname,
                               const avtDatabaseMetaData *md,
                               std::vector<std::string> &materialList);

    avtContract_p ApplyMaterialsToContract(avtDataObject_p input,
                                           avtContract_p c0, 
                                           const std::string &meshname,
                                           const std::vector<std::string> &mats,
                                           bool &changed,
                                           bool &hasMaterialsInProblem);
private:
    class Implementation;

    Implementation *impl;
};

#endif
