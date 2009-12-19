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
#include "main.h"
#include "GenerateAVT.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if (docType == "Plugin" && plugin->type == "database")
    {
        // avt writer mode
        QFile *ffh;
        if ((ffh = Open(QString("avt") + plugin->name + "FileFormat.h")) != 0)
        {
            QTextStream fh(ffh);
            plugin->WriteFileFormatReaderHeader(fh);
            ffh->close();
            delete ffh;
        }

        QFile *ffc;
        if ((ffc = Open(QString("avt") + plugin->name + "FileFormat.C")) != 0)
        {
            QTextStream fc(ffc);
            plugin->WriteFileFormatReaderSource(fc);
            ffc->close();
            delete ffc;
        }

        if (plugin->haswriter)
        {
            // avt writer mode
            QFile *fwh;
            if ((fwh = Open(QString("avt") + plugin->name + "Writer.h")) != 0)
            {
                QTextStream wh(fwh);
                plugin->WriteFileFormatWriterHeader(wh);
                fwh->close();
                delete fwh;
            }

            QFile *fwc;
            if ((fwc = Open(QString("avt") + plugin->name + "Writer.C")) != 0)
            {
                QTextStream wc(fwc);
                plugin->WriteFileFormatWriterSource(wc);
                fwc->close();
                delete fwc;
            }
        }
        if (plugin->hasoptions)
        {
            // DB options mode.
            QFile *fwh;
            if ((fwh = Open(QString("avt") + plugin->name + "Options.h")) != 0)
            {
                QTextStream wh(fwh);
                plugin->WriteFileFormatOptionsHeader(wh);
                fwh->close();
                delete fwh;
            }

            QFile *fwc;
            if ((fwc = Open(QString("avt") + plugin->name + "Options.C")) != 0)
            {
                QTextStream wc(fwc);
                plugin->WriteFileFormatOptionsSource(wc);
                fwc->close();
                delete fwc;
            }
        }
    }
    else
    {
        // avt filters
        QFile *ffh;
        if ((ffh = Open(QString("avt") + plugin->name + "Filter.h")) != 0)
        {
            QTextStream fh(ffh);
            plugin->WriteFilterHeader(fh);
            ffh->close();
            delete ffh;
        }

        QFile *ffc;
        if ((ffc = Open(QString("avt") + plugin->name + "Filter.C")) != 0)
        {
            QTextStream fc(ffc);
            plugin->WriteFilterSource(fc);
            ffc->close();
            delete ffc;
        }

        if (plugin->type=="plot")
        {
            QFile *fph;
            if ((fph = Open(QString("avt") + plugin->name + "Plot.h")) != 0)
            {
                QTextStream ph(fph);
                plugin->WritePlotHeader(ph);
                fph->close();
                delete fph;
            }

            QFile *fpc;
            if ((fpc = Open(QString("avt") + plugin->name + "Plot.C")) != 0)
            {
                QTextStream pc(fpc);
                plugin->WritePlotSource(pc);
                fpc->close();
                delete fpc;
            }
        }
    }
}

