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

#ifndef AVT_COLORTABLES_H
#define AVT_COLORTABLES_H
#include <plotter_exports.h>
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
// ****************************************************************************

class PLOTTER_API avtColorTables
{
public:
    static avtColorTables *Instance();

    const std::string   &GetDefaultContinuousColorTable() const;
    void                 SetDefaultContinuousColorTable(const std::string &);
    const std::string   &GetDefaultDiscreteColorTable() const;
    void                 SetDefaultDiscreteColorTable(const std::string &);

    const unsigned char *GetColors(const std::string &ctName);

    unsigned char       *GetSampledColors(const std::string &ctName,
                                          int nColors) const;
    bool                 GetControlPointColor(const std::string &ctName,
                                              int i, unsigned char *rgb) const;
    int                  GetNumColors() const { return 256; };
    bool                 IsDiscrete(const std::string &ctName) const;
    bool                 ColorTableExists(const std::string &ctName) const;

    ColorTableAttributes   *GetColorTables() { return ctAtts; }
    void                    SetColorTables(const ColorTableAttributes &);

    bool                 ExportColorTable(const std::string &ctName,
                                          std::string &message);
    void                 ImportColorTables();
protected:
    avtColorTables();
    ~avtColorTables();

    ColorTableAttributes   *ctAtts;
    unsigned char           tmpColors[256*3];

    static avtColorTables  *instance;
};

#endif
