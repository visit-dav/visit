// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "main.h"
#include "GeneratePython.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if (docType == "Plugin" && plugin->type == "database")
    {
        cErr << "No python to generate for database plugins\n";
    }
    else
    {
        // scripting writer mode
        QString prefix("Py");
        QFile *fh;
        if ((fh = Open("pre_"+prefix+attribute->name+".h")) != 0)
        {
            QTextStream h(fh);

            if (docType == "Plugin")
            {
                plugin->WriteHeader(h);
            }
            else 
            {
                attribute->WriteHeader(h);
            }
            CloseHeader(h, "pre_"+prefix+attribute->name+".h");
        }

        QFile *fs;
        if ((fs = Open(prefix+attribute->name+".C")) != 0)
        {
            QTextStream s(fs);
            attribute->WriteSource(s);
            fs->close();
            delete fs;
        }
    }
}
