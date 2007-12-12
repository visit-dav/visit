/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef GENERATE_AVT_H
#define GENERATE_AVT_H

#include "Field.h"
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include <snprintf.h>
#if defined(_WIN32)
#include <windows.h>
#endif

// ****************************************************************************
//  File:  GenerateAVT
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create AVT files (avtPlot, avtFilter) for the plugin.
//
//  Note: This file overrides --
//    Plugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Feb  6 17:26:42 PST 2002
//    Changed references to plugin->name+"Attributes" to instead refer
//    directly to atts->name.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Hank Childs, Thu Sep 12 19:34:31 PDT 2002
//    Add string argument to ExecuteData.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Kathleen Bonnell, Wed Oct 23 18:10:26 PDT 2002  
//    Added new plot method ApplyRenderingTransformation. 
//
//    Jeremy Meredith, Mon Sep 22 14:58:36 PDT 2003
//    Changed an instance where ApplyRenderingTransformation was called
//    ApplyRenderingOperators.  Made haswriter be a bool.
//
//    Jeremy Meredith, Tue Nov  4 15:53:27 PST 2003
//    Added sample avt files for database plugins.  Sample code by Hank Childs.
//
//    Jeremy Meredith, Fri Mar 19 13:43:20 PST 2004
//    Changed MTSD format constructors to pass the correct version of 
//    filename (file list) to the base MTSD constructor.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Thu Aug 12 10:38:06 PDT 2004
//    I removed the include for database_exports.h in the generated code
//    since it was not needed.
//
//    Hank Childs, Mon Feb 14 16:54:29 PST 2005
//    Use EXCEPTION1 macro instead of EXCEPTION0 ('5979) and also add skeleton
//    for expressions and time ('4557).
//
//    Hank Childs, Fri Mar 18 10:50:37 PST 2005
//    Return type of GetAuxiliaryData is void *, not void &.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Moved PopulateDatabaseMetaData method down to format specific classes
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Added hasoptions.
//
//    Hank Childs, Thu Jun  8 16:46:56 PDT 2006
//    Added copyright string.
//
//    Cyrus Harrison, Wed Mar  7 09:46:07 PST 2007
//    Allow for engine-specific code in a plugin's source files
//
//    Hank Childs, Fri May 18 17:46:23 PDT 2007
//    Added argument to OpenFile, since it now takes a "num blocks" argument.
//
//    Hank Childs, Tue May 22 11:17:19 PDT 2007
//    Changed "float *extents" to "double *extents", since that is now the
//    interface.
//
//    Dave Bremer, Wed May 23 16:52:34 PDT 2007
//    Fix minor typo in comment for plugin databases.
//
//    Brad Whitlock, Sat Sep 29 14:49:09 PST 2007
//    Replace getenv("USER") with a safer function so we don't crash on Windows.
//
//    Hank Childs, Tue Nov 20 10:50:49 PST 2007
//    Add include for DBOptionsAttributes.
//
//    Hank Childs, Tue Nov 27 09:10:45 PST 2007
//    Fix type (cyle -> cycle).
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                             Utility Functions
// ----------------------------------------------------------------------------

QString
CurrentTime()
{
    char *tstr[] = {"PDT", "PST"};
    char s1[10], s2[10], s3[10], tmpbuf[200];
    time_t t;
    char *c = NULL;
    int h,m,s,y;
    t = time(NULL);
    c = asctime(localtime(&t));
    // Read the hour.
    sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
    // Reformat the string a little.
    sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
            s1, s2, s3, h, m, s, tstr[h > 12], y);

    return QString(tmpbuf);
}

