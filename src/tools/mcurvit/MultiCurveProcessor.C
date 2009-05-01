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

#include <MultiCurveProcessor.h>

#include <QApplication>
#include <QTimer>

#include <AnnotationAttributes.h>
#include <SaveWindowAttributes.h>

#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <DataNode.h>
#include <Synchronizer.h>
#include <ViewerEngineManager.h>
#include <VisItViewer.h>

#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

#define COMMAND_OPEN_DATABASE       0
#define COMMAND_CREATE_PLOT         1
#define COMMAND_FINISH_PLOT         2
#define COMMAND_SAVE_WINDOW         3
#define COMMAND_QUIT                4

const char *formatToString[] = {"ps", "bmp", "jpeg", "png",
                                "ppm", "rgb", "tif"};

// ****************************************************************************
// Method: MultiCurveProcessor::MultiCurveProcessor
//
// Purpose:
//   Constructor.
//
// Arguments:
//   v         The VisItViewer object that we'll use to control the viewer.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

MultiCurveProcessor::MultiCurveProcessor(VisItViewer *v) : QObject()
{
    viewer = v;

    sync = new Synchronizer(v->State()->GetSyncAttributes(),
                            v->DelayedState()->GetSyncAttributes());
}

// ****************************************************************************
// Method: MultiCurveProcessor::~MultiCurveProcessor
//
// Purpose:
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

MultiCurveProcessor::~MultiCurveProcessor()
{
}

// ****************************************************************************
// Method: MultiCurveProcessor::ProcessBatchFile
//
// Purpose:
//   Process a batch file and exit.
//
// Arguments:
//   fileName  The name of the file with the commands.
//   dataFile  The name of the data file to process.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ProcessBatchFile(const char *fileName, const char *dataFile)
{
    //
    // Read the file.
    //
    save = false;
    saveFormat = 0;
    openFile = string(dataFile);
    saveFile = string("multicurve");

    ReadFile(fileName);

    //
    // Print status informatoin to the user.
    //
    cerr << "Saving images: " << (save == 1 ? "true" : "false") << endl;
    cerr << "Saving images in format " << formatToString[saveFormat] << endl;
    cerr << "Saving images with the base file name " << saveFile << endl;
    cerr << "Data file: " << openFile << endl;
    cerr << "Creating plots of variables:" << endl;
    for (int i = 0; i < plotVars.size(); i++)
        cerr << "   " << plotVars[i] << endl;

    //
    // Create a list of commands.
    //
    if (openFile != "")
    {
        commandList.push_back(COMMAND_OPEN_DATABASE);
        for (int i = 0; i < plotVars.size(); i++)
        {
            commandList.push_back(COMMAND_CREATE_PLOT);
            commandList.push_back(COMMAND_FINISH_PLOT);
            if (save)
                commandList.push_back(COMMAND_SAVE_WINDOW);
        }
    }
    commandList.push_back(COMMAND_QUIT);
    iCommand = 0;
    iVar = 0;

    QTimer::singleShot(20, this, SLOT(execCommands()));
}

// ****************************************************************************
// Method: MultiCurveProcessor::OpenDatabase
//
// Purpose:
//   Have the viewer open a database.
//
// Arguments:
//   fileName  The name of the file to open.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::OpenDatabase(const string &fileName)
{
    viewer->Methods()->OpenDatabase(fileName);
}

