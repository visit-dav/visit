/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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

#include <QTextStream>
#include "Field.h"
#include "PluginBase.h"

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
//    Hank Childs, Thu Jan 10 14:33:30 PST 2008
//    Added filenames, specifiedFilenames.
//
//    Jeremy Meredith, Thu Jan 24 15:37:47 EST 2008
//    Changed the example for EnumStrings in the options to match the
//    current interface.
//
//    Brad Whitlock, Wed Mar 5 11:32:08 PDT 2008
//    Made it use a base class.
//
// ****************************************************************************

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class AVTGeneratorPlugin : public PluginBase
{
  public:
    Attribute *atts;
    QString    Endl;
  public:
    AVTGeneratorPlugin(const QString &n,const QString &l,const QString &t,
          const QString &vt,const QString &dt,const QString &v,const QString &ifile,
          bool hw,bool ho,bool onlyengine,bool noengine)
        : PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), atts(NULL), Endl("\n")
    {
    }
    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< Endl;
        if (atts)
            atts->Print(out);
    }
    void WritePlotHeader(QTextStream &h)
    {
        if (type!="plot")
        {
            cErr << "Must be of type plot!" << Endl;
            return;
        }

        h << copyright_str << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "//                                 avt"<<name<<"Plot.h                             //" << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << Endl;
        h << "#ifndef AVT_"<<name<<"_PLOT_H" << Endl;
        h << "#define AVT_"<<name<<"_PLOT_H" << Endl;
        h << Endl;
        h << Endl;
        h << "#include <avtLegend.h>" << Endl;
        h << "#include <avtPlot.h>" << Endl;
        h << Endl;
        h << "#include <"<<atts->name<<".h>" << Endl;
        h << Endl;
        h << "class     avt"<<name<<"Filter;" << Endl;
        h << Endl;
        h << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "//  Class:  avt"<<name<<"Plot" << Endl;
        h << "//" << Endl;
        h << "//  Purpose:" << Endl;
        h << "//      A concrete type of avtPlot, this is the "<<name<<" plot." << Endl;
        h << "//" << Endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        h << "//  Creation:   "<<CurrentTime()<< Endl;
        h << "//" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << Endl;
        h << "YOU SHOULD INHERIT FROM ONE OF THE FOLLOWING PLOT TYPES:" << Endl;
        h << "avtImageDataPlot" << Endl;
        h << "avtPointDataPlot" << Endl;
        h << "avtLineDataPlot" << Endl;
        h << "avtSurfaceDataPlot" << Endl;
        h << "avtVolumeDataPlot" << Endl;
        h << Endl;
        h << "class avt"<<name<<"Plot : public avtPointDataPlot" << Endl;
        h << "{" << Endl;
        h << "  public:" << Endl;
        h << "                                avt"<<name<<"Plot();" << Endl;
        h << "    virtual                    ~avt"<<name<<"Plot();" << Endl;
        h << Endl;
        h << "    virtual const char         *GetName(void) { return \""<<name<<"Plot\"; };" << Endl;
        h << Endl;
        h << "    static avtPlot             *Create();" << Endl;
        h << Endl;
        h << "    virtual void                SetAtts(const AttributeGroup*);" << Endl;
        h << Endl;
        h << "  protected:" << Endl;
        h << "    "<<atts->name<<"              atts;" << Endl;
        h << Endl;
        h << "    YOU MUST HAVE SOME SORT OF MAPPER FOR THE PLOT." << Endl;
        h << "    avt...Mapper               *myMapper;" << Endl;
        h << "    avt"<<name<<"Filter              *"<<name<<"Filter;" << Endl;
        h << Endl;
        h << "    virtual avtMapper          *GetMapper(void);" << Endl;
        h << "    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);" << Endl;
        h << "    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);"<< Endl;
        h << "    virtual void                CustomizeBehavior(void);" << Endl;
        h << "    virtual void                CustomizeMapper(avtDataObjectInformation &);" << Endl;
        h << Endl;
        h << "    virtual avtLegend_p         GetLegend(void) { return NULL; };" << Endl;
        h << "};" << Endl;
        h << Endl;
        h << Endl;
        h << "#endif" << Endl;
    }
    void WritePlotSource(QTextStream &c)
    {
        if (type!="plot")
        {
            cErr << "Must be of type plot!" << Endl;
            return;
        }
        c << copyright_str << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << "//                             avt"<<name<<"Plot.C                                 //" << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << Endl;
        c << "#include <avt"<<name<<"Plot.h>" << Endl;
        c << Endl;
        c << "#include <avt"<<name<<"Filter.h>" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot constructor" << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avt"<<name<<"Plot::avt"<<name<<"Plot()" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Filter = new avt"<<name<<"Filter(ARGS FOR FILTER);" << Endl;
        c << "    myMapper   = ....;" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot destructor" << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avt"<<name<<"Plot::~avt"<<name<<"Plot()" << Endl;
        c << "{" << Endl;
        c << "    if (myMapper != NULL)" << Endl;
        c << "    {" << Endl;
        c << "        delete myMapper;" << Endl;
        c << "        myMapper = NULL;" << Endl;
        c << "    }" << Endl;
        c << "    if ("<<name<<"Filter != NULL)" << Endl;
        c << "    {" << Endl;
        c << "        delete "<<name<<"Filter;" << Endl;
        c << "        "<<name<<"Filter = NULL;" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method:  avt"<<name<<"Plot::Create" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//    Call the constructor." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avtPlot*" << Endl;
        c << "avt"<<name<<"Plot::Create()" << Endl;
        c << "{" << Endl;
        c << "    return new avt"<<name<<"Plot;" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::GetMapper" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Gets a mapper for this plot, it is actually a variable mapper." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    The variable mapper typed as its base class mapper." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avtMapper *" << Endl;
        c << "avt"<<name<<"Plot::GetMapper(void)" << Endl;
        c << "{" << Endl;
        c << "    return myMapper;" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::ApplyOperators" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Applies the operators associated with a "<<name<<" plot.  " << Endl;
        c << "//      The output from this method is a query-able object." << Endl;
        c << "//" << Endl;
        c << "//  Arguments:" << Endl;
        c << "//      input   The input data object." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    The data object after the "<<name<<" plot has been applied." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avtDataObject_p" << Endl;
        c << "avt"<<name<<"Plot::ApplyOperators(avtDataObject_p input)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Filter->SetInput(input);" << Endl;
        c << "    return "<<name<<"Filter->GetOutput();" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::ApplyRenderingTransformation" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Applies the rendering transformation associated with a "<<name<<" plot.  " << Endl;
        c << "//" << Endl;
        c << "//  Arguments:" << Endl;
        c << "//      input   The input data object." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    The data object after the "<<name<<" plot has been applied." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "avtDataObject_p" << Endl;
        c << "avt"<<name<<"Plot::ApplyRenderingTransformation(avtDataObject_p input)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Filter->SetInput(input);" << Endl;
        c << "    return "<<name<<"Filter->GetOutput();" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::CustomizeBehavior" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Customizes the behavior as appropriate for a "<<name<<" plot.  This includes" << Endl;
        c << "//      behavior like shifting towards or away from the screen." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Plot::CustomizeBehavior(void)" << Endl;
        c << "{" << Endl;
        c << "    //behavior->SetShiftFactor(0.6);" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::CustomizeMapper" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      A hook from the base class that allows the plot to change its mapper" << Endl;
        c << "//      based on the dataset input. " << Endl;
        c << "//" << Endl;
        c << "//  Arguments:" << Endl;
        c << "//      doi     The data object information." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Plot::CustomizeMapper(avtDataObjectInformation &doi)" << Endl;
        c << "{" << Endl;
        c << "/* Example of usage." << Endl;
        c << "    int dim = doi.GetAttributes().GetCurrentSpatialDimension();" << Endl;
        c << "    if (dim == 2)" << Endl;
        c << "    {" << Endl;
        c << "    }" << Endl;
        c << "    else" << Endl;
        c << "    {" << Endl;
        c << "    }" << Endl;
        c << " */" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Plot::SetAtts" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Sets the atts for the "<<name<<" plot." << Endl;
        c << "//" << Endl;
        c << "//  Arguments:" << Endl;
        c << "//      atts    The attributes for this "<<name<<" plot." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Plot::SetAtts(const AttributeGroup *a)" << Endl;
        c << "{" << Endl;
        c << "    const "<<atts->name<<" *newAtts = (const "<<atts->name<<" *)a;" << Endl;
        c << Endl;
        c << "    BASED ON ATTRIBUTE VALUES, CHANGE PARAMETERS IN MAPPER AND FILTER." << Endl;
        c << "}" << Endl;
    }
    void WriteFilterHeader(QTextStream &h)
    {
        if (type=="operator")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//  File: avt"<<name<<"Filter.h" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << Endl;
            h << "#ifndef AVT_"<<name<<"_FILTER_H" << Endl;
            h << "#define AVT_"<<name<<"_FILTER_H" << Endl;
            h << Endl;
            h << Endl;
            h << "#include <avtPluginDataTreeIterator.h>" << Endl;
            h << "#include <"<<atts->name<<".h>" << Endl;
            h << Endl;
            h << Endl;
            h << "class vtkDataSet;" << Endl;
            h << Endl;
            h << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"Filter" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      A plugin operator for "<<name<<"." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << Endl;
            h << "class avt"<<name<<"Filter : public avtPluginDataTreeIterator" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "                         avt"<<name<<"Filter();" << Endl;
            h << "    virtual             ~avt"<<name<<"Filter();" << Endl;
            h << Endl;
            h << "    static avtFilter    *Create();" << Endl;
            h << Endl;
            h << "    virtual const char  *GetType(void)  { return \"avt"<<name<<"Filter\"; };" << Endl;
            h << "    virtual const char  *GetDescription(void)" << Endl;
            h << "                             { return \""<<label<<"\"; };" << Endl;
            h << Endl;
            h << "    virtual void         SetAtts(const AttributeGroup*);" << Endl;
            h << "    virtual bool         Equivalent(const AttributeGroup*);" << Endl;
            h << Endl;
            h << "  protected:" << Endl;
            h << "    "<<atts->name<<"   atts;" << Endl;
            h << Endl;
            h << "    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);" << Endl;
            h << "};" << Endl;
            h << Endl;
            h << Endl;
            h << "#endif" << Endl;
        }
        else if (type=="plot")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//                              avt"<<name<<"Filter.h                              //" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << Endl;
            h << "#ifndef AVT_"<<name<<"_FILTER_H" << Endl;
            h << "#define AVT_"<<name<<"_FILTER_H" << Endl;
            h << Endl;
            h << Endl;
            h << "#include <avtDataTreeIterator.h>" << Endl;
            h << Endl;
            h << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"Filter" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      This operator is the implied operator associated with an "<<name<<" plot." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << Endl;
            h << "class avt"<<name<<"Filter : public avtDataTreeIterator" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "                              avt"<<name<<"Filter(YOUR INITIALIZATION ARGS);" << Endl;
            h << "    virtual                  ~avt"<<name<<"Filter();" << Endl;
            h << Endl;
            h << "    virtual const char       *GetType(void)   { return \"avt"<<name<<"Filter\"; };" << Endl;
            h << "    virtual const char       *GetDescription(void)" << Endl;
            h << "                                  { return \"Performing "<<label<<"\"; };" << Endl;
            h << Endl;
            h << "    ADD THE SET METHODS YOU NEED HERE" << Endl;
            h << Endl;
            h << "  protected:" << Endl;
            h << "    ADD YOUR DATA MEMBERS HERE" << Endl;
            h << Endl;
            h << "    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);" << Endl;
            h << "    virtual void              UpdateDataObjectInfo(void);" << Endl;
            h << "};" << Endl;
            h << Endl;
            h << Endl;
            h << "#endif" << Endl;
        }
    }
    void WriteFilterSource(QTextStream &c)
    {
        if (type=="operator")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//  File: avt"<<name<<"Filter.C" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << Endl;
            c << "#include <avt"<<name<<"Filter.h>" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "avt"<<name<<"Filter::avt"<<name<<"Filter()" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter destructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "//  Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "avt"<<name<<"Filter::~avt"<<name<<"Filter()" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:  avt"<<name<<"Filter::Create" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "avtFilter *" << Endl;
            c << "avt"<<name<<"Filter::Create()" << Endl;
            c << "{" << Endl;
            c << "    return new avt"<<name<<"Filter();" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:      avt"<<name<<"Filter::SetAtts" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Sets the state of the filter based on the attribute object." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      a        The attributes to use." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"Filter::SetAtts(const AttributeGroup *a)" << Endl;
            c << "{" << Endl;
            c << "    atts = *(const "<<atts->name<<"*)a;" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter::Equivalent" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Returns true if creating a new avt"<<name<<"Filter with the given" << Endl;
            c << "//      parameters would result in an equivalent avt"<<name<<"Filter." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "bool" << Endl;
            c << "avt"<<name<<"Filter::Equivalent(const AttributeGroup *a)" << Endl;
            c << "{" << Endl;
            c << "    return (atts == *("<<atts->name<<"*)a);" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter::ExecuteData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Sends the specified input and output through the "<<name<<" filter." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      in_ds      The input dataset." << Endl;
            c << "//      <unused>   The domain number." << Endl;
            c << "//      <unused>   The label." << Endl;
            c << "//" << Endl;
            c << "//  Returns:       The output dataset." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"Filter::ExecuteData(vtkDataSet *in_ds, int, std::string)" << Endl;
            c << "{" << Endl;
            c << "    YOUR CODE TO MODIFY THE DATASET GOES HERE" << Endl;
            c << "}" << Endl;
        }
        else if (type=="plot")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//                              avt"<<name<<"Filter.C                              //" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << Endl;
            c << "#include <avt"<<name<<"Filter.h>" << Endl;
            c << Endl;
            c << "#include <vtkDataSet.h>" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "avt"<<name<<"Filter::avt"<<name<<"Filter(YOUR INITIALIZERS)" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter destructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "avt"<<name<<"Filter::~avt"<<name<<"Filter()" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "YOUR ROUTINES TO SET THE PARAMETERS OF THE FILTERS" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter::ExecuteData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Does the actual VTK code to modify the dataset." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      inDS      The input dataset." << Endl;
            c << "//      <unused>  The domain number." << Endl;
            c << "//      <unused>  The label." << Endl;
            c << "//" << Endl;
            c << "//  Returns:      The output dataset." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"Filter::ExecuteData(vtkDataSet *inDS, int, std::string)" << Endl;
            c << "{" << Endl;
            c << "    THIS IS THE REAL VTK CODE" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"Filter::UpdateDataObjectInfo" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Allows the filter to change its output's data object information, which" << Endl;
            c << "//      is a description of the data object." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"Filter::UpdateDataObjectInfo(void)" << Endl;
            c << "{" << Endl;
            c << "    IF YOU SEE FUNNY THINGS WITH EXTENTS, ETC, YOU CAN CHANGE THAT HERE." << Endl;
            c << "}" << Endl;
        }
    }
    void WriteFileFormatReaderHeader(QTextStream &h)
    {
        if (dbtype == "STSD")
        {
            h << copyright_str << Endl;
            h << "" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "" << Endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "" << Endl;
            h << "#include <avtSTSDFileFormat.h>" << Endl;
            h << "" << Endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << Endl;
            h << "" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"FileFormat" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "class avt"<<name<<"FileFormat : public avtSTSDFileFormat" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *);" << Endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *filename);" << Endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // This is used to return unconvention data -- ranging from material" << Endl;
            h << "    // information to information about block connectivity." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, const char *type," << Endl;
            h << "    //                                  void *args, DestructorFunction &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // These are used to declare what the current time and cycle are for the" << Endl;
            h << "    // file.  These should only be defined if the file format knows what the" << Endl;
            h << "    // time and/or cycle is." << Endl;
            h << "    //" << Endl;
            h << "    // virtual bool      ReturnsValidCycle() const { return true; };" << Endl;
            h << "    // virtual int       GetCycle(void);" << Endl;
            h << "    // virtual bool      ReturnsValidTime() const { return true; };" << Endl;
            h << "    // virtual double    GetTime(void);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << Endl;
            h << "    virtual void           FreeUpResources(void); " << Endl;
            h << "" << Endl;
            h << "    virtual vtkDataSet    *GetMesh(const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVar(const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVectorVar(const char *);" << Endl;
            h << "" << Endl;
            h << "  protected:" << Endl;
            h << "    // DATA MEMBERS" << Endl;
            h << "" << Endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);" << Endl;
            h << "};" << Endl;
            h << "" << Endl;
            h << "" << Endl;
            h << "#endif" << Endl;
        }
        else if (dbtype == "MTSD")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "" << Endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "" << Endl;
            h << "#include <avtMTSDFileFormat.h>" << Endl;
            h << "" << Endl;
            h << "#include <vector>" << Endl;
            h << "" << Endl;
            if (hasoptions)
            {
                h << "class DBOptionsAttributes;" << Endl;
                h << "" << Endl;
            }
            h << "" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"FileFormat" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "class avt"<<name<<"FileFormat : public avtMTSDFileFormat" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << Endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << Endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // This is used to return unconvention data -- ranging from material" << Endl;
            h << "    // information to information about block connectivity." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int timestep, " << Endl;
            h << "    //                                     const char *type, void *args, " << Endl;
            h << "    //                                     DestructorFunction &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // If you know the times and cycle numbers, overload this function." << Endl;
            h << "    // Otherwise, VisIt will make up some reasonable ones for you." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void        GetCycles(std::vector<int> &);" << Endl;
            h << "    // virtual void        GetTimes(std::vector<double> &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    virtual int            GetNTimesteps(void);" << Endl;
            h << "" << Endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << Endl;
            h << "    virtual void           FreeUpResources(void); " << Endl;
            h << "" << Endl;
            h << "    virtual vtkDataSet    *GetMesh(int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVar(int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, const char *);" << Endl;
            h << "" << Endl;
            h << "  protected:" << Endl;
            h << "    // DATA MEMBERS" << Endl;
            h << "" << Endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);" << Endl;
            h << "};" << Endl;
            h << "" << Endl;
            h << "" << Endl;
            h << "#endif" << Endl;
        }
        else if (dbtype == "STMD")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "" << Endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "" << Endl;
            h << "#include <avtSTMDFileFormat.h>" << Endl;
            h << "" << Endl;
            h << "#include <vector>" << Endl;
            h << "" << Endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << Endl;
            h << "" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"FileFormat" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "class avt"<<name<<"FileFormat : public avtSTMDFileFormat" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << Endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << Endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // This is used to return unconvention data -- ranging from material" << Endl;
            h << "    // information to information about block connectivity." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int domain," << Endl;
            h << "    //                                     const char *type, void *args, " << Endl;
            h << "    //                                     DestructorFunction &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // If you know the cycle number, overload this function." << Endl;
            h << "    // Otherwise, VisIt will make up a reasonable one for you." << Endl;
            h << "    //" << Endl;
            h << "    // virtual int         GetCycle(void);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << Endl;
            h << "    virtual void           FreeUpResources(void); " << Endl;
            h << "" << Endl;
            h << "    virtual vtkDataSet    *GetMesh(int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVar(int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, const char *);" << Endl;
            h << "" << Endl;
            h << "  protected:" << Endl;
            h << "    // DATA MEMBERS" << Endl;
            h << "" << Endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);" << Endl;
            h << "};" << Endl;
            h << "" << Endl;
            h << "" << Endl;
            h << "#endif" << Endl;
        }
        else if (dbtype == "MTMD")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//                            avt"<<name<<"FileFormat.h                           //" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "" << Endl;
            h << "#ifndef AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "#define AVT_"<<name<<"_FILE_FORMAT_H" << Endl;
            h << "" << Endl;
            h << "#include <avtMTMDFileFormat.h>" << Endl;
            h << "" << Endl;
            h << "#include <vector>" << Endl;
            h << "" << Endl;
            if (hasoptions)
                h << "class DBOptionsAttributes;" << Endl;
            h << "" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: avt"<<name<<"FileFormat" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//      Reads in "<<name<<" files as a plugin to VisIt." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            h << "//  Creation:   "<<CurrentTime()<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "class avt"<<name<<"FileFormat : public avtMTMDFileFormat" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if (hasoptions)
                h << "                       avt"<<name<<"FileFormat(const char *, DBOptionsAttributes *);" << Endl;
            else
                h << "                       avt"<<name<<"FileFormat(const char *);" << Endl;
            h << "    virtual           ~avt"<<name<<"FileFormat() {;};" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // This is used to return unconvention data -- ranging from material" << Endl;
            h << "    // information to information about block connectivity." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void      *GetAuxiliaryData(const char *var, int timestep, " << Endl;
            h << "    //                                     int domain, const char *type, void *args, " << Endl;
            h << "    //                                     DestructorFunction &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    //" << Endl;
            h << "    // If you know the times and cycle numbers, overload this function." << Endl;
            h << "    // Otherwise, VisIt will make up some reasonable ones for you." << Endl;
            h << "    //" << Endl;
            h << "    // virtual void        GetCycles(std::vector<int> &);" << Endl;
            h << "    // virtual void        GetTimes(std::vector<double> &);" << Endl;
            h << "    //" << Endl;
            h << "" << Endl;
            h << "    virtual int            GetNTimesteps(void);" << Endl;
            h << "" << Endl;
            h << "    virtual const char    *GetType(void)   { return \""<<name<<"\"; };" << Endl;
            h << "    virtual void           FreeUpResources(void); " << Endl;
            h << "" << Endl;
            h << "    virtual vtkDataSet    *GetMesh(int, int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVar(int, int, const char *);" << Endl;
            h << "    virtual vtkDataArray  *GetVectorVar(int, int, const char *);" << Endl;
            h << "" << Endl;
            h << "  protected:" << Endl;
            h << "    // DATA MEMBERS" << Endl;
            h << "" << Endl;
            h << "    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);" << Endl;
            h << "};" << Endl;
            h << "" << Endl;
            h << "" << Endl;
            h << "#endif" << Endl;
        }
    }
    void WriteFileFormatReaderSource(QTextStream &c)
    {
        if (dbtype == "STSD")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "" << Endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << Endl;
            c << "" << Endl;
            c << "#include <string>" << Endl;
            c << "" << Endl;
            c << "#include <vtkFloatArray.h>" << Endl;
            c << "#include <vtkRectilinearGrid.h>" << Endl;
            c << "#include <vtkStructuredGrid.h>" << Endl;
            c << "#include <vtkUnstructuredGrid.h>" << Endl;
            c << "" << Endl;
            c << "#include <avtDatabaseMetaData.h>" << Endl;
            c << "" << Endl;
            c << "#include <DBOptionsAttributes.h>" << Endl;
            c << "#include <Expression.h>" << Endl;
            c << "" << Endl;
            c << "#include <InvalidVariableException.h>" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "using     std::string;" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << Endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << Endl;
            c << "    : avtSTSDFileFormat(filename)" << Endl;
            c << "{" << Endl;
            c << "    // INITIALIZE DATA MEMBERS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << Endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << Endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << Endl;
            c << "//      that." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      This database meta-data object is like a table of contents for the" << Endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << Endl;
            c << "//      information it can request from you." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)" << Endl;
            c << "{" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MESH" << Endl;
            c << "    //" << Endl;
            c << "    // string meshname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << Endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << Endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << Endl;
            c << "    //" << Endl;
            c << "    // int nblocks = 1;  <-- this must be 1 for STSD" << Endl;
            c << "    // int block_origin = 0;" << Endl;
            c << "    // int spatial_dimension = 2;" << Endl;
            c << "    // int topological_dimension = 2;" << Endl;
            c << "    // double *extents = NULL;" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << Endl;
            c << "    //                   spatial_dimension, topological_dimension);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int vector_dim = 2;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int tensor_dim = 9;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MATERIAL" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string matname = ..." << Endl;
            c << "    // int nmats = ...;" << Endl;
            c << "    // vector<string> mnames;" << Endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //     char str[32];" << Endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << Endl;
            c << "    //     -- or -- " << Endl;
            c << "    //     strcpy(str, \"Aluminum\");" << Endl;
            c << "    //     mnames.push_back(str);" << Endl;
            c << "    // }" << Endl;
            c << "    // " << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the way to add expressions:" << Endl;
            c << "    //Expression momentum_expr;" << Endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << Endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << Endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << Endl;
            c << "    //md->AddExpression(&momentum_expr);" << Endl;
            c << "    //Expression KineticEnergy_expr;" << Endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << Endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << Endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << Endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << Endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << Endl;
            c << "//      vtkUnstructuredGrid, etc)." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << Endl;
            c << "//                  there is only one mesh." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"FileFormat::GetMesh(const char *meshname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVar(const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << Endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // float *one_entry = new float[ucomps];" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      int j;" << Endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << Endl;
            c << "    //           one_entry[j] = ..." << Endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << Endl;
            c << "    //           one_entry[j] = 0.;" << Endl;
            c << "    //      rv->SetTuple(i, one_entry); " << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // delete [] one_entry;" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
        }
        else if (dbtype == "MTSD")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "" << Endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << Endl;
            c << "" << Endl;
            c << "#include <string>" << Endl;
            c << "" << Endl;
            c << "#include <vtkFloatArray.h>" << Endl;
            c << "#include <vtkRectilinearGrid.h>" << Endl;
            c << "#include <vtkStructuredGrid.h>" << Endl;
            c << "#include <vtkUnstructuredGrid.h>" << Endl;
            c << "" << Endl;
            c << "#include <avtDatabaseMetaData.h>" << Endl;
            c << "" << Endl;
            c << "#include <DBOptionsAttributes.h>" << Endl;
            c << "#include <Expression.h>" << Endl;
            c << "" << Endl;
            c << "#include <InvalidVariableException.h>" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "using     std::string;" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << Endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << Endl;
            c << "    : avtMTSDFileFormat(&filename, 1)" << Endl;
            c << "{" << Endl;
            c << "    // INITIALIZE DATA MEMBERS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avtEMSTDFileFormat::GetNTimesteps" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Tells the rest of the code how many timesteps there are in this file." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "int" << Endl;
            c << "avt"<<name<<"FileFormat::GetNTimesteps(void)" << Endl;
            c << "{" << Endl;
            c << "    return YOU_MUST_DECIDE;" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << Endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << Endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << Endl;
            c << "//      that." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      This database meta-data object is like a table of contents for the" << Endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << Endl;
            c << "//      information it can request from you." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)" << Endl;
            c << "{" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MESH" << Endl;
            c << "    //" << Endl;
            c << "    // string meshname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << Endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << Endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << Endl;
            c << "    //" << Endl;
            c << "    // int nblocks = 1;  <-- this must be 1 for MTSD" << Endl;
            c << "    // int block_origin = 0;" << Endl;
            c << "    // int spatial_dimension = 2;" << Endl;
            c << "    // int topological_dimension = 2;" << Endl;
            c << "    // double *extents = NULL;" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << Endl;
            c << "    //                   spatial_dimension, topological_dimension);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int vector_dim = 2;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int tensor_dim = 9;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MATERIAL" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string matname = ..." << Endl;
            c << "    // int nmats = ...;" << Endl;
            c << "    // vector<string> mnames;" << Endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //     char str[32];" << Endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << Endl;
            c << "    //     -- or -- " << Endl;
            c << "    //     strcpy(str, \"Aluminum\");" << Endl;
            c << "    //     mnames.push_back(str);" << Endl;
            c << "    // }" << Endl;
            c << "    // " << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the way to add expressions:" << Endl;
            c << "    //Expression momentum_expr;" << Endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << Endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << Endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << Endl;
            c << "    //md->AddExpression(&momentum_expr);" << Endl;
            c << "    //Expression KineticEnergy_expr;" << Endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << Endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << Endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << Endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << Endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << Endl;
            c << "//      vtkUnstructuredGrid, etc)." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate   The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                  'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << Endl;
            c << "//                  there is only one mesh." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int timestate, const char *meshname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVar(int timestate, const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int timestate, const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << Endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // float *one_entry = new float[ucomps];" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      int j;" << Endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << Endl;
            c << "    //           one_entry[j] = ..." << Endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << Endl;
            c << "    //           one_entry[j] = 0.;" << Endl;
            c << "    //      rv->SetTuple(i, one_entry); " << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // delete [] one_entry;" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
        }
        else if (dbtype == "STMD")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "" << Endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << Endl;
            c << "" << Endl;
            c << "#include <string>" << Endl;
            c << "" << Endl;
            c << "#include <vtkFloatArray.h>" << Endl;
            c << "#include <vtkRectilinearGrid.h>" << Endl;
            c << "#include <vtkStructuredGrid.h>" << Endl;
            c << "#include <vtkUnstructuredGrid.h>" << Endl;
            c << "" << Endl;
            c << "#include <avtDatabaseMetaData.h>" << Endl;
            c << "" << Endl;
            c << "#include <DBOptionsAttributes.h>" << Endl;
            c << "#include <Expression.h>" << Endl;
            c << "" << Endl;
            c << "#include <InvalidVariableException.h>" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "using     std::string;" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << Endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << Endl;
            c << "    : avtSTMDFileFormat(&filename, 1)" << Endl;
            c << "{" << Endl;
            c << "    // INITIALIZE DATA MEMBERS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << Endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << Endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << Endl;
            c << "//      that." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      This database meta-data object is like a table of contents for the" << Endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << Endl;
            c << "//      information it can request from you." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)" << Endl;
            c << "{" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MESH" << Endl;
            c << "    //" << Endl;
            c << "    // string meshname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << Endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << Endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << Endl;
            c << "    //" << Endl;
            c << "    // int nblocks = YOU_MUST_DECIDE;" << Endl;
            c << "    // int block_origin = 0;" << Endl;
            c << "    // int spatial_dimension = 2;" << Endl;
            c << "    // int topological_dimension = 2;" << Endl;
            c << "    // double *extents = NULL;" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << Endl;
            c << "    //                   spatial_dimension, topological_dimension);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int vector_dim = 2;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int tensor_dim = 9;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MATERIAL" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string matname = ..." << Endl;
            c << "    // int nmats = ...;" << Endl;
            c << "    // vector<string> mnames;" << Endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //     char str[32];" << Endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << Endl;
            c << "    //     -- or -- " << Endl;
            c << "    //     strcpy(str, \"Aluminum\");" << Endl;
            c << "    //     mnames.push_back(str);" << Endl;
            c << "    // }" << Endl;
            c << "    // " << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the way to add expressions:" << Endl;
            c << "    //Expression momentum_expr;" << Endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << Endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << Endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << Endl;
            c << "    //md->AddExpression(&momentum_expr);" << Endl;
            c << "    //Expression KineticEnergy_expr;" << Endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << Endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << Endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << Endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << Endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << Endl;
            c << "//      vtkUnstructuredGrid, etc)." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      domain      The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                  value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                  regardless of block origin." << Endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << Endl;
            c << "//                  there is only one mesh." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int domain, const char *meshname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                 regardless of block origin." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVar(int domain, const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                 regardless of block origin." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int domain, const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << Endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // float *one_entry = new float[ucomps];" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      int j;" << Endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << Endl;
            c << "    //           one_entry[j] = ..." << Endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << Endl;
            c << "    //           one_entry[j] = 0.;" << Endl;
            c << "    //      rv->SetTuple(i, one_entry); " << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // delete [] one_entry;" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
        }
        else if (dbtype == "MTMD")
        {
            c << copyright_str << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "//                            avt"<<name<<"FileFormat.C                           //" << Endl;
            c << "// ************************************************************************* //" << Endl;
            c << "" << Endl;
            c << "#include <avt"<<name<<"FileFormat.h>" << Endl;
            c << "" << Endl;
            c << "#include <string>" << Endl;
            c << "" << Endl;
            c << "#include <vtkFloatArray.h>" << Endl;
            c << "#include <vtkRectilinearGrid.h>" << Endl;
            c << "#include <vtkStructuredGrid.h>" << Endl;
            c << "#include <vtkUnstructuredGrid.h>" << Endl;
            c << "" << Endl;
            c << "#include <avtDatabaseMetaData.h>" << Endl;
            c << "" << Endl;
            c << "#include <DBOptionsAttributes.h>" << Endl;
            c << "#include <Expression.h>" << Endl;
            c << "" << Endl;
            c << "#include <InvalidVariableException.h>" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "using     std::string;" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat constructor" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            if (hasoptions)
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename, DBOptionsAttributes *readOpts)" << Endl;
            else
                c << "avt"<<name<<"FileFormat::avt"<<name<<"FileFormat(const char *filename)" << Endl;
            c << "    : avtMTMDFileFormat(filename)" << Endl;
            c << "{" << Endl;
            c << "    // INITIALIZE DATA MEMBERS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avtEMSTDFileFormat::GetNTimesteps" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Tells the rest of the code how many timesteps there are in this file." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "int" << Endl;
            c << "avt"<<name<<"FileFormat::GetNTimesteps(void)" << Endl;
            c << "{" << Endl;
            c << "    return YOU_MUST_DECIDE;" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::FreeUpResources" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      When VisIt is done focusing on a particular timestep, it asks that" << Endl;
            c << "//      timestep to free up any resources (memory, file descriptors) that" << Endl;
            c << "//      it has associated with it.  This method is the mechanism for doing" << Endl;
            c << "//      that." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::FreeUpResources(void)" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::PopulateDatabaseMetaData" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      This database meta-data object is like a table of contents for the" << Endl;
            c << "//      file.  By populating it, you are telling the rest of VisIt what" << Endl;
            c << "//      information it can request from you." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << "avt"<<name<<"FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)" << Endl;
            c << "{" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MESH" << Endl;
            c << "    //" << Endl;
            c << "    // string meshname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH," << Endl;
            c << "    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH" << Endl;
            c << "    // avtMeshType mt = AVT_RECTILINEAR_MESH;" << Endl;
            c << "    //" << Endl;
            c << "    // int nblocks = YOU_MUST_DECIDE;" << Endl;
            c << "    // int block_origin = 0;" << Endl;
            c << "    // int spatial_dimension = 2;" << Endl;
            c << "    // int topological_dimension = 2;" << Endl;
            c << "    // double *extents = NULL;" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mesh:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin," << Endl;
            c << "    //                   spatial_dimension, topological_dimension);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A SCALAR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A VECTOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int vector_dim = 2;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A TENSOR VARIABLE" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string varname = ..." << Endl;
            c << "    // int tensor_dim = 9;" << Endl;
            c << "    //" << Endl;
            c << "    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT" << Endl;
            c << "    // avtCentering cent = AVT_NODECENT;" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a var:" << Endl;
            c << "    //" << Endl;
            c << "    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // CODE TO ADD A MATERIAL" << Endl;
            c << "    //" << Endl;
            c << "    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes" << Endl;
            c << "    // string matname = ..." << Endl;
            c << "    // int nmats = ...;" << Endl;
            c << "    // vector<string> mnames;" << Endl;
            c << "    // for (int i = 0 ; i < nmats ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //     char str[32];" << Endl;
            c << "    //     sprintf(str, \"mat%d\", i);" << Endl;
            c << "    //     -- or -- " << Endl;
            c << "    //     strcpy(str, \"Aluminum\");" << Endl;
            c << "    //     mnames.push_back(str);" << Endl;
            c << "    // }" << Endl;
            c << "    // " << Endl;
            c << "    // Here's the call that tells the meta-data object that we have a mat:" << Endl;
            c << "    //" << Endl;
            c << "    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);" << Endl;
            c << "    //" << Endl;
            c << "    //" << Endl;
            c << "    // Here's the way to add expressions:" << Endl;
            c << "    //Expression momentum_expr;" << Endl;
            c << "    //momentum_expr.SetName(\"momentum\");" << Endl;
            c << "    //momentum_expr.SetDefinition(\"{u, v}\");" << Endl;
            c << "    //momentum_expr.SetType(Expression::VectorMeshVar);" << Endl;
            c << "    //md->AddExpression(&momentum_expr);" << Endl;
            c << "    //Expression KineticEnergy_expr;" << Endl;
            c << "    //KineticEnergy_expr.SetName(\"KineticEnergy\");" << Endl;
            c << "    //KineticEnergy_expr.SetDefinition(\"0.5*(momentum*momentum)/(rho*rho)\");" << Endl;
            c << "    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);" << Endl;
            c << "    //md->AddExpression(&KineticEnergy_expr);" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetMesh" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets the mesh associated with this file.  The mesh is returned as a" << Endl;
            c << "//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid," << Endl;
            c << "//      vtkUnstructuredGrid, etc)." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate   The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                  'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      domain      The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                  value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                  regardless of block origin." << Endl;
            c << "//      meshname    The name of the mesh of interest.  This can be ignored if" << Endl;
            c << "//                  there is only one mesh." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataSet *" << Endl;
            c << "avt"<<name<<"FileFormat::GetMesh(int timestate, int domain, const char *meshname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a scalar variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                 regardless of block origin." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVar(int timestate, int domain, const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a scalar variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry." << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: avt"<<name<<"FileFormat::GetVectorVar" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//      Gets a vector variable associated with this file.  Although VTK has" << Endl;
            c << "//      support for many different types, the best bet is vtkFloatArray, since" << Endl;
            c << "//      that is supported everywhere through VisIt." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//      timestate  The index of the timestate.  If GetNTimesteps returned" << Endl;
            c << "//                 'N' time steps, this is guaranteed to be between 0 and N-1." << Endl;
            c << "//      domain     The index of the domain.  If there are NDomains, this" << Endl;
            c << "//                 value is guaranteed to be between 0 and NDomains-1," << Endl;
            c << "//                 regardless of block origin." << Endl;
            c << "//      varname    The name of the variable requested." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
            c << "//  Creation:   "<<CurrentTime()<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "vtkDataArray *" << Endl;
            c << "avt"<<name<<"FileFormat::GetVectorVar(int timestate, int domain,const char *varname)" << Endl;
            c << "{" << Endl;
            c << "    YOU MUST IMPLEMENT THIS" << Endl;
            c << "    //" << Endl;
            c << "    // If you have a file format where variables don't apply (for example a" << Endl;
            c << "    // strictly polygonal format like the STL (Stereo Lithography) format," << Endl;
            c << "    // then uncomment the code below." << Endl;
            c << "    //" << Endl;
            c << "    // EXCEPTION1(InvalidVariableException, varname);" << Endl;
            c << "    //" << Endl;
            c << "" << Endl;
            c << "    //" << Endl;
            c << "    // If you do have a vector variable, here is some code that may be helpful." << Endl;
            c << "    //" << Endl;
            c << "    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3." << Endl;
            c << "    // int ntuples = XXX; // this is the number of entries in the variable." << Endl;
            c << "    // vtkFloatArray *rv = vtkFloatArray::New();" << Endl;
            c << "    // int ucomps = (ncomps == 2 ? 3 : ncomps);" << Endl;
            c << "    // rv->SetNumberOfComponents(ucomps);" << Endl;
            c << "    // rv->SetNumberOfTuples(ntuples);" << Endl;
            c << "    // float *one_entry = new float[ucomps];" << Endl;
            c << "    // for (int i = 0 ; i < ntuples ; i++)" << Endl;
            c << "    // {" << Endl;
            c << "    //      int j;" << Endl;
            c << "    //      for (j = 0 ; j < ncomps ; j++)" << Endl;
            c << "    //           one_entry[j] = ..." << Endl;
            c << "    //      for (j = ncomps ; j < ucomps ; j++)" << Endl;
            c << "    //           one_entry[j] = 0.;" << Endl;
            c << "    //      rv->SetTuple(i, one_entry); " << Endl;
            c << "    // }" << Endl;
            c << "    //" << Endl;
            c << "    // delete [] one_entry;" << Endl;
            c << "    // return rv;" << Endl;
            c << "    //" << Endl;
            c << "}" << Endl;
        }
    }
    void WriteFileFormatOptionsHeader(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "//                             avt"<<name<<"Options.h                              //" << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "" << Endl;
        h << "#ifndef AVT_"<<name<<"_OPTIONS_H" << Endl;
        h << "#define AVT_"<<name<<"_OPTIONS_H" << Endl;
        h << "" << Endl;
        h << "class DBOptionsAttributes;" << Endl;
        h << "" << Endl;
        h << "#include <string>" << Endl;
        h << "" << Endl;
        h << "" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "//  Functions: avt"<<name<<"Options" << Endl;
        h << "//" << Endl;
        h << "//  Purpose:" << Endl;
        h << "//      Creates the options for  "<<name<<" readers and/or writers." << Endl;
        h << "//" << Endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        h << "//  Creation:   "<<CurrentTime()<< Endl;
        h << "//" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "" << Endl;
        h << "DBOptionsAttributes *Get"<<name<<"ReadOptions(void);" << Endl;
        h << "DBOptionsAttributes *Get"<<name<<"WriteOptions(void);" << Endl;
        h << "" << Endl;
        h << "" << Endl;
        h << "#endif" << Endl;
    }
    void WriteFileFormatOptionsSource(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "//                             avt"<<name<<"Options.C                              //" << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "" << Endl;
        h << "#include <avt"<<name<<"Options.h>" << Endl;
        h << "" << Endl;
        h << "#include <DBOptionsAttributes.h>" << Endl;
        h << "" << Endl;
        h << "#include <string>" << Endl;
        h << "" << Endl;
        h << "" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "//  Function: Get"<<name<<"ReadOptions" << Endl;
        h << "//" << Endl;
        h << "//  Purpose:" << Endl;
        h << "//      Creates the options for "<<name<<" readers." << Endl;
        h << "//" << Endl;
        h << "//  Important Note:" << Endl;
        h << "//      The code below sets up empty options.  If your format "
          << Endl;
        h << "//      does not require read options, no modifications are "
          << Endl;
        h << "//      necessary." << Endl;
        h << "//" << Endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        h << "//  Creation:   "<<CurrentTime()<< Endl;
        h << "//" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "" << Endl;
        h << "DBOptionsAttributes *" << Endl;
        h << "Get"<<name<<"ReadOptions(void)" << Endl;
        h << "{" << Endl;
        h << "    DBOptionsAttributes *rv = new DBOptionsAttributes;" << Endl;
        h << "    return rv;" << Endl;
        h << "/* EXAMPLE OF OPTIONS" << Endl;
        h << "    rv->SetBool(\"Binary format\", true);" << Endl;
        h << "    rv->SetBool(\"Big Endian\", false);" << Endl;
        h << "    rv->SetEnum(\"Dimension\", 1);" << Endl;
        h << "    vector<string> dims;" << Endl;
        h << "    dims.push_back(\"0D\");" << Endl;
        h << "    dims.push_back(\"1D\");" << Endl;
        h << "    dims.push_back(\"2D\");" << Endl;
        h << "    dims.push_back(\"3D\");" << Endl;
        h << "    rv->SetEnumStrings(\"Dimension\", dims);" << Endl;
        h << "    rv->SetInt(\"Number of variables\", 5);" << Endl;
        h << "    rv->SetString(\"Name of auxiliary file\", "");" << Endl;
        h << "    rv->SetDouble(\"Displacement factor\", 1.0);" << Endl;
        h << "" << Endl;
        h << "    // When reading or writing the file, you can get the options out of this object like:" << Endl;
        h << "    rv->GetDouble(\"Displacement factor\");" << Endl;
        h << "*/" << Endl;
        h << "}" << Endl;
        h << "" << Endl;
        h << "" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "//  Function: Get"<<name<<"WriteOptions" << Endl;
        h << "//" << Endl;
        h << "//  Purpose:" << Endl;
        h << "//      Creates the options for "<<name<<" writers." << Endl;
        h << "//" << Endl;
        h << "//  Important Note:" << Endl;
        h << "//      The code below sets up empty options.  If your format "
          << Endl;
        h << "//      does not require write options, no modifications are "
          << Endl;
        h << "//      necessary." << Endl;
        h << "//" << Endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        h << "//  Creation:   "<<CurrentTime()<< Endl;
        h << "//" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "" << Endl;
        h << "DBOptionsAttributes *" << Endl;
        h << "Get"<<name<<"WriteOptions(void)" << Endl;
        h << "{" << Endl;
        h << "    DBOptionsAttributes *rv = new DBOptionsAttributes;" << Endl;
        h << "    return rv;" << Endl;
        h << "}" << Endl;
    }
    void WriteFileFormatWriterHeader(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "//                             avt"<<name<<"Writer.h                              //" << Endl;
        h << "// ************************************************************************* //" << Endl;
        h << "" << Endl;
        h << "#ifndef AVT_"<<name<<"_WRITER_H" << Endl;
        h << "#define AVT_"<<name<<"_WRITER_H" << Endl;
        h << "" << Endl;
        h << "#include <avtDatabaseWriter.h>" << Endl;
        h << "" << Endl;
        h << "#include <string>" << Endl;
        h << "" << Endl;
        if (hasoptions)
            h << "class DBOptionsAttributes;" << Endl;
        h << "" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "//  Class: avt"<<name<<"Writer" << Endl;
        h << "//" << Endl;
        h << "//  Purpose:" << Endl;
        h << "//      A module that writes out "<<name<<" files." << Endl;
        h << "//" << Endl;
        h << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        h << "//  Creation:   "<<CurrentTime()<< Endl;
        h << "//" << Endl;
        h << "// ****************************************************************************" << Endl;
        h << "" << Endl;
        h << "class" << Endl;
        h << "avt"<<name<<"Writer : public avtDatabaseWriter" << Endl;
        h << "{" << Endl;
        h << "  public:" << Endl;
        if (hasoptions)
            h << "                   avt"<<name<<"Writer(DBOptionsAttributes *);" << Endl;
        else
            h << "                   avt"<<name<<"Writer();" << Endl;
        h << "    virtual       ~avt"<<name<<"Writer() {;};" << Endl;
        h << "" << Endl;
        h << "  protected:" << Endl;
        h << "    std::string    stem;" << Endl;
        h << "" << Endl;
        h << "    virtual void   OpenFile(const std::string &, int nb);" << Endl;
        h << "    virtual void   WriteHeaders(const avtDatabaseMetaData *," << Endl;
        h << "                                std::vector<std::string> &, " << Endl;
        h << "                                std::vector<std::string> &," << Endl;
        h << "                                std::vector<std::string> &);" << Endl;
        h << "    virtual void   WriteChunk(vtkDataSet *, int);" << Endl;
        h << "    virtual void   CloseFile(void);" << Endl;
        h << "};" << Endl;
        h << "" << Endl;
        h << "" << Endl;
        h << "#endif" << Endl;
    }
    void WriteFileFormatWriterSource(QTextStream &c)
    {
        c << copyright_str << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << "//                              avt"<<name<<"Writer.C                             //" << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << "" << Endl;
        c << "#include <avt"<<name<<"Writer.h>" << Endl;
        c << "" << Endl;
        c << "#include <vector>" << Endl;
        c << "" << Endl;
        c << "#include <vtkDataSetWriter.h>" << Endl;
        c << "" << Endl;
        c << "#include <avtDatabaseMetaData.h>" << Endl;
        if (hasoptions)
            c << "#include <DBOptionsAttributes.h>" << Endl;
        c << "" << Endl;
        c << "using     std::string;" << Endl;
        c << "using     std::vector;" << Endl;
        c << "" << Endl;
        c << "" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Writer constructor" << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "" << Endl;
        if (hasoptions)
            c << "avt"<<name<<"Writer::avt"<<name<<"Writer(DBOptionsAttributes *)" << Endl;
        else
            c << "avt"<<name<<"Writer::avt"<<name<<"Writer(void)" << Endl;
        c << "{" << Endl;
        c << "}" << Endl;
        c << "" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Writer::OpenFile" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Does no actual work.  Just records the stem name for the files." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "" << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Writer::OpenFile(const string &stemname, int numblocks)" << Endl;
        c << "{" << Endl;
        c << "    stem = stemname;" << Endl;
        c << "}" << Endl;
        c << "" << Endl;
        c << "" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Writer::WriteHeaders" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Writes out a VisIt file to tie the "<<name<<" files together." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "" << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Writer::WriteHeaders(const avtDatabaseMetaData *md," << Endl;
        c << "                           vector<string> &scalars, vector<string> &vectors," << Endl;
        c << "                           vector<string> &materials)" << Endl;
        c << "{" << Endl;
        c << "    // WRITE OUT HEADER INFO" << Endl;
        c << "}" << Endl;
        c << "" << Endl;
        c << "" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Writer::WriteChunk" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      This writes out one chunk of an avtDataset." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "" << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Writer::WriteChunk(vtkDataSet *ds, int chunk)" << Endl;
        c << "{" << Endl;
        c << "    // WRITE OUT THIS ONE CHUNK" << Endl;
        c << "}" << Endl;
        c << "" << Endl;
        c << "" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: avt"<<name<<"Writer::CloseFile" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//      Closes the file.  This does nothing in this case." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: "<<UserName()<<"generated by xml2avt" << Endl;
        c << "//  Creation:   "<<CurrentTime()<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "" << Endl;
        c << "void" << Endl;
        c << "avt"<<name<<"Writer::CloseFile(void)" << Endl;
        c << "{" << Endl;
        c << "    // CLOSE FILES" << Endl;
        c << "}" << Endl;
    }
    
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       AVTGeneratorPlugin

#endif
