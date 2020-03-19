// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "main.h"
#include "GenerateAtts.h"
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
        // atts writer mode
        QFile *fh;
        if ((fh = Open("pre_"+attribute->name+".h")) != 0)
        {
            QTextStream h(fh);
            attribute->WriteHeader(h);
            CloseHeader(h, "pre_"+attribute->name+".h");
        }

        QFile *fc;
        if ((fc = Open(attribute->name+".C")) != 0)
        {
            QTextStream c(fc);
            attribute->WriteSource(c);
            fc->close();
            delete fc;
        }
    }
}
