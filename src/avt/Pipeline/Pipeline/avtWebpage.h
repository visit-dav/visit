// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_WEBPAGE_H
#define AVT_WEBPAGE_H

#include <pipeline_exports.h>
#include <visitstream.h>


// ****************************************************************************
// Class: avtWebpage
//
// Purpose:
//     Creates webpages that are custom to AVT.
//
// Programmer: Hank Childs
// Creation:   December 21, 2006
//
// Modifications: 
//
//   Hank Childs, Fri Jun 15 15:55:30 PDT 2007
//   Added OnPageLink, AddSectionForLinks, and GetName.
//
//   Hank Childs, Tue Dec 18 10:04:43 PST 2007
//   Define private copy constructor and assignment operator to prevent
//   accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtWebpage
{
  public:
                  avtWebpage(const char *filename);
    virtual      ~avtWebpage();
    
    const char   *GetName(void);

    void          InitializePage(const char *heading);
    void          WriteTitle(const char *title);
    void          FinalizePage(void);

    void          AddLink(const char *, const char *);
    void          AddOnPageLink(const char *, const char *);
    void          AddSectionForLinks(const char *);

    void          AddHeading(const char *);
    void          AddSubheading(const char *);
    void          AddEntry(const char *);
    void          AddSubentry(const char *);
    void          StartTable(void);
    void          AddTableHeader2(const char *, const char *);
    void          AddTableEntry2(const char *, const char *);
    void          AddTableHeader3(const char *, const char *, const char *);
    void          AddTableEntry3(const char *, const char *, const char *);
    void          AddTableHeader4(const char *, const char *, const char *,
                                  const char *);
    void          AddTableEntry4(const char *, const char *, const char *,
                                 const char *);
    void          EndTable(void);

  protected:
    std::string   name;
    ofstream     *ofile;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtWebpage(const avtWebpage &) {;};
    avtWebpage          &operator=(const avtWebpage &) { return *this; };
};


#endif


