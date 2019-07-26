// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

