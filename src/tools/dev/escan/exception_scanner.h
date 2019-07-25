// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef EXCEPTION_SCANNER_H
#define EXCEPTION_SCANNER_H

void ignoreToken(void);
void processToken(const char *);
void incrementScope(void);
void decrementScope(void);
void advanceSourceLineCount();
void set_filename(const char *fn);
void set_debugflag(int val);
void set_verboseflag(int val);

#endif