const char *
UserName()
{
     static char user_buffer[100];
     const char *user = getenv("USER");
     if(user != 0)
         SNPRINTF(user_buffer, 100, "%s -- ", user);
     else
     {
#if defined(_WIN32)
         char tmp[100];
         DWORD maxLen = 100;
         GetUserName((LPTSTR)tmp, (LPDWORD)&maxLen);
         SNPRINTF(user_buffer, 100, "%s -- ", tmp);
#else
         user_buffer[0] = '\0';
#endif
     }
     return user_buffer;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class AVTGeneratorPlugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    bool    haswriter;
    bool    hasoptions;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    hasEngineSpecificCode;

    vector<QString> cxxflags;
    vector<QString> ldflags;
    vector<QString> libs;
    vector<QString> extensions; // for DB plugins
    bool customgfiles;
    vector<QString> gfiles;     // gui
    bool customsfiles;
    vector<QString> sfiles;     // scripting
    bool customvfiles;
    vector<QString> vfiles;     // viewer
    bool custommfiles;
    vector<QString> mfiles;     // mdserver
    bool customefiles;
    vector<QString> efiles;     // engine
    bool customwfiles;
    vector<QString> wfiles;     // widget
    bool customvwfiles;
    vector<QString> vwfiles;    // viewer widget

    Attribute *atts;
  public:
    AVTGeneratorPlugin(const QString &n,const QString &l,const QString &t,const QString &vt,const QString &dt,const QString &v,const QString &, bool hw,bool ho,bool,bool)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), haswriter(hw), hasoptions(ho), atts(NULL)
    {
        enabledByDefault = true;
    }
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
    void WritePlotHeader(ostream &h)
    {
        if (type!="plot")
        {
            cerr << "Must be of type plot!" << endl;
            return;
        }

        h << copyright_str.c_str() << endl;
        h << "// ************************************************************************* //" << endl;
        h << "//                                 avt"<<name<<"Plot.h                             //" << endl;
        h << "// ************************************************************************* //" << endl;
        h << endl;
        h << "#ifndef AVT_"<<name<<"_PLOT_H" << endl;
        h << "#define AVT_"<<name<<"_PLOT_H" << endl;
        h << endl;
        h << endl;
        h << "#include <avtLegend.h>" << endl;
        h << "#include <avtPlot.h>" << endl;
        h << endl;
        h << "#include <"<<atts->name<<".h>" << endl;
        h << endl;
        h << "class     avt"<<name<<"Filter;" << endl;
        h << endl;
        h << endl;
        h << "// ****************************************************************************" << endl;
        h << "//  Class:  avt"<<name<<"Plot" << endl;
        h << "//" << endl;
        h << "//  Purpose:" << endl;
        h << "//      A concrete type of avtPlot, this is the "<<name<<" plot." << endl;
        h << "//" << endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        h << "//  Creation:   "<<CurrentTime()<< endl;
        h << "//" << endl;
        h << "// ****************************************************************************" << endl;
        h << endl;
        h << "YOU SHOULD INHERIT FROM ONE OF THE FOLLOWING PLOT TYPES:" << endl;
        h << "avtImageDataPlot" << endl;
        h << "avtPointDataPlot" << endl;
        h << "avtLineDataPlot" << endl;
        h << "avtSurfaceDataPlot" << endl;
        h << "avtVolumeDataPlot" << endl;
        h << endl;
        h << "class avt"<<name<<"Plot : public avtPointDataPlot" << endl;
        h << "{" << endl;
        h << "  public:" << endl;
        h << "                                avt"<<name<<"Plot();" << endl;
        h << "    virtual                    ~avt"<<name<<"Plot();" << endl;
        h << endl;
        h << "    virtual const char         *GetName(void) { return \""<<name<<"Plot\"; };" << endl;
        h << endl;
        h << "    static avtPlot             *Create();" << endl;
        h << endl;
        h << "    virtual void                SetAtts(const AttributeGroup*);" << endl;
        h << endl;
        h << "  protected:" << endl;
        h << "    "<<atts->name<<"              atts;" << endl;
        h << endl;
        h << "    YOU MUST HAVE SOME SORT OF MAPPER FOR THE PLOT." << endl;
        h << "    avt...Mapper               *myMapper;" << endl;
        h << "    avt"<<name<<"Filter              *"<<name<<"Filter;" << endl;
        h << endl;
        h << "    virtual avtMapper          *GetMapper(void);" << endl;
        h << "    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);" << endl;
        h << "    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);"<< endl;
        h << "    virtual void                CustomizeBehavior(void);" << endl;
        h << "    virtual void                CustomizeMapper(avtDataObjectInformation &);" << endl;
        h << endl;
        h << "    virtual avtLegend_p         GetLegend(void) { return NULL; };" << endl;
        h << "};" << endl;
        h << endl;
        h << endl;
        h << "#endif" << endl;
    }
    void WritePlotSource(ostream &c)
    {
        if (type!="plot")
        {
            cerr << "Must be of type plot!" << endl;
            return;
        }
        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//                             avt"<<name<<"Plot.C                                 //" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <avt"<<name<<"Plot.h>" << endl;
        c << endl;
        c << "#include <avt"<<name<<"Filter.h>" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot constructor" << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avt"<<name<<"Plot::avt"<<name<<"Plot()" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Filter = new avt"<<name<<"Filter(ARGS FOR FILTER);" << endl;
        c << "    myMapper   = ....;" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot destructor" << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avt"<<name<<"Plot::~avt"<<name<<"Plot()" << endl;
        c << "{" << endl;
        c << "    if (myMapper != NULL)" << endl;
        c << "    {" << endl;
        c << "        delete myMapper;" << endl;
        c << "        myMapper = NULL;" << endl;
        c << "    }" << endl;
        c << "    if ("<<name<<"Filter != NULL)" << endl;
        c << "    {" << endl;
        c << "        delete "<<name<<"Filter;" << endl;
        c << "        "<<name<<"Filter = NULL;" << endl;
        c << "    }" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method:  avt"<<name<<"Plot::Create" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Call the constructor." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avtPlot*" << endl;
        c << "avt"<<name<<"Plot::Create()" << endl;
        c << "{" << endl;
        c << "    return new avt"<<name<<"Plot;" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::GetMapper" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Gets a mapper for this plot, it is actually a variable mapper." << endl;
        c << "//" << endl;
        c << "//  Returns:    The variable mapper typed as its base class mapper." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avtMapper *" << endl;
        c << "avt"<<name<<"Plot::GetMapper(void)" << endl;
        c << "{" << endl;
        c << "    return myMapper;" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::ApplyOperators" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Applies the operators associated with a "<<name<<" plot.  " << endl;
        c << "//      The output from this method is a query-able object." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//      input   The input data object." << endl;
        c << "//" << endl;
        c << "//  Returns:    The data object after the "<<name<<" plot has been applied." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avtDataObject_p" << endl;
        c << "avt"<<name<<"Plot::ApplyOperators(avtDataObject_p input)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Filter->SetInput(input);" << endl;
        c << "    return "<<name<<"Filter->GetOutput();" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::ApplyRenderingTransformation" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Applies the rendering transformation associated with a "<<name<<" plot.  " << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//      input   The input data object." << endl;
        c << "//" << endl;
        c << "//  Returns:    The data object after the "<<name<<" plot has been applied." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "avtDataObject_p" << endl;
        c << "avt"<<name<<"Plot::ApplyRenderingTransformation(avtDataObject_p input)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Filter->SetInput(input);" << endl;
        c << "    return "<<name<<"Filter->GetOutput();" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::CustomizeBehavior" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Customizes the behavior as appropriate for a "<<name<<" plot.  This includes" << endl;
        c << "//      behavior like shifting towards or away from the screen." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Plot::CustomizeBehavior(void)" << endl;
        c << "{" << endl;
        c << "    //behavior->SetShiftFactor(0.6);" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::CustomizeMapper" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      A hook from the base class that allows the plot to change its mapper" << endl;
        c << "//      based on the dataset input. " << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//      doi     The data object information." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Plot::CustomizeMapper(avtDataObjectInformation &doi)" << endl;
        c << "{" << endl;
        c << "/* Example of usage." << endl;
        c << "    int dim = doi.GetAttributes().GetCurrentSpatialDimension();" << endl;
        c << "    if (dim == 2)" << endl;
        c << "    {" << endl;
        c << "    }" << endl;
        c << "    else" << endl;
        c << "    {" << endl;
        c << "    }" << endl;
        c << " */" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Plot::SetAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Sets the atts for the "<<name<<" plot." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//      atts    The attributes for this "<<name<<" plot." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Plot::SetAtts(const AttributeGroup *a)" << endl;
        c << "{" << endl;
        c << "    const "<<atts->name<<" *newAtts = (const "<<atts->name<<" *)a;" << endl;
        c << endl;
        c << "    BASED ON ATTRIBUTE VALUES, CHANGE PARAMETERS IN MAPPER AND FILTER." << endl;
        c << "}" << endl;
    }
    void WriteFilterHeader(ostream &h)
    {
        if (type=="operator")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//  File: avt"<<name<<"Filter.h" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef AVT_"<<name<<"_FILTER_H" << endl;
            h << "#define AVT_"<<name<<"_FILTER_H" << endl;
            h << endl;
            h << endl;
            h << "#include <avtPluginStreamer.h>" << endl;
            h << "#include <"<<atts->name<<".h>" << endl;
            h << endl;
            h << endl;
            h << "class vtkDataSet;" << endl;
            h << endl;
            h << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"Filter" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      A plugin operator for "<<name<<"." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;
            h << "class avt"<<name<<"Filter : public avtPluginStreamer" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "                         avt"<<name<<"Filter();" << endl;
            h << "    virtual             ~avt"<<name<<"Filter();" << endl;
            h << endl;
            h << "    static avtFilter    *Create();" << endl;
            h << endl;
            h << "    virtual const char  *GetType(void)  { return \"avt"<<name<<"Filter\"; };" << endl;
            h << "    virtual const char  *GetDescription(void)" << endl;
            h << "                             { return \""<<label<<"\"; };" << endl;
            h << endl;
            h << "    virtual void         SetAtts(const AttributeGroup*);" << endl;
            h << "    virtual bool         Equivalent(const AttributeGroup*);" << endl;
            h << endl;
            h << "  protected:" << endl;
            h << "    "<<atts->name<<"   atts;" << endl;
            h << endl;
            h << "    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);" << endl;
            h << "};" << endl;
            h << endl;
            h << endl;
            h << "#endif" << endl;
        }
        else if (type=="plot")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                              avt"<<name<<"Filter.h                              //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef AVT_"<<name<<"_FILTER_H" << endl;
            h << "#define AVT_"<<name<<"_FILTER_H" << endl;
            h << endl;
            h << endl;
            h << "#include <avtStreamer.h>" << endl;
            h << endl;
            h << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"Filter" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      This operator is the implied operator associated with an "<<name<<" plot." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;
            h << "class avt"<<name<<"Filter : public avtStreamer" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "                              avt"<<name<<"Filter(YOUR INITIALIZATION ARGS);" << endl;
            h << "    virtual                  ~avt"<<name<<"Filter();" << endl;
            h << endl;
            h << "    virtual const char       *GetType(void)   { return \"avt"<<name<<"Filter\"; };" << endl;
            h << "    virtual const char       *GetDescription(void)" << endl;
            h << "                                  { return \"Performing "<<label<<"\"; };" << endl;
            h << endl;
            h << "    ADD THE SET METHODS YOU NEED HERE" << endl;
            h << endl;
            h << "  protected:" << endl;
            h << "    ADD YOUR DATA MEMBERS HERE" << endl;
            h << endl;
            h << "    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);" << endl;
            h << "    virtual void              RefashionDataObjectInfo(void);" << endl;
            h << "};" << endl;
            h << endl;
            h << endl;
            h << "#endif" << endl;
        }
    }
    void WriteFilterSource(ostream &c)
    {
        if (type=="operator")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//  File: avt"<<name<<"Filter.C" << endl;
            c << "// ************************************************************************* //" << endl;
            c << endl;
            c << "#include <avt"<<name<<"Filter.h>" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avt"<<name<<"Filter::avt"<<name<<"Filter()" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter destructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "//  Modifications:" << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avt"<<name<<"Filter::~avt"<<name<<"Filter()" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  avt"<<name<<"Filter::Create" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avtFilter *" << endl;
            c << "avt"<<name<<"Filter::Create()" << endl;
            c << "{" << endl;
            c << "    return new avt"<<name<<"Filter();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:      avt"<<name<<"Filter::SetAtts" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Sets the state of the filter based on the attribute object." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      a        The attributes to use." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void" << endl;
            c << "avt"<<name<<"Filter::SetAtts(const AttributeGroup *a)" << endl;
            c << "{" << endl;
            c << "    atts = *(const "<<atts->name<<"*)a;" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter::Equivalent" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Returns true if creating a new avt"<<name<<"Filter with the given" << endl;
            c << "//      parameters would result in an equivalent avt"<<name<<"Filter." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "bool" << endl;
            c << "avt"<<name<<"Filter::Equivalent(const AttributeGroup *a)" << endl;
            c << "{" << endl;
            c << "    return (atts == *("<<atts->name<<"*)a);" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter::ExecuteData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Sends the specified input and output through the "<<name<<" filter." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      in_ds      The input dataset." << endl;
            c << "//      <unused>   The domain number." << endl;
            c << "//      <unused>   The label." << endl;
            c << "//" << endl;
            c << "//  Returns:       The output dataset." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"Filter::ExecuteData(vtkDataSet *in_ds, int, std::string)" << endl;
            c << "{" << endl;
            c << "    YOUR CODE TO MODIFY THE DATASET GOES HERE" << endl;
            c << "}" << endl;
        }
        else if (type=="plot")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//                              avt"<<name<<"Filter.C                              //" << endl;
            c << "// ************************************************************************* //" << endl;
            c << endl;
            c << "#include <avt"<<name<<"Filter.h>" << endl;
            c << endl;
            c << "#include <vtkDataSet.h>" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avt"<<name<<"Filter::avt"<<name<<"Filter(YOUR INITIALIZERS)" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter destructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avt"<<name<<"Filter::~avt"<<name<<"Filter()" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "YOUR ROUTINES TO SET THE PARAMETERS OF THE FILTERS" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter::ExecuteData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Does the actual VTK code to modify the dataset." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      inDS      The input dataset." << endl;
            c << "//      <unused>  The domain number." << endl;
            c << "//      <unused>  The label." << endl;
            c << "//" << endl;
            c << "//  Returns:      The output dataset." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"Filter::ExecuteData(vtkDataSet *inDS, int, std::string)" << endl;
            c << "{" << endl;
            c << "    THIS IS THE REAL VTK CODE" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"Filter::RefashionDataObjectInfo" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Allows the filter to change its output's data object information, which" << endl;
            c << "//      is a description of the data object." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void" << endl;
            c << "avt"<<name<<"Filter::RefashionDataObjectInfo(void)" << endl;
            c << "{" << endl;
            c << "    IF YOU SEE FUNNY THINGS WITH EXTENTS, ETC, YOU CAN CHANGE THAT HERE." << endl;
            c << "}" << endl;
        }
    }
    void WriteFileFormatReaderHeader(ostream &h)
    {
        if (dbtype == "STSD")
        {
            h << copyright_str.c_str() << endl;
            h << "" << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << "" << endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "" << endl;
            h << "#include <avtSTSDFileFormat.h>" << endl;
            h << "" << endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << endl;
            h << "" << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"FileFormat" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class avt"<<name<<"FileFormat : public avtSTSDFileFormat" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *);" << endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *filename);" << endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // This is used to return unconvention data -- ranging from material" << endl;
            h << "    // information to information about block connectivity." << endl;
            h << "    //" << endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, const char *type," << endl;
            h << "    //                                  void *args, DestructorFunction &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // These are used to declare what the current time and cycle are for the" << endl;
            h << "    // file.  These should only be defined if the file format knows what the" << endl;
            h << "    // time and/or cycle is." << endl;
            h << "    //" << endl;
            h << "    // virtual bool      ReturnsValidCycle() const { return true; };" << endl;
            h << "    // virtual int       GetCycle(void);" << endl;
            h << "    // virtual bool      ReturnsValidTime() const { return true; };" << endl;
            h << "    // virtual double    GetTime(void);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << endl;
            h << "    virtual void           FreeUpResources(void); " << endl;
            h << "" << endl;
            h << "    virtual vtkDataSet    *GetMesh(const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVar(const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVectorVar(const char *);" << endl;
            h << "" << endl;
            h << "  protected:" << endl;
            h << "    // DATA MEMBERS" << endl;
            h << "" << endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "" << endl;
            h << "#endif" << endl;
        }
        else if (dbtype == "MTSD")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << "" << endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "" << endl;
            h << "#include <avtMTSDFileFormat.h>" << endl;
            h << "" << endl;
            h << "#include <vector>" << endl;
            h << "" << endl;
            if (hasoptions)
            {
                h << "class DBOptionsAttributes;" << endl;
                h << "" << endl;
            }
            h << "" << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"FileFormat" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class avt"<<name<<"FileFormat : public avtMTSDFileFormat" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // This is used to return unconvention data -- ranging from material" << endl;
            h << "    // information to information about block connectivity." << endl;
            h << "    //" << endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int timestep, " << endl;
            h << "    //                                     const char *type, void *args, " << endl;
            h << "    //                                     DestructorFunction &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // If you know the times and cycle numbers, overload this function." << endl;
            h << "    // Otherwise, VisIt will make up some reasonable ones for you." << endl;
            h << "    //" << endl;
            h << "    // virtual void        GetCycles(std::vector<int> &);" << endl;
            h << "    // virtual void        GetTimes(std::vector<double> &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    virtual int            GetNTimesteps(void);" << endl;
            h << "" << endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << endl;
            h << "    virtual void           FreeUpResources(void); " << endl;
            h << "" << endl;
            h << "    virtual vtkDataSet    *GetMesh(int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVar(int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, const char *);" << endl;
            h << "" << endl;
            h << "  protected:" << endl;
            h << "    // DATA MEMBERS" << endl;
            h << "" << endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "" << endl;
            h << "#endif" << endl;
        }
        else if (dbtype == "STMD")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << "" << endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "" << endl;
            h << "#include <avtSTMDFileFormat.h>" << endl;
            h << "" << endl;
            h << "#include <vector>" << endl;
            h << "" << endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << endl;
            h << "" << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"FileFormat" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class avt"<<name<<"FileFormat : public avtSTMDFileFormat" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // This is used to return unconvention data -- ranging from material" << endl;
            h << "    // information to information about block connectivity." << endl;
            h << "    //" << endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int domain," << endl;
            h << "    //                                     const char *type, void *args, " << endl;
            h << "    //                                     DestructorFunction &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // If you know the cycle number, overload this function." << endl;
            h << "    // Otherwise, VisIt will make up a reasonable one for you." << endl;
            h << "    //" << endl;
            h << "    // virtual int         GetCycle(void);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << endl;
            h << "    virtual void           FreeUpResources(void); " << endl;
            h << "" << endl;
            h << "    virtual vtkDataSet    *GetMesh(int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVar(int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, const char *);" << endl;
            h << "" << endl;
            h << "  protected:" << endl;
            h << "    // DATA MEMBERS" << endl;
            h << "" << endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "" << endl;
            h << "#endif" << endl;
        }
        else if (dbtype == "MTMD")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << "" << endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << endl;
            h << "" << endl;
            h << "#include <avtMTMDFileFormat.h>" << endl;
            h << "" << endl;
            h << "#include <vector>" << endl;
            h << "" << endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << endl;
            h << "" << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: avt"<<name<<"FileFormat" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class avt"<<name<<"FileFormat : public avtMTMDFileFormat" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // This is used to return unconvention data -- ranging from material" << endl;
            h << "    // information to information about block connectivity." << endl;
            h << "    //" << endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int timestep, " << endl;
            h << "    //                                     int domain, const char *type, void *args, " << endl;
            h << "    //                                     DestructorFunction &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    //" << endl;
            h << "    // If you know the times and cycle numbers, overload this function." << endl;
            h << "    // Otherwise, VisIt will make up some reasonable ones for you." << endl;
            h << "    //" << endl;
            h << "    // virtual void        GetCycles(std::vector<int> &);" << endl;
            h << "    // virtual void        GetTimes(std::vector<double> &);" << endl;
            h << "    //" << endl;
            h << "" << endl;
            h << "    virtual int            GetNTimesteps(void);" << endl;
            h << "" << endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << endl;
            h << "    virtual void           FreeUpResources(void); " << endl;
            h << "" << endl;
            h << "    virtual vtkDataSet    *GetMesh(int, int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVar(int, int, const char *);" << endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, int, const char *);" << endl;
            h << "" << endl;
            h << "  protected:" << endl;
            h << "    // DATA MEMBERS" << endl;
            h << "" << endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "" << endl;
            h << "#endif" << endl;
        }
    }
    void WriteFileFormatReaderSource(ostream &c)
    {
        if (dbtype == "STSD")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << endl;
            c << "// ************************************************************************* //" << endl;
            c << "" << endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << endl;
            c << "" << endl;
            c << "#include <string>" << endl;
            c << "" << endl;
            c << "#include <vtkFloatArray.h>" << endl;
            c << "#include <vtkRectilinearGrid.h>" << endl;
            c << "#include <vtkStructuredGrid.h>" << endl;
            c << "#include <vtkUnstructuredGrid.h>" << endl;
            c << "" << endl;
            c << "#include <avtDatabaseMetaData.h>" << endl;
            c << "" << endl;
            c << "#include <DBOptionsAttributes.h>" << endl;
            c << "#include <Expression.h>" << endl;
            c << "" << endl;
            c << "#include <InvalidVariableException.h>" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "using     std::string;" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << endl;
            c << "    : avtSTSDFileFormat(filename)" << endl;
            c << "{" << endl;
            c << "    // INITIALIZE DATA MEMBERS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << endl;
            c << "//      that." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      This database meta-data object is like a table of contents for the" << endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << endl;
            c << "//      information it can request from you." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)" << endl;
            c << "{" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MESH" << endl;
            c << "    //" << endl;
            c << "    // string meshname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << endl;
            c << "    //" << endl;
            c << "    // int nblocks = 1;  <-- this must be 1 for STSD" << endl;
            c << "    // int block_origin = 0;" << endl;
            c << "    // int spatial_dimension = 2;" << endl;
            c << "    // int topological_dimension = 2;" << endl;
            c << "    // double *extents = NULL;" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << endl;
            c << "    //" << endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << endl;
            c << "    //                   spatial_dimension, topological_dimension);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int vector_dim = 2;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int tensor_dim = 9;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MATERIAL" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string matname = ..." << endl;
            c << "    // int nmats = ...;" << endl;
            c << "    // vector<string> mnames;" << endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //     char str[32];" << endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << endl;
            c << "    //     -- or -- " << endl;
            c << "    //     strcpy(str, \"Aluminum\");" << endl;
            c << "    //     mnames.push_back(str);" << endl;
            c << "    // }" << endl;
            c << "    // " << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << endl;
            c << "    //" << endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the way to add expressions:" << endl;
            c << "    //Expression momentum_expr;" << endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << endl;
            c << "    //md->AddExpression(&momentum_expr);" << endl;
            c << "    //Expression KineticEnergy_expr;" << endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << endl;
            c << "//      vtkUnstructuredGrid, etc)." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << endl;
            c << "//                  there is only one mesh." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"FileFormat::GetMesh(const char *meshname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVar(const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // float *one_entry = new float[ucomps];" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      int j;" << endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << endl;
            c << "    //           one_entry[j] = ..." << endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << endl;
            c << "    //           one_entry[j] = 0.;" << endl;
            c << "    //      rv->SetTuple(i, one_entry); " << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // delete [] one_entry;" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
        }
        else if (dbtype == "MTSD")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << endl;
            c << "// ************************************************************************* //" << endl;
            c << "" << endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << endl;
            c << "" << endl;
            c << "#include <string>" << endl;
            c << "" << endl;
            c << "#include <vtkFloatArray.h>" << endl;
            c << "#include <vtkRectilinearGrid.h>" << endl;
            c << "#include <vtkStructuredGrid.h>" << endl;
            c << "#include <vtkUnstructuredGrid.h>" << endl;
            c << "" << endl;
            c << "#include <avtDatabaseMetaData.h>" << endl;
            c << "" << endl;
            c << "#include <DBOptionsAttributes.h>" << endl;
            c << "#include <Expression.h>" << endl;
            c << "" << endl;
            c << "#include <InvalidVariableException.h>" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "using     std::string;" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << endl;
            c << "    : avtMTSDFileFormat(&filename, 1)" << endl;
            c << "{" << endl;
            c << "    // INITIALIZE DATA MEMBERS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avtEMSTDFileFormat::GetNTimesteps" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Tells the rest of the code how many timesteps there are in this file." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "int" << endl;
            c << "avt"<<name<<"FileFormat::GetNTimesteps(void)" << endl;
            c << "{" << endl;
            c << "    return YOU_MUST_DECIDE;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << endl;
            c << "//      that." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      This database meta-data object is like a table of contents for the" << endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << endl;
            c << "//      information it can request from you." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)" << endl;
            c << "{" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MESH" << endl;
            c << "    //" << endl;
            c << "    // string meshname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << endl;
            c << "    //" << endl;
            c << "    // int nblocks = 1;  <-- this must be 1 for MTSD" << endl;
            c << "    // int block_origin = 0;" << endl;
            c << "    // int spatial_dimension = 2;" << endl;
            c << "    // int topological_dimension = 2;" << endl;
            c << "    // double *extents = NULL;" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << endl;
            c << "    //" << endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << endl;
            c << "    //                   spatial_dimension, topological_dimension);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int vector_dim = 2;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int tensor_dim = 9;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MATERIAL" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string matname = ..." << endl;
            c << "    // int nmats = ...;" << endl;
            c << "    // vector<string> mnames;" << endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //     char str[32];" << endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << endl;
            c << "    //     -- or -- " << endl;
            c << "    //     strcpy(str, \"Aluminum\");" << endl;
            c << "    //     mnames.push_back(str);" << endl;
            c << "    // }" << endl;
            c << "    // " << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << endl;
            c << "    //" << endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the way to add expressions:" << endl;
            c << "    //Expression momentum_expr;" << endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << endl;
            c << "    //md->AddExpression(&momentum_expr);" << endl;
            c << "    //Expression KineticEnergy_expr;" << endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << endl;
            c << "//      vtkUnstructuredGrid, etc)." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate   The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                  'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << endl;
            c << "//                  there is only one mesh." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int timestate, const char *meshname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVar(int timestate, const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int timestate, const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // float *one_entry = new float[ucomps];" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      int j;" << endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << endl;
            c << "    //           one_entry[j] = ..." << endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << endl;
            c << "    //           one_entry[j] = 0.;" << endl;
            c << "    //      rv->SetTuple(i, one_entry); " << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // delete [] one_entry;" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
        }
        else if (dbtype == "STMD")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << endl;
            c << "// ************************************************************************* //" << endl;
            c << "" << endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << endl;
            c << "" << endl;
            c << "#include <string>" << endl;
            c << "" << endl;
            c << "#include <vtkFloatArray.h>" << endl;
            c << "#include <vtkRectilinearGrid.h>" << endl;
            c << "#include <vtkStructuredGrid.h>" << endl;
            c << "#include <vtkUnstructuredGrid.h>" << endl;
            c << "" << endl;
            c << "#include <avtDatabaseMetaData.h>" << endl;
            c << "" << endl;
            c << "#include <DBOptionsAttributes.h>" << endl;
            c << "#include <Expression.h>" << endl;
            c << "" << endl;
            c << "#include <InvalidVariableException.h>" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "using     std::string;" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << endl;
            c << "    : avtSTMDFileFormat(&filename, 1)" << endl;
            c << "{" << endl;
            c << "    // INITIALIZE DATA MEMBERS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << endl;
            c << "//      that." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      This database meta-data object is like a table of contents for the" << endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << endl;
            c << "//      information it can request from you." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)" << endl;
            c << "{" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MESH" << endl;
            c << "    //" << endl;
            c << "    // string meshname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << endl;
            c << "    //" << endl;
            c << "    // int nblocks = YOU_MUST_DECIDE;" << endl;
            c << "    // int block_origin = 0;" << endl;
            c << "    // int spatial_dimension = 2;" << endl;
            c << "    // int topological_dimension = 2;" << endl;
            c << "    // double *extents = NULL;" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << endl;
            c << "    //" << endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << endl;
            c << "    //                   spatial_dimension, topological_dimension);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int vector_dim = 2;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int tensor_dim = 9;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MATERIAL" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string matname = ..." << endl;
            c << "    // int nmats = ...;" << endl;
            c << "    // vector<string> mnames;" << endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //     char str[32];" << endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << endl;
            c << "    //     -- or -- " << endl;
            c << "    //     strcpy(str, \"Aluminum\");" << endl;
            c << "    //     mnames.push_back(str);" << endl;
            c << "    // }" << endl;
            c << "    // " << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << endl;
            c << "    //" << endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the way to add expressions:" << endl;
            c << "    //Expression momentum_expr;" << endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << endl;
            c << "    //md->AddExpression(&momentum_expr);" << endl;
            c << "    //Expression KineticEnergy_expr;" << endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << endl;
            c << "//      vtkUnstructuredGrid, etc)." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      domain      The index of the domain.  If there are NDomains, this" << endl;
            c << "//                  value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                  regardless of block origin." << endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << endl;
            c << "//                  there is only one mesh." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int domain, const char *meshname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                 regardless of block origin." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVar(int domain, const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                 regardless of block origin." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int domain, const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // float *one_entry = new float[ucomps];" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      int j;" << endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << endl;
            c << "    //           one_entry[j] = ..." << endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << endl;
            c << "    //           one_entry[j] = 0.;" << endl;
            c << "    //      rv->SetTuple(i, one_entry); " << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // delete [] one_entry;" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
        }
        else if (dbtype == "MTMD")
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << endl;
            c << "// ************************************************************************* //" << endl;
            c << "" << endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << endl;
            c << "" << endl;
            c << "#include <string>" << endl;
            c << "" << endl;
            c << "#include <vtkFloatArray.h>" << endl;
            c << "#include <vtkRectilinearGrid.h>" << endl;
            c << "#include <vtkStructuredGrid.h>" << endl;
            c << "#include <vtkUnstructuredGrid.h>" << endl;
            c << "" << endl;
            c << "#include <avtDatabaseMetaData.h>" << endl;
            c << "" << endl;
            c << "#include <DBOptionsAttributes.h>" << endl;
            c << "#include <Expression.h>" << endl;
            c << "" << endl;
            c << "#include <InvalidVariableException.h>" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "using     std::string;" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << endl;
            c << "    : avtMTMDFileFormat(filename)" << endl;
            c << "{" << endl;
            c << "    // INITIALIZE DATA MEMBERS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avtEMSTDFileFormat::GetNTimesteps" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Tells the rest of the code how many timesteps there are in this file." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "int" << endl;
            c << "avt"<<name<<"FileFormat::GetNTimesteps(void)" << endl;
            c << "{" << endl;
            c << "    return YOU_MUST_DECIDE;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << endl;
            c << "//      that." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      This database meta-data object is like a table of contents for the" << endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << endl;
            c << "//      information it can request from you." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)" << endl;
            c << "{" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MESH" << endl;
            c << "    //" << endl;
            c << "    // string meshname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << endl;
            c << "    //" << endl;
            c << "    // int nblocks = YOU_MUST_DECIDE;" << endl;
            c << "    // int block_origin = 0;" << endl;
            c << "    // int spatial_dimension = 2;" << endl;
            c << "    // int topological_dimension = 2;" << endl;
            c << "    // double *extents = NULL;" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << endl;
            c << "    //" << endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << endl;
            c << "    //                   spatial_dimension, topological_dimension);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int vector_dim = 2;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string varname = ..." << endl;
            c << "    // int tensor_dim = 9;" << endl;
            c << "    //" << endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << endl;
            c << "    //" << endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // CODE TO ADD A MATERIAL" << endl;
            c << "    //" << endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << endl;
            c << "    // string matname = ..." << endl;
            c << "    // int nmats = ...;" << endl;
            c << "    // vector<string> mnames;" << endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //     char str[32];" << endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << endl;
            c << "    //     -- or -- " << endl;
            c << "    //     strcpy(str, \"Aluminum\");" << endl;
            c << "    //     mnames.push_back(str);" << endl;
            c << "    // }" << endl;
            c << "    // " << endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << endl;
            c << "    //" << endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << endl;
            c << "    //" << endl;
            c << "    //" << endl;
            c << "    // Here's the way to add expressions:" << endl;
            c << "    //Expression momentum_expr;" << endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << endl;
            c << "    //md->AddExpression(&momentum_expr);" << endl;
            c << "    //Expression KineticEnergy_expr;" << endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << endl;
            c << "//      vtkUnstructuredGrid, etc)." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate   The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                  'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      domain      The index of the domain.  If there are NDomains, this" << endl;
            c << "//                  value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                  regardless of block origin." << endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << endl;
            c << "//                  there is only one mesh." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataSet *" << endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int timestate, int domain, const char *meshname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                 regardless of block origin." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVar(int timestate, int domain, const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << endl;
            c << "//      that is supported everywhere through VisIt." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << endl;
            c << "//                 regardless of block origin." << endl;
            c << "//      varname    The name of the variable requested." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "vtkDataArray *" << endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int timestate, int domain,const char *varname)" << endl;
            c << "{" << endl;
            c << "    YOU MUST IMPLEMENT THIS" << endl;
            c << "    //" << endl;
            c << "    // If you have a file format where variables don't apply (for example a" << endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << endl;
            c << "    // then uncomment the code below." << endl;
            c << "    //" << endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << endl;
            c << "    //" << endl;
            c << "" << endl;
            c << "    //" << endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << endl;
            c << "    //" << endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << endl;
            c << "    // float *one_entry = new float[ucomps];" << endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << endl;
            c << "    // {" << endl;
            c << "    //      int j;" << endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << endl;
            c << "    //           one_entry[j] = ..." << endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << endl;
            c << "    //           one_entry[j] = 0.;" << endl;
            c << "    //      rv->SetTuple(i, one_entry); " << endl;
            c << "    // }" << endl;
            c << "    //" << endl;
            c << "    // delete [] one_entry;" << endl;
            c << "    // return rv;" << endl;
            c << "    //" << endl;
            c << "}" << endl;
        }
    }
    void WriteFileFormatOptionsHeader(ostream &h)
    {
        h << copyright_str.c_str() << endl;
        h << "// ************************************************************************* //" << endl;
        h << "//                             avt"<<name<<"Options.h                              //" << endl;
        h << "// ************************************************************************* //" << endl;
        h << "" << endl;
        h << "#ifndef AVT_"<<name<<"_OPTIONS_H" << endl;
        h << "#define AVT_"<<name<<"_OPTIONS_H" << endl;
        h << "" << endl;
        h << "class DBOptionsAttributes;" << endl;
        h << "" << endl;
        h << "#include <string>" << endl;
        h << "" << endl;
        h << "" << endl;
        h << "// ****************************************************************************" << endl;
        h << "//  Functions: avt"<<name<<"Options" << endl;
        h << "//" << endl;
        h << "//  Purpose:" << endl;
        h << "//      Creates the options for  "<<name<<" readers and/or writers." << endl;
        h << "//" << endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        h << "//  Creation:   "<<CurrentTime()<< endl;
        h << "//" << endl;
        h << "// ****************************************************************************" << endl;
        h << "" << endl;
        h << "DBOptionsAttributes *Get"<<name<<"ReadOptions(void);" << endl;
        h << "DBOptionsAttributes *Get"<<name<<"WriteOptions(void);" << endl;
        h << "" << endl;
        h << "" << endl;
        h << "#endif" << endl;
    }
    void WriteFileFormatOptionsSource(ostream &h)
    {
        h << copyright_str.c_str() << endl;
        h << "// ************************************************************************* //" << endl;
        h << "//                             avt"<<name<<"Options.C                              //" << endl;
        h << "// ************************************************************************* //" << endl;
        h << "" << endl;
        h << "#include <avt"<<name<<"Options.h>" << endl;
        h << "" << endl;
        h << "#include <DBOptionsAttributes.h>" << endl;
        h << "" << endl;
        h << "#include <string>" << endl;
        h << "" << endl;
        h << "" << endl;
        h << "// ****************************************************************************" << endl;
        h << "//  Function: Get"<<name<<"ReadOptions" << endl;
        h << "//" << endl;
        h << "//  Purpose:" << endl;
        h << "//      Creates the options for "<<name<<" readers." << endl;
        h << "//" << endl;
        h << "//  Important Note:" << endl;
        h << "//      The code below sets up empty options.  If your format "
          << endl;
        h << "//      does not require read options, no modifications are "
          << endl;
        h << "//      necessary." << endl;
        h << "//" << endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        h << "//  Creation:   "<<CurrentTime()<< endl;
        h << "//" << endl;
        h << "// ****************************************************************************" << endl;
        h << "" << endl;
        h << "DBOptionsAttributes *" << endl;
        h << "Get"<<name<<"ReadOptions(void)" << endl;
        h << "{" << endl;
        h << "    DBOptionsAttributes *rv = new DBOptionsAttributes;" << endl;
        h << "    return rv;" << endl;
        h << "/* EXAMPLE OF OPTIONS" << endl;
        h << "    rv->SetBool(\"Binary format\", true);" << endl;
        h << "    rv->SetBool(\"Big Endian\", false);" << endl;
        h << "    rv->SetEnum(\"Dimension\", 1);" << endl;
        h << "    vector<string> dims;" << endl;
        h << "    dims.push_back(\"0D\");" << endl;
        h << "    dims.push_back(\"1D\");" << endl;
        h << "    dims.push_back(\"2D\");" << endl;
        h << "    dims.push_back(\"3D\");" << endl;
        h << "    rv->SetEnumStrings(2, dims);" << endl;
        h << "    rv->SetInt(\"Number of variables\", 5);" << endl;
        h << "    rv->SetString(\"Name of auxiliary file\", "");" << endl;
        h << "    rv->SetDouble(\"Displacement factor\", 1.0);" << endl;
        h << "" << endl;
        h << "    // When reading or writing the file, you can get the options out of this object like:" << endl;
        h << "    rv->GetDouble(\"Displacement factor\");" << endl;
        h << "*/" << endl;
        h << "}" << endl;
        h << "" << endl;
        h << "" << endl;
        h << "// ****************************************************************************" << endl;
        h << "//  Function: Get"<<name<<"WriteOptions" << endl;
        h << "//" << endl;
        h << "//  Purpose:" << endl;
        h << "//      Creates the options for "<<name<<" writers." << endl;
        h << "//" << endl;
        h << "//  Important Note:" << endl;
        h << "//      The code below sets up empty options.  If your format "
          << endl;
        h << "//      does not require write options, no modifications are "
          << endl;
        h << "//      necessary." << endl;
        h << "//" << endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        h << "//  Creation:   "<<CurrentTime()<< endl;
        h << "//" << endl;
        h << "// ****************************************************************************" << endl;
        h << "" << endl;
        h << "DBOptionsAttributes *" << endl;
        h << "Get"<<name<<"WriteOptions(void)" << endl;
        h << "{" << endl;
        h << "    DBOptionsAttributes *rv = new DBOptionsAttributes;" << endl;
        h << "    return rv;" << endl;
        h << "}" << endl;
    }
    void WriteFileFormatWriterHeader(ostream &h)
    {
        h << copyright_str.c_str() << endl;
        h << "// ************************************************************************* //" << endl;
        h << "//                             avt"<<name<<"Writer.h                              //" << endl;
        h << "// ************************************************************************* //" << endl;
        h << "" << endl;
        h << "#ifndef AVT_"<<name<<"_WRITER_H" << endl;
        h << "#define AVT_"<<name<<"_WRITER_H" << endl;
        h << "" << endl;
        h << "#include <avtDatabaseWriter.h>" << endl;
        h << "" << endl;
        h << "#include <string>" << endl;
        h << "" << endl;
        if (hasoptions)
            h << "class DBOptionsAttributes;" << endl;
        h << "" << endl;
        h << "// ****************************************************************************" << endl;
        h << "//  Class: avt"<<name<<"Writer" << endl;
        h << "//" << endl;
        h << "//  Purpose:" << endl;
        h << "//      A module that writes out "<<name<<" files." << endl;
        h << "//" << endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        h << "//  Creation:   "<<CurrentTime()<< endl;
        h << "//" << endl;
        h << "// ****************************************************************************" << endl;
        h << "" << endl;
        h << "class" << endl;
        h << "avt"<<name<<"Writer : public avtDatabaseWriter" << endl;
        h << "{" << endl;
        h << "  public:" << endl;
        if (hasoptions)
            h << "                   avt"<<name<<"Writer(DBOptionsAttributes *);" << endl;
        else
            h << "                   avt"<<name<<"Writer();" << endl;
        h << "    virtual       ~avt"<<name<<"Writer() {;};" << endl;
        h << "" << endl;
        h << "  protected:" << endl;
        h << "    std::string    stem;" << endl;
        h << "" << endl;
        h << "    virtual void   OpenFile(const std::string &, int nb);" << endl;
        h << "    virtual void   WriteHeaders(const avtDatabaseMetaData *," << endl;
        h << "                                std::vector<std::string> &, " << endl;
        h << "                                std::vector<std::string> &," << endl;
        h << "                                std::vector<std::string> &);" << endl;
        h << "    virtual void   WriteChunk(vtkDataSet *, int);" << endl;
        h << "    virtual void   CloseFile(void);" << endl;
        h << "};" << endl;
        h << "" << endl;
        h << "" << endl;
        h << "#endif" << endl;
    }
    void WriteFileFormatWriterSource(ostream &c)
    {
        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//                              avt"<<name<<"Writer.C                             //" << endl;
        c << "// ************************************************************************* //" << endl;
        c << "" << endl;
        c << "#include <avt"<<name<<"Writer.h>" << endl;
        c << "" << endl;
        c << "#include <vector>" << endl;
        c << "" << endl;
        c << "#include <vtkDataSetWriter.h>" << endl;
        c << "" << endl;
        c << "#include <avtDatabaseMetaData.h>" << endl;
        if (hasoptions)
            c << "#include <DBOptionsAttributes.h>" << endl;
        c << "" << endl;
        c << "using     std::string;" << endl;
        c << "using     std::vector;" << endl;
        c << "" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Writer constructor" << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        if (hasoptions)
            c << "avt"<<name<<"Writer::avt"<<name<<"Writer(DBOptionsAttributes *)" << endl;
        else
            c << "avt"<<name<<"Writer::avt"<<name<<"Writer(void)" << endl;
        c << "{" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Writer::OpenFile" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Does no actual work.  Just records the stem name for the files." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Writer::OpenFile(const string &stemname, int numblocks)" << endl;
        c << "{" << endl;
        c << "    stem = stemname;" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Writer::WriteHeaders" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Writes out a VisIt file to tie the "<<name<<" files together." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Writer::WriteHeaders(const avtDatabaseMetaData *md," << endl;
        c << "                           vector<string> &scalars, vector<string> &vectors," << endl;
        c << "                           vector<string> &materials)" << endl;
        c << "{" << endl;
        c << "    // WRITE OUT HEADER INFO" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Writer::WriteChunk" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      This writes out one chunk of an avtDataset." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Writer::WriteChunk(vtkDataSet *ds, int chunk)" << endl;
        c << "{" << endl;
        c << "    // WRITE OUT THIS ONE CHUNK" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: avt"<<name<<"Writer::CloseFile" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//      Closes the file.  This does nothing in this case." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void" << endl;
        c << "avt"<<name<<"Writer::CloseFile(void)" << endl;
        c << "{" << endl;
        c << "    // CLOSE FILES" << endl;
        c << "}" << endl;
    }
    
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       AVTGeneratorPlugin

#endif
