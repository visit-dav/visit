// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "main.h"
#include "GenerateSim.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if (docType == "Plugin" && plugin->type == "database")
    {
        cErr << "No attributes to generate for database plugins\n";
    }
    else
    {
        QFile *fc;
        if ((fc = Open(QString("VisIt_%1.h").arg(attribute->name))) != 0)
        {
            QTextStream c(fc);
            attribute->WriteVisItHeader(c);
            fc->close();
            delete fc;
        }

        if ((fc = Open(QString("VisIt_%1.c").arg(attribute->name))) != 0)
        {
            QTextStream c(fc);
            attribute->WriteVisItSource(c);
            fc->close();
            delete fc;
        }

        if ((fc = Open(QString("simv2_%1.h").arg(attribute->name))) != 0)
        {
            QTextStream c(fc);
            attribute->WriteSimV2Header(c);
            fc->close();
            delete fc;
        }


        if ((fc = Open(QString("simv2_%1.C").arg(attribute->name))) != 0)
        {
            QTextStream c(fc);
            attribute->WriteSimV2Source(c);
            fc->close();
            delete fc;
        }
    }
}
