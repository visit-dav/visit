/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

#ifndef AVT_COLORTABLES_H
#define AVT_COLORTABLES_H
#include <pipeline_exports.h>
#include <string>

// Forward declarations
class ColorTableAttributes;

// ****************************************************************************
// Class: avtColorTables
//
// Purpose:
//   This class is a singleton that contains all of the color tables that are
//   available to avt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 13:57:27 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:12:49 PST 2002
//   I made it understand discrete color tables. I also removed most of
//   the implementation in favor of using the ColorTableAttributes state
//   object.
//
//   Brad Whitlock, Tue Jul 1 17:20:38 PST 2003
//   I added methods to import and export color tables.
//
//   Brad Whitlock, Thu Nov 13 11:58:13 PDT 2003
//   I changed how the message is passed out of ExportColorTable.
//
//   Jeremy Meredith, Fri Feb 20 14:54:04 EST 2009
//   Added some alpha support methods.
//
//   David Camp, Thu Jan 13 11:03:52 PST 2011
//   Added DeleteInstance function to delete global data. Helps with valgrind.
//   It is inline because it will only be used if you define DEBUG_MEMORY_LEAKS.
//
//    Kathleen Bonnell, Mon Jan 17 11:18:35 MST 2011
//    Added invert arg to color retrieval methods.
//
//    Mark C. Miller, Tue Jun 18 14:28:09 PDT 2019
//    Added color distance methods. The just noticeable threshold was
//    determined emperically by observing printed distance values for colors
//    in the "levels" and "distinct" tables.
// ****************************************************************************

class PIPELINE_API avtColorTables
{
public:
    static avtColorTables *Instance();
    static double          PerceptualColorDistance(unsigned char const *rgbA,
                                                   unsigned char const *rgbB);
    static double          JustNoticeableColorDistance() { return 200.0; };

    const std::string   &GetDefaultContinuousColorTable() const;
    void                 SetDefaultContinuousColorTable(const std::string &);
    const std::string   &GetDefaultDiscreteColorTable() const;
    void                 SetDefaultDiscreteColorTable(const std::string &);

    const unsigned char *GetColors(const std::string &ctName, 
                                   bool invert = false);
    const unsigned char *GetAlphas(const std::string &ctName, 
                                   bool invert = false);
    bool                 ColorTableIsFullyOpaque(const std::string &ctName);

    unsigned char       *GetSampledColors(const std::string &ctName,
                                          int nColors,
                                          bool invert = false) const;
    bool                 GetControlPointColor(const std::string &ctName,
                                              int i, unsigned char *rgb,
                                              bool invert = false) const;
    int                  GetNumColors() const { return 256; };
    bool                 IsDiscrete(const std::string &ctName) const;
    bool                 ColorTableExists(const std::string &ctName) const;

    ColorTableAttributes   *GetColorTables() { return ctAtts; }
    void                    SetColorTables(const ColorTableAttributes &);

    bool                 ExportColorTable(const std::string &ctName,
                                          std::string &message);
    void                 ImportColorTables();

    void                 DeleteInstance()
                         {
                             if(instance) delete instance;
                             instance = NULL;
                         }

protected:
    avtColorTables();
    ~avtColorTables();

    ColorTableAttributes   *ctAtts;
    unsigned char           tmpColors[256*3];
    unsigned char           tmpAlphas[256];

    static avtColorTables  *instance;
};

#endif
