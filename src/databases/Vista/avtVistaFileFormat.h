/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtVistaFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Vista_FILE_FORMAT_H
#define AVT_Vista_FILE_FORMAT_H

#include <string>

#include <VisitALE.h>
#include <avtSTMDFileFormat.h>
#include <avtFileFormatInterface.h>

using std::string;

// used in STL maps where we need to control initialization of
// value upon first reference into the map
template<class T, T ival=0>
class IMVal {
  public:
    T val;
    IMVal() : val(ival) {};
};

// ****************************************************************************
//  Class: avtVistaFileFormat
//
//  Purpose:
//      Base class for all Vista file formats. Includes functionality that is
//      NOT specific to any particular Vista file format
//
//      Note that a Vista file can be written by either Silo or HDF5 natively.
//      We support both here. Where there is a choice in API data-types, we
//      favor use of HDF5's data-types.
//
//  Programmer: Mark C. Miller 
//  Creation:   July 14, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jul 21 11:19:15 PDT 2004
//    Totally re-organized to support multiple different Vista file formats
//    Moved most of Vista/Ale3d specific stuff to its respectife file format
//    file.
//
//    Mark C. Miller, Mon Oct 25 17:13:46 PDT 2004
//    Added ReadDataset convenience function to always return float data
//
// ****************************************************************************

class avtVistaFileFormat : public avtSTMDFileFormat
{

  public:

    typedef enum
    {
        FTYPE_ALE3D,
        FTYPE_DIABLO,
        FTYPE_UNKNOWN
    } VistaFormatType;

    typedef enum
    {
        DTYPE_CHAR,
        DTYPE_INT,
        DTYPE_FLOAT,
        DTYPE_DOUBLE,
        DTYPE_UNKNOWN
    } VistaDataType;

    static avtFileFormatInterface *
                               CreateFileFormatInterface(
                                   const char * const *, int);

                               avtVistaFileFormat(const char *,
                                   VistaFormatType _formatType = FTYPE_UNKNOWN);
    virtual                   ~avtVistaFileFormat();

    virtual const char        *GetType(void)   { return "Vista"; };

    VistaFormatType            GetFormatType(void) const
                                   { return formatType; };
    const char *               GetWriterName(void) const
                                   { return writerName; };

    virtual void               FreeUpResources(void); 

    // satisfy avtSTMDFileFormat interface
    virtual vtkDataSet        *GetMesh(int, const char *) { return 0; };
    virtual vtkDataArray      *GetVar(int, const char *) { return 0; };
    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *) {};

  protected:

    class VistaTree
    {

      public:
                         VistaTree(const char *buf, size_t size);
                        ~VistaTree();
          const void     DumpTree() const;
          const Node    *GetNodeFromPath(const Node *root, const char *path) const;
          char          *GetPathFromNode(const Node *root, const Node *node) const;
          void           FindNodes(const Node *root, const char *re, Node ***results,
                                   int *nmatches, RecurseMode rmode) const;
          const Node    *GetTop() const;

      private:
          Node          *top;
          char          *theVistaString;
    };

    VistaTree           *vTree;
    VistaFormatType      formatType;

    static const int     MASTER_FILE_INDEX;
    string               masterFileName;
    string               masterDirName;

    int                  numChunks;
    int                 *chunkToFileMap;

                         avtVistaFileFormat(const char *,
                                            avtVistaFileFormat *morphFrom);

    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             VistaDataType *type, size_t *size, void **buf);
    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             size_t *size, float **buf);

  private:

    // low-level I/O methods
    void                *OpenFile(const char *fileName);
    void                *OpenFile(int fid);
    void                 CloseFile(int fid);

    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             VistaDataType *type, size_t *size, void **buf,
                             bool convertToFloat);

    char                *writerName;

    // we use void * here so we can use either HDF5 or Silo
    void                **fileHandles;

    static int           objcnt;
    bool                 isSilo;

    bool                 wasMorphed;

};


#endif
