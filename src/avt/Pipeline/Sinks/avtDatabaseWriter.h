/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtDatabaseWriter.h                            //
// ************************************************************************* //

#ifndef AVT_DATABASE_WRITER_H
#define AVT_DATABASE_WRITER_H

#include <avtTerminatingDatasetSink.h>
#include <avtParallelContext.h>

#include <string>
#include <vector>

#include <pipeline_exports.h>


class avtDatabaseMetaData;
class vtkPolyData;

// ****************************************************************************
//  Class: avtDatabaseWriter
//
//  Purpose:
//      This is a general, abstract interface for writing out databases.  It is
//      designed to be utilized by the plugin writers.
//      This differs from avtDatasetFileWriter, in that avtDatasetFileWriter is
//      specialized to specific polygonal file formats.  That writer should
//      probably be renamed.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Sat Sep 11 10:03:49 PDT 2004
//    Add support for repartitioning the chunks.  Also support reading only
//    one variable.
//
//    Brad Whitlock, Mon Nov 1 15:51:32 PST 2004
//    I replaced long long with VISIT_LONG_LONG so the code can build 
//    on Windows using the MSVC 6.0 compiler.
//
//    Jeremy Meredith, Wed Feb 16 15:01:40 PST 2005
//    Added ability to disable MIR and Expressions.
//
//    Hank Childs, Thu May 26 15:45:48 PDT 2005
//    Add an "all-variables" option.
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Jeremy Meredith, Tue Mar 27 17:17:16 EDT 2007
//    Pass the number of blocks into the OpenFile virtual function,
//    because we can't trust the meta data.  Also, allow for a saved
//    pipeline specification in case we need to re-execute the pipeline.
//
//    Hank Childs, Fri Sep  7 17:54:21 PDT 2012
//    Add support for outputZonal.
//
//    Brad Whitlock, Tue Jan 21 15:35:55 PST 2014
//    Added the GetDefaultVariables method.
//    Pass more identifying plot information to the Write method. I added
//    methods to let this class perform polydata aggregation.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Aug  6 16:47:50 PDT 2015
//    Added support for writing using groups of MPI ranks.
//
//    Mark C. Miller, Tue Jun 14 10:38:17 PDT 2016
//    Added logic to keep track of whether the writeContext wound up with
//    some processors with no data.
// ****************************************************************************

class PIPELINE_API avtDatabaseWriter : public virtual avtTerminatingDatasetSink
{
  public:
    typedef enum 
    {
        CombineNone, // Do no combination of geometry before export.

        CombineNoneGather, // Do no combination of geometry but gather to rank 0

        CombineLike, // Combine like geometry by label and convert to polydata
                     // so rank 0 will get N polydata objects representing
                     // data from all ranks (1 per label).

        CombineAll   // Combine all geometry from all ranks onto rank 0 and
                     // get a single polydata object.
    } CombineMode;

                       avtDatabaseWriter();
    virtual           ~avtDatabaseWriter();
   
    void               Write(const std::string &filename,
                             const avtDatabaseMetaData *md);
    void               Write(const std::string &plotName,
                             const std::string &filename,
                             const avtDatabaseMetaData *md,
                             std::vector<std::string> &vars,
                             bool allVars);
    void               Write(const std::string &plotName,
                             const std::string &filename,
                             const avtDatabaseMetaData *md,
                             std::vector<std::string> &vars,
                             bool allVars,
                             bool writeUsingGroups, int groupSize);

    void               SetShouldAlwaysDoMIR(bool s)
                             { shouldAlwaysDoMIR = s; };

    void               SetShouldNeverDoMIR(bool s)
                             { shouldNeverDoMIR = s; };

    void               SetShouldNeverDoExpressions(bool s)
                             { shouldNeverDoExpressions = s; };

    bool               SetTargetChunks(int nChunks);
    bool               SetTargetZones(VISIT_LONG_LONG nTotalZones);
    bool               SetOutputZonal(bool);
    void               SetVariableList(std::vector<std::string> &);
    void               SetContractToUse(avtContract_p ps);

