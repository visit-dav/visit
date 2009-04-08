/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtBOVFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_BOV_FILE_FORMAT_H
#define AVT_BOV_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>


// ****************************************************************************
//  Class: avtBOVFileFormat
//
//  Purpose:
//      Reads "brick of float" files.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Added support for deferring reading in all of the TOC.
//
//    Brad Whitlock, Thu Mar 16 14:54:01 PST 2006
//    Added byte offset so we can skip past a header. I also added a new
//    flag for dividing a brick into bricklets.
//
//    Brad Whitlock, Thu May 4 09:36:53 PDT 2006
//    Added support for double and int.
//
//    Hank Childs, Thu Apr 24 13:29:41 PDT 2008
//    Change char *'s to strings and float *'s to vector<double>.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Convert most int's to long longs to support >4GB files.
//
//    Brad Whitlock, Wed Apr  8 09:40:02 PDT 2009
//    I added short int support.
//
// ****************************************************************************

class avtBOVFileFormat : public avtSTMDFileFormat
{
  public:
                               avtBOVFileFormat(const char *);
    virtual                   ~avtBOVFileFormat();

    virtual const char        *GetType(void) { return "Brick of values"; };

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);
    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { return cycle; };
    virtual bool               ReturnsValidCycle(void) { return haveReadTOC; };
    void                       ActivateTimestep(void);

  protected:
    typedef enum {ByteData, ShortData, IntegerData, FloatData, DoubleData} DataFormatEnum;

    void                       ReadTOC(void);
    void                       ReadWholeAndExtractBrick(void *dest, bool gzipped,
                                  void *file_handle, void *gz_handle,
                                  unsigned long long unit_size,
                                  unsigned long long x_start,
                                  unsigned long long x_stop, 
                                  unsigned long long y_start,
                                  unsigned long long y_stop, 
                                  unsigned long long z_start,
                                  unsigned long long z_stop, 
                                  unsigned long long dx, unsigned long long dy,
                                  unsigned long long whole_size);

    bool                       haveReadTOC;
    char                      *path;
    std::string                file_pattern;
    int                        cycle;
    long long                  full_size[3];
    long long                  bricklet_size[3];
    long long                  byteOffset;
    std::string                varname;
    bool                       hasBoundaries;
    bool                       declaredEndianess;
    bool                       littleEndian;
    bool                       nodalCentering;
    bool                       divideBrick;
    bool                       byteToFloatTransform;
    DataFormatEnum             dataFormat;
    long long                  dataNumComponents;
    double                     min, max;
    double                     origin[3];
    double                     dimensions[3];
    std::vector<double>        var_brick_min;
    std::vector<double>        var_brick_max;
};


#endif


