/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtImageFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Image_FILE_FORMAT_H
#define AVT_Image_FILE_FORMAT_H

#include <string>
#include <vector>
#include <database_exports.h>
#include <avtDataSelection.h>
#include <avtSTSDFileFormat.h>
#include <vtkImageData.h>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtImageFileFormat
//
//  Purpose:
//      Reads in Image files as a plugin to VisIt.
// 
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 17:00:40 PST 2004
//    Added support for data selections. Eliminated xdim/ydim data members
//    since they are known from vtkImageData object
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Removed unnused pointvarnames, pointvars. Added fext and CanCacheVariable
//
//    Hank Childs, Fri Mar 18 11:41:04 PST 2005
//    Added support for image volumes.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Deleted a bunch of commented-out code that was hold-over from the
//    plugin generation processes.
//
// ****************************************************************************

class avtImageFileFormat : public avtSTSDFileFormat
{
  public:
                       avtImageFileFormat(const char *filename, DBOptionsAttributes *);
    virtual           ~avtImageFileFormat();

    virtual const char    *GetType(void)   { return "Image"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual bool           CanCacheVariable(const char *);

    virtual void           RegisterDataSelections(
                               const std::vector<avtDataSelection_p> &selList,
                               std::vector<bool> *selectionsApplied);

  protected:
    std::string                          fname;
    std::string                          fext;
    std::string                          image_fext;
    std::vector<std::vector<float> >     cellvars; 
    std::vector<std::string>             cellvarnames;
    vtkImageData                         *image;
    std::vector<avtDataSelection_p>      selList;
    std::vector<bool>                    *selsApplied;
    bool                                 haveInitialized;

    bool                                 haveImageVolume;
    std::vector<std::string>             subImages;
    float                                zStart;
    bool                                 specifiedZStart;
    float                                zStep;
    bool                                 specifiedZStep;

    bool                                 haveReadWholeImage;
    int                                  indexOfImageAlreadyRead;
    int                                  indexOfImageToRead;

    float                                xStart, yStart;
    float                                xStep, yStep;

    void                                 ReadInImage(void);
    void                                 ReadImageVolumeHeader(void);
    void                                 Initialize(void);
    bool                                 ProcessDataSelections(
                                             int *xmin, int *xmax,
                                             int *ymin, int *ymax);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    vtkDataSet                          *GetImageVolumeMesh(const char *);
    vtkDataSet                          *GetOneMesh(const char *);
    vtkDataArray                        *GetImageVolumeVar(const char *);
    vtkDataArray                        *GetOneVar(const char *);
};


#endif
