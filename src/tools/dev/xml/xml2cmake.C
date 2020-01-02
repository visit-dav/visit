// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "main.h"
#include "GenerateCMake.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if(attribute == NULL && plugin->type != "database")
        throw "Cannot generate code for this XML file.";

    // cmake writer mode
    QFile *fout;
    if ((fout = Open("CMakeLists.txt")) != 0)
    {
        QTextStream out(fout);
        plugin->WriteCMake(out);
        fout->close();
        delete fout;
    }
}
