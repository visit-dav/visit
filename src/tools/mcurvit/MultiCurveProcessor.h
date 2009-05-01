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
#ifndef MULTI_CURVE_PROCESSOR_H
#define MULTI_CURVE_PROCESSOR_H

#include <QObject>

class Synchronizer;
class VisItViewer;

#include <string>
#include <vector>

#include <stdio.h>

using std::string;
using std::vector;

// ****************************************************************************
// Class: MultiCurveProcessor
//
// Purpose:
//   This application creates a multi curve command processor.  It reads a
//   command file, processes the commands, and exits.  It only handles files
//   with 2d rectilinear node centered variables.  It applies an index select
//   operator to the data to allow the user to subselect portions of the
//   data.  It also allows the user to control many of the viewing attributes
//   of the plot.
//
// Notes:
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:41:27 PST 2009
//
// Modifications:
//
// ****************************************************************************

class MultiCurveProcessor : public QObject
{
    Q_OBJECT
public:
    MultiCurveProcessor(VisItViewer *);
    virtual ~MultiCurveProcessor();

    void                   ProcessBatchFile(const char *, const char *);

    void                   OpenDatabase(const string &);
    void                   CreatePlot(const string &);
    void                   FinishPlot();
    void                   SaveWindow(const int, const string &);

private slots:
    void execCommands();
    void execCommand();
private:
    void                   ReadInt(FILE *, int &, int &);
    void                   ReadDouble(FILE *, int &, double &);
    void                   ReadString(FILE *, int &, string &);
    void                   ReadQuotedString(FILE *, int &, string &);
    void                   ReadFile(const char *);

    VisItViewer           *viewer;

    Synchronizer          *sync;

    bool                   save;
    int                    saveFormat;
    string                 openFile;
    string                 saveFile;
    vector<string>         plotVars;

    vector<int>            commandList;
    int                    iCommand;
    int                    iVar;
};

#endif
