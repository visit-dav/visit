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
//                            avtTFTFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_TFT_FILE_FORMAT_H
#define AVT_TFT_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <visitstream.h>
#include <string>
#include <map>

// ****************************************************************************
//  Class: avtTFTFileFormat
//
//  Purpose:
//      Reads in TFT files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:15 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtTFTFileFormat : public avtSTSDFileFormat
{
public:
                           avtTFTFileFormat(const char *filename);
    virtual               ~avtTFTFileFormat();

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual bool           ReturnsValidTime() const;
    virtual double         GetTime(void);

    virtual const char    *GetType(void)   { return "TFT"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

protected:
    void                   Initialize();
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    class CurveData
    {
    public:
                     CurveData();
        virtual     ~CurveData();
        bool         Read(ifstream &, bool &, float &);
        std::string  GetLine(ifstream &);
        std::string  StripExcess(const std::string &input);
        void         GetLabelsAndUnits(const std::string &, std::string &,
                                       std::string &);

        std::string  title;
        std::string  subtitle;
        std::string  xLabel;
        std::string  xUnits;
        std::string  yLabel;
        std::string  yUnits;
        int          nPoints;
        float       *data;
        float        xmin;
        float        xmax;
        float        ymin;
        float        ymax;
    };

    typedef std::map<std::string, CurveData *> CurveDataMap;

    CurveDataMap     variables;
    bool             initialized;
    bool             hasValidTime;
    float            time;
};


#endif
