// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "main.h"
const char *java_copyright_str = 
"// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n"
"// Project developers.  See the top-level LICENSE file for dates and other\n"
"// details.  No copyright assignment is required to contribute to VisIt.\n";

#include "GenerateJava.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if (docType == "Plugin" && plugin->type == "database")
    {
        cErr << "No java to generate for database plugins\n";
    }
    else
    {
        if (docType == "Plugin")
        {
            attribute->pluginVersion = plugin->version;
            attribute->pluginName = plugin->name;
            attribute->pluginType = plugin->type;
        }

        // java atts writer mode
        QFile *fj;
        if ((fj = Open(attribute->name+".java")) != 0)
        {
            QTextStream j(fj);
            attribute->WriteSource(j);
            fj->close();
            delete fj;
        }
    }
}