// ****************************************************************************
// Method: MultiCurveProcessor::CreatePlot
//
// Purpose:
//   Have the viewer create a plot. The creation is split into 2 parts
//   because there are some annotations in the image that have properties
//   set and that can only be done later.
//
// Arguments:
//   varName   The name of the variable to create the plot of.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::CreatePlot(const string &varName)
{
    // Delete the active plots.
    viewer->DelayedMethods()->DeleteActivePlots();

    // Create a title.
    viewer->DelayedMethods()->AddAnnotationObject(AnnotationObject::Text2D, "title0");
    viewer->DelayedMethods()->AddAnnotationObject(AnnotationObject::Text2D, "title1");

    // Create a multi curve plot.
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    viewer->DelayedMethods()->AddPlot(MultiCurve, varName);
    AttributeSubject *multiCurveAtts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(multiCurveAtts != 0)
    {
        DataNode root("root");
        multiCurveAtts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");

        DataNode *multiColorNode = multiCurveNode->GetNode("multiColor");
        DataNode *calNode = multiColorNode->GetNode("ColorAttributeList");
        DataNode **children = calNode->GetChildren();

        unsigned char color[4];
        color[0] = 0; color[1] = 100; color[2] = 0; color[3] = 255;
        children[0]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 85; color[1] = 107; color[2] = 47;
        children[1]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 205; color[2] = 208;
        children[2]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 0; color[2] = 254;
        children[3]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 209; color[1] = 105; color[2] = 30;
        children[4]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 147; color[1] = 0; color[2] = 210;
        children[5]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 83; color[1] = 133; color[2] = 138;
        children[6]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 204; color[1] = 38; color[2] = 38;
        children[7]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 0; color[2] = 0;
        children[8]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 220; color[1] = 20; color[2] = 60;
        children[9]->GetNode("color")->SetUnsignedCharArray(color, 4);

        multiCurveNode->GetNode("yAxisTitleFormat")->SetString("%4.2f");
        multiCurveNode->GetNode("useYAxisTickSpacing")->SetBool(true);
        multiCurveNode->GetNode("yAxisTickSpacing")->SetDouble(0.25);
        multiCurveNode->GetNode("markerVariable")->SetString("v");

        multiCurveAtts->SetFromNode(&root);
        multiCurveAtts->Notify();
        viewer->DelayedMethods()->SetPlotOptions(MultiCurve);
    }

    // Add an index select operator.
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    viewer->DelayedMethods()->AddOperator(IndexSelect);
    AttributeSubject *indexSelectAtts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(indexSelectAtts != 0)
    {
        indexSelectAtts->SetValue("xMin", 0);
        indexSelectAtts->SetValue("xMax", -1);
        indexSelectAtts->SetValue("yMin", 16);
        indexSelectAtts->SetValue("yMax", 25);
        indexSelectAtts->Notify();
        viewer->DelayedMethods()->SetOperatorOptions(IndexSelect);
    }
}

// ****************************************************************************
// Method: MultiCurveProcessor::FinishPlot
//
// Purpose:
//   Have the viewer finish creating a plot. The creation is split into 2
//   parts because there are some annotations in the image that have
//   properties set and that can only be done later.
//
// Arguments:
//   varName   The name of the variable to create the plot of.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::FinishPlot()
{
    if (viewer->DelayedState()->GetAnnotationObjectList()->GetNumAnnotations()> 0)
    {
        // Set the first title.
        vector<string> text;
        text.push_back("Multi Curve Plot");
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetText(text);
        double position[2];
        position[0] = 0.3; position[1] = 0.92;
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetPosition(position);
        position[0] = 0.4;  // The text width
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetPosition2(position);
        ColorAttribute red;
        red.SetRgba(255, 0, 0, 255);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetTextColor(red);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetUseForegroundForTextColor(false);

        // Set the second title.
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetText(text);
        position[0] = 0.3; position[1] = 0.02;
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetPosition(position);
        position[0] = 0.4;  // The text width
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetPosition2(position);
        red.SetRgba(0, 255, 0, 255);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetTextColor(red);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetUseForegroundForTextColor(false);

        viewer->DelayedState()->GetAnnotationObjectList()->Notify();
        viewer->DelayedMethods()->SetAnnotationObjectOptions();
    }

    // Draw the plot.
    viewer->DelayedMethods()->DrawPlots();
}

