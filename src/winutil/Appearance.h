// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef APPEARANCE_H
#define APPEARANCE_H
#include <winutil_exports.h>

class QApplication;
class AppearanceAttributes;

void WINUTIL_API GetAppearance(QApplication *a, AppearanceAttributes *aa);
void WINUTIL_API SetAppearance(QApplication *a, AppearanceAttributes *aa);

#endif
