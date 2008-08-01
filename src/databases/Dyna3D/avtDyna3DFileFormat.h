/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                            avtDyna3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_DYNA3D_FILE_FORMAT_H
#define AVT_DYNA3D_FILE_FORMAT_H
#include <visitstream.h>
#include <avtSTSDFileFormat.h>
#include <string>
#include <vector>

class vtkFloatArray;

// ****************************************************************************
//  Class: avtDyna3DFileFormat
//
//  Purpose:
//      Reads in Dyna3D files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 14:00:02 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 11:41:18 PDT 2008
//    Added support for material strength.
//
// ****************************************************************************

class avtDyna3DFileFormat : public avtSTSDFileFormat
{
public:
                       avtDyna3DFileFormat(const char *filename);
    virtual           ~avtDyna3DFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);
    
    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual bool      ReturnsValidCycle() const { return true; };
    // virtual int       GetCycle(void);
    // virtual bool      ReturnsValidTime() const { return true; };
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "Dyna3D input data"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    typedef struct
    {
        int   nMaterials;
        int   nPoints;
        int   nSolidHexes;
        int   nBeamElements;
        int   nShellElements4;
        int   nShellElements8;
        int   nInterfaceSegments;
        float interfaceInterval;
        float shellTimestep;
    } Card2_t;
 
    typedef struct
    {
        Card2_t card2;
    } ControlCards_t;

    typedef struct
    {
        int         materialNumber;
        std::string materialName;
        double      density;
        double      strength;
    } MaterialCard_t;

    typedef std::vector<MaterialCard_t> MaterialCardVector;

    void SkipComments(ifstream &ifile, const char *sectionName,
                      bool &, bool &);
    bool SkipToSection(ifstream &ifile, const char *section);
    void GetLine(ifstream &ifile);

    bool ReadControlCards(ifstream &ifile);
    bool ReadControlCard2(ifstream &);
    void ReadControlCard3(ifstream &);
    void ReadControlCard4(ifstream &);
    void ReadControlCard5(ifstream &);
    void ReadControlCard6(ifstream &);
    void ReadControlCard7(ifstream &);
    void ReadControlCard8(ifstream &);
    void ReadControlCard9(ifstream &);
    void ReadMaterialCards(ifstream &);

    bool ReadFile(const char *, int nLines);

    // DATA MEMBERS
    vtkDataSet            *meshDS;
    ControlCards_t         cards;
    MaterialCardVector     materialCards;
    int                   *matNumbers;
    char                  *line;
    vtkFloatArray         *velocity;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