// ****************************************************************************
// Method: MultiCurveProcessor::SaveWindow
//
// Purpose:
//   Have the viewer save the window.
//
// Arguments:
//   saveFormat  The format to save the image in.
//   saveFile    The base name of the file to save the image in.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::SaveWindow(const int saveFormat, const string &saveFile)
{
    // Set the output format and file name.
    SaveWindowAttributes *swa = viewer->State()->GetSaveWindowAttributes();
    switch (saveFormat)
    {
      case 0:
        swa->SetFormat(SaveWindowAttributes::POSTSCRIPT);
        break;
      case 1:
        swa->SetFormat(SaveWindowAttributes::BMP);
        break;
      case 2:
        swa->SetFormat(SaveWindowAttributes::JPEG);
        break;
      case 3:
        swa->SetFormat(SaveWindowAttributes::PNG);
        break;
      case 4:
        swa->SetFormat(SaveWindowAttributes::PPM);
        break;
      case 5:
        swa->SetFormat(SaveWindowAttributes::RGB);
        break;
      case 6:
        swa->SetFormat(SaveWindowAttributes::TIFF);
        break;
    }
    swa->SetFileName(saveFile);
    swa->SetScreenCapture(false);

    // Save the window.
    viewer->Methods()->SaveWindow();
}

// ****************************************************************************
// Method: MultiCurveProcessor::ReadInt
//
// Purpose:
//   Read an integer from the file.
//
// Arguments:
//   file      The file to read.
//   c         The current character to process from the file.
//   value     The integer read from the file.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ReadInt(FILE *file, int &c, int &value)
{
    char str[256];
    int istr = 0;

    while (!isspace(c) && c != EOF)
    {
        str[istr++] = c;
        c = fgetc(file);
    }
    str[istr] = '\0';

    value = int(strtod(str, (char**)NULL));
}

// ****************************************************************************
// Method: MultiCurveProcessor::ReadDouble
//
// Purpose:
//   Read a double from the file.
//
// Arguments:
//   file      The file to read.
//   c         The current character to process from the file.
//   value     The double read from the file.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ReadDouble(FILE *file, int &c, double &value)
{
    char str[256];
    int istr = 0;

    while (!isspace(c) && c != EOF)
    {
        str[istr++] = c;
        c = fgetc(file);
    }
    str[istr] = '\0';

    value = strtod(str, (char**)NULL);
}

// ****************************************************************************
// Method: MultiCurveProcessor::ReadString
//
// Purpose:
//   Read an unquoted string from the file.
//
// Arguments:
//   file      The file to read.
//   c         The current character to process from the file.
//   value     The string read from the file.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ReadString(FILE *file, int &c, string &value)
{
    char str[256];
    int istr = 0;

    while (!isspace(c) && c != EOF && istr < 255)
    {
        str[istr++] = c;
        c = fgetc(file);
    }
    str[istr] = '\0';

    value = str;
}

// ****************************************************************************
// Method: MultiCurveProcessor::ReadQuotedString
//
// Purpose:
//   Read a quoted string from the file.
//
// Arguments:
//   file      The file to read.
//   c         The current character to process from the file.
//   value     The string read from the file.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ReadQuotedString(FILE *file, int &c, string &value)
{
    char str[256];
    int istr = 0;

    if (c == '"')
        c = fgetc(file);

    while (c != '"' && c != EOF && istr < 255)
    {
        str[istr++] = c;
        c = fgetc(file);
    }
    str[istr] = '\0';

    value = str;
}