    void               SetWriteContext(avtParallelContext &);
    avtParallelContext &GetWriteContext();
    bool               GetWriteContextHasNoDataProcs(void) const
                           { return writeContextHasNoDataProcs; };
  protected:
    bool               shouldAlwaysDoMIR;
    bool               shouldNeverDoMIR;
    bool               shouldNeverDoExpressions;
    bool               mustGetMaterialsAdditionally;
    bool               hasMaterialsInProblem;

    bool               shouldChangeChunks;
    bool               shouldChangeTotalZones;
    bool               shouldOutputZonal;
    int                nTargetChunks;
    VISIT_LONG_LONG    targetTotalZones;

    avtContract_p      savedContract;
    avtParallelContext writeContext;
    bool               writeContextHasNoDataProcs;

    virtual bool       CanHandleMaterials(void) { return false; };

    virtual void       CheckCompatibility(const std::string &);
    virtual void       OpenFile(const std::string &, int) = 0;
    virtual void       WriteHeaders(const avtDatabaseMetaData *, 
                                    const std::vector<std::string>&scalars,
                                    const std::vector<std::string>&vectors,
                                    const std::vector<std::string>&materials) = 0;
    virtual void       BeginPlot(const std::string &);
    virtual void       WriteChunk(vtkDataSet *, int, int, const std::string &);
    virtual void       WriteChunk(vtkDataSet *, int) = 0; // DEPRECATED VERSION
    virtual void       EndPlot(const std::string &);
    virtual void       CloseFile(void) = 0;
    virtual void       WriteRootFile();

    virtual bool       SupportsTargetChunks(void) { return false; };
    virtual bool       SupportsTargetZones(void) { return false; };
    virtual bool       SupportsOutputZonal(void) { return false; };

    virtual CombineMode GetCombineMode(const std::string &plotName) const;
    virtual bool        CreateTrianglePolyData() const;
    virtual bool        CreateNormals() const;
    virtual bool        SequentialOutput() const;

    std::string         GetMeshName(const avtDatabaseMetaData *md) const;
    double              GetTime() const;
    int                 GetCycle() const;

    virtual int         GetVariables(const std::string &meshname,
                                     const avtDatabaseMetaData *md,
                                     const std::vector<std::string> &varlist,
                                     bool allVars,
                                     bool allowExpressions,
                                     const std::vector<std::string> &defaultVars,
                                     std::vector<std::string> &scalarList,
                                     std::vector<std::string> &vectorList);

    virtual std::vector<std::string> GetDefaultVariables(avtDataRequest_p ds);

    virtual void        GetMaterials(bool needsExecute,
                                     const std::string &meshname,
                                     const avtDatabaseMetaData *md,
                                     std::vector<std::string> &materialList);

    virtual avtContract_p ApplyVariablesToContract(avtContract_p c0, 
                                     const std::string &meshname,
                                     const std::vector<std::string> &vars,
                                     bool &changed);

    virtual avtContract_p ApplyMaterialsToContract(avtContract_p c0, 
                                      const std::string &meshname,
                                      const std::vector<std::string> &mats,
                                      bool &changed);

    vtkPolyData               *CreateSinglePolyData(avtParallelContext &context,
                                                    avtDataTree_p rootnode);
    std::vector<vtkPolyData *> ConvertDatasetsIntoPolyData(vtkDataSet **ds, int nds);
    vtkPolyData               *CombinePolyData(const std::vector<vtkPolyData *> &pds);
    std::vector<vtkPolyData *> SendPolyDataToRank0(avtParallelContext &context,
                                                   const std::vector<vtkPolyData *> &pds);
    void                GroupWrite(const std::string &plotName,
                                   const std::string &filename,
                                   const avtDatabaseMetaData *md,
                                   const std::vector<std::string> &scalarList,
                                   const std::vector<std::string> &vectorList,
                                   const std::vector<std::string> &materialList,
                                   int numTotalChunks, int startIndex,
                                   int tag, bool writeUsingGroups, int groupSize);
    void                WaitForTurn(int tag, int &nWritten);
    void                GrantTurn(int tag, int &nWritten);
};


#endif