// ****************************************************************************
// Method: MultiCurveProcessor::ReadFile
//
// Purpose:
//   Read a command file.
//
// Arguments:
//   fileName  The name of the file to read.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveProcessor::ReadFile(const char *fileName)
{
    //
    // Open the file, making appropriate error checks.
    //
    if (fileName == NULL)
        return;

    FILE *file = fopen(fileName, "r");
    if (file == NULL)
        return;

    //
    // Process the file.
    //
    int c = fgetc(file);
    while (c != EOF)
    {
        //
        // Skip over any white space.
        //
        while (isspace(c))
            c = fgetc(file);

        //
        // If it is an alpha numeric character then it might be a keyword.
        //
        if (isalpha(c))
        {
            //
            // Read in the keyword. It can't be longer than 31 characters.
            //
            char keyword[32];
            int ikeyword;

            keyword[0] = char(c);
            ikeyword = 1;
            c = fgetc(file);
            while (isalpha(c) && ikeyword < 32)
            {
                keyword[ikeyword++] = char(c);
                c = fgetc(file);
            }

            //
            // If not a space or =, then it is an error.
            //
            if ((isspace(c) || c == '=') && ikeyword < 32)
            {
                keyword[ikeyword] = '\0';

                while (isspace(c))
                    c = fgetc(file);
                if (c == '=')
                {
                    c = fgetc(file);
                    while (isspace(c))
                        c = fgetc(file);
                }

                if (strcmp(keyword, "newplot") == 0)
                {
                     string varName;

                     ReadString(file, c, varName);
                     plotVars.push_back(varName);
                }
                else if (strcmp(keyword, "save") == 0)
                {
                     save = true;
                }
                else if (strcmp(keyword, "format") == 0)
                {
                     string format;

                     ReadString(file, c, format);
                     if (format == "ps")
                         saveFormat = 0;
                     else if (format == "bmp")
                         saveFormat = 1;
                     else if (format == "jpeg")
                         saveFormat = 2;
                     else if (format == "png")
                         saveFormat = 3;
                     else if (format == "ppm")
                         saveFormat = 4;
                     else if (format == "rgb")
                         saveFormat = 5;
                     else if (format == "tif")
                         saveFormat = 6;
                }
                else if (strcmp(keyword, "filename") == 0)
                {
                     ReadQuotedString(file, c, saveFile);
                }
                else
                {
                    //
                    // An unknown keyword, just skip any remaining white space.
                    //
                    if (!isspace(c) && c != EOF)
                        c = fgetc(file);
                }
            }
            else
            {
                if (!isspace(c) && c != EOF)
                    c = fgetc(file);
            }
        }
        else
        {
            //
            // Keep reading until we encounter some white space.
            //
            while (!isspace(c) && c != EOF)
                 c = fgetc(file);
        }
    }

    fclose(file);
}

//
// Qt slots
//

void
MultiCurveProcessor::execCommands()
{
    //
    // Connect the synchronizer and post a synchronize.  This causes
    // execCommand to be called after the viewer is initialized.
    //
    if (commandList.size() > 0)
    {
        connect(sync, SIGNAL(synchronized()),
                this, SLOT(execCommand()));
        sync->PostSynchronize();
    }
}

void
MultiCurveProcessor::execCommand()
{
    //
    // Execute the next command, if there is one.
    //
    if (iCommand >= commandList.size())
    {
        disconnect(sync, SIGNAL(synchronized()),
                   this, SLOT(execCommand()));
        return;
    }

    switch (commandList[iCommand])
    {
      case COMMAND_OPEN_DATABASE:
        cerr << "Opening the file " << openFile << endl;
        OpenDatabase(openFile);
        break;
      case COMMAND_CREATE_PLOT:
        cerr << "Creating a plot of " << plotVars[iVar] << endl;
        CreatePlot(plotVars[iVar++]);
        break;
      case COMMAND_FINISH_PLOT:
        FinishPlot();
        break;
      case COMMAND_SAVE_WINDOW:
        cerr << "Saving an image" << endl;
        SaveWindow(saveFormat, saveFile);
        break;
      case COMMAND_QUIT:
        cerr << "Quiting" << endl;
        qApp->quit();
        break;
    }
    iCommand++;

    //
    // Issue a synchronize. We'll get back to this method when the
    // synchronizer issues its signal.
    //
    sync->PostSynchronize();
}
