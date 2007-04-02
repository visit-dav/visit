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

// ************************************************************************* //
//                            avtTFTFileFormat.C                             //
// ************************************************************************* //

#include <avtTFTFileFormat.h>

#include <string>
#include <string.h> // for strlen

#include <vtkCellArray.h>
#include <vtkPolyData.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <Utility.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


// ****************************************************************************
//  Method: avtTFT constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:16 PST 2005
//
// ****************************************************************************

avtTFTFileFormat::avtTFTFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), variables()
{
    initialized = false;
    hasValidTime = false;
    time = 0.f;
}

// ****************************************************************************
// Method: avtTFTFileFormat::~avtTFTFileFormat
//
// Purpose: 
//   Destructor for the avtTFTFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:50:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtTFTFileFormat::~avtTFTFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
// Method: avtTFTFileFormat::ReturnsValidTime
//
// Purpose: 
//   Returns whether this format returns valid times.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:50:15 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtTFTFileFormat::ReturnsValidTime() const
{
    debug4 << "avtTFTFileFormat::ReturnsValidTime" << endl;
    return hasValidTime;
}

// ****************************************************************************
// Method: avtTFTFileFormat::GetTime
//
// Purpose: 
//   Gets the current time value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:50:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

double
avtTFTFileFormat::GetTime(void)
{
    debug4 << "avtTFTFileFormat::GetTime" << endl;
    return double(time);
}

// ****************************************************************************
//  Method: avtTFTFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:16 PST 2005
//
// ****************************************************************************

void
avtTFTFileFormat::FreeUpResources(void)
{
    debug4 << "avtTFTFileFormat::FreeUpResources" << endl;
    for(CurveDataMap::iterator it = variables.begin();
        it != variables.end(); ++it)
    {
        delete it->second;
        it->second = 0;
    }
    variables.clear();
    initialized = false;
}

// ****************************************************************************
// Method: avtTFTFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates this time step.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:55:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtTFTFileFormat::ActivateTimestep(void)
{
    Initialize();
}

// ****************************************************************************
//  Method: avtTFTFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:16 PST 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtTFTFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    TRY
    {
        Initialize();
        CurveDataMap::const_iterator it = variables.begin();
        if(it != variables.end() && it->second != 0)
            md->SetDatabaseComment(it->second->subtitle);
    }
    CATCH(InvalidFilesException)
    {
#ifndef MDSERVER
        // Rethrow the exception on the engine.
        RETHROW;
#endif
    }
    ENDTRY

    // Add all of the curves to the metadata.
    for(CurveDataMap::const_iterator it = variables.begin();
        it != variables.end(); ++it)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = it->first;
        curve->xLabel = it->second->xLabel;
        curve->xUnits = it->second->xUnits;
        curve->yLabel = it->second->yLabel;
        curve->yUnits = it->second->yUnits;
        md->Add(curve);
    }
}


// ****************************************************************************
//  Method: avtTFTFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:16 PST 2005
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtTFTFileFormat::GetMesh(const char *name)
{
    CurveDataMap::const_iterator it = variables.find(name);
    if(it == variables.end())
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();
    pd->SetPoints(pts);
    int i, nPts = it->second->nPoints;
    pts->SetNumberOfPoints(nPts);
    const float *pdata = it->second->data;
    for(i = 0; i < nPts; ++i, pdata += 2)
        pts->SetPoint(i, pdata[0], pdata[1], 0.f);
 
    //
    // Connect the points up with line segments.
    //
    vtkCellArray *line = vtkCellArray::New();
    pd->SetLines(line);
    for(int k = 1 ; k < nPts ; k++)
    {
        line->InsertNextCell(2);
        line->InsertCellPoint(k-1);
        line->InsertCellPoint(k);
    }

    pts->Delete();
    line->Delete();

    return pd;
}

// ****************************************************************************
// Method: avtTFTFileFormat::GetVar
//
// Purpose: 
//   Returns a variable.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:51:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtTFTFileFormat::GetVar(const char *name)
{
    EXCEPTION1(InvalidVariableException, name);
}

// ****************************************************************************
// Method: avtTFTFileFormat::Initialize
//
// Purpose: 
//   Initializes the variable map by reading in the file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 17:51:52 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtTFTFileFormat::Initialize()
{
    if(!initialized)
    {
        ifstream ifile(filename);
        if (ifile.fail())
        {
            EXCEPTION1(InvalidFilesException, filename);
        }

        // Read all of the variables.
        std::string lastTitle;
        do
        {
            CurveData *curve = new CurveData;
            if(curve->Read(ifile, hasValidTime, time))
            {
                if(curve->title.size() > 0)
                    lastTitle = curve->title;
                else
                {
#if 1 
// This is a work around. If we don't have a title here, use the title,
// and units, etc from the last good curve with a title.
                    CurveDataMap::const_iterator it = 
                        variables.find(lastTitle);
                    if(it != variables.end())
                    {
                        curve->title = (it->second->title + "_2");
                        curve->subtitle = it->second->subtitle;
                        curve->xLabel = it->second->xLabel;
                        curve->xUnits = it->second->xUnits;
                        curve->yLabel = it->second->yLabel;
                        curve->yUnits = it->second->yUnits;

                        debug4 << "We're naming a curve: "
                               << curve->title.c_str()
                               << " because it had no name." << endl;
                        lastTitle = curve->title;
                    }
#endif
                }

                // Print the data about the curve data.
                debug4 << curve->title.c_str() << endl;
                debug4 << "============================================================\n";
                debug4 << "\tsubtitle = " << curve->subtitle.c_str() << endl;
                debug4 << "\tnPoints = " << curve->nPoints << endl;
                debug4 << "\txLabel = " << curve->xLabel.c_str() << endl;
                debug4 << "\txUnits = " << curve->xUnits.c_str() << endl;
                debug4 << "\tyLabel = " << curve->yLabel.c_str() << endl;
                debug4 << "\tyUnits = " << curve->yUnits.c_str() << endl;
                debug4 << "\txmin = " << curve->xmin << endl;
                debug4 << "\txmax = " << curve->xmax << endl;
                debug4 << "\tymin = " << curve->ymin << endl;
                debug4 << "\tymax = " << curve->ymax << endl;
                debug4 << endl;

                variables[lastTitle] = curve;

                debug4 << "Adding " << curve->title.c_str()
                       << " to the variables map." << endl;
            }
            else
            {
                debug4 << "Deleting the curve associated with "
                       << curve->title.c_str()
                       << " because we could not read in all of its data."
                       << endl;
                delete curve;
            }
        } while(!ifile.fail());

        if(hasValidTime)
            debug4 << "Time set to: " << time << endl;
        else
            debug4 << "Time not set!" << endl;

        initialized = true;
    }
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::CurveData
//
// Purpose: 
//   Constructor for avtTFTFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 19:15:12 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtTFTFileFormat::CurveData::CurveData() : title(), subtitle(), xLabel(),
    xUnits(), yLabel(), yUnits()
{
    nPoints = 0;
    data = 0;
    xmin = xmax = 0.f;
    ymin = ymax = 0.f;
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::~CurveData
//
// Purpose: 
//   Destructor for avtTFTFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 19:15:44 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtTFTFileFormat::CurveData::~CurveData()
{
    if(data != 0)
        delete [] data;
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::GetLine
//
// Purpose: 
//   Gets a line and strips off bad trailing characters.
//
// Arguments:
//   ifile : the input stream.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 19:16:42 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
avtTFTFileFormat::CurveData::GetLine(ifstream &ifile)
{
    char line[1024];
    ifile.getline(line, 1024);
    return StripExcess(line);
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::StripExcess
//
// Purpose: 
//   Strips excess characters off the end of a string.
//
// Arguments:
//   input : The input string.
//
// Returns:    The string that has had excess characters stripped.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 23 10:52:03 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
avtTFTFileFormat::CurveData::StripExcess(const std::string &input)
{
    std::string line(input);
    int s = input.size();
    int lastBadLocation = -1;
    for(int i = s; i > 0; --i)
    {
        if(line[i] == ' ' || line[i] == '\n' || line[i] == '\r' ||
           line[i] == '\0')
            lastBadLocation = i;
        else
            break;
    }

    if(lastBadLocation != -1)
        line = line.substr(0, lastBadLocation);

    return line;
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::GetLabelsAndUnits
//
// Purpose: 
//   Takes an input string and gets the label and units from it.
//
// Arguments:
//   input : The input string.
//   label : The return string for the label.
//   units : The return string for the units.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 23 10:51:16 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtTFTFileFormat::CurveData::GetLabelsAndUnits(const std::string &input,
    std::string &label, std::string &units)
{
    int startParen = input.find("(");
    if(startParen == -1)
    {
        // no units.
        units = "";
        label = StripExcess(input);
    }
    else
    {
        label = StripExcess(input.substr(0, startParen-1));
        int endParen = input.rfind(")");
        if(endParen != -1)
        {
            units = input.substr(startParen+1, endParen-startParen-1);
        }
        else
        {
            units = input.substr(startParen+1, input.size()-startParen-1);
        }
    }

#if 1
    // This is just for now since VisIt can't draw sideways text yet.
    // Remove this someday.
    for(int i = 0; i < label.size(); ++i)
    {
        if(label[i] == ' ')
            label[i] = '\n';
    }
#endif
}

// ****************************************************************************
// Method: avtTFTFileFormat::CurveData::Read
//
// Purpose: 
//   Reads a CurveData object from the ifstream.
//
// Arguments:
//   ifile   : the input stream.
//   setTime : whether the time was set.
//   time    : The time value that was set.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 19:18:04 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtTFTFileFormat::CurveData::Read(ifstream &ifile, bool &setTime, float &time)
{
    bool retval = true;
    char line[1024];
    int readHeader = 1, a,b,c,d;

    //
    // Get the number of points.
    //
    ifile.getline(line, 1024);
    if(sscanf(line, "%d %d %d %d %d", &readHeader,&b,&c,&nPoints, &d) == 5)
    {
        if(nPoints < 0)
            nPoints = 0;
        if(readHeader < 0 || readHeader > 1)
            readHeader = 1;
    }
    else
        retval = false;

    if(nPoints == 0)
        retval = false;

    // Only continue reading if it's okay by now.
    if(retval)
    {
        if(readHeader == 1)
        {
            // Get the subtitle.
            subtitle = GetLine(ifile);

            // Get the title and replace bad characters with underscore.
            title = GetLine(ifile);
            for(a = 0; a < title.size(); ++a)
            {
                if(title[a] == '(' || title[a] == ')' ||
                   title[a] == '[' || title[a] == ']' ||
                   title[a] == ' ')
                {
                    title[a] = '_';
                }
            }

            // Get the next 9 lines and see if any of them contain time.
            for(a = 0; a < 9; ++a)
            {
                float t = 0.f;
                ifile.getline(line, 1024);
                if(sscanf(line, "Time: %f", &t) == 1)
                {
                    setTime = true;
                    time = t;
                }
            }

            // Get the line that contains the label and units.
            std::string labelsAndUnits = GetLine(ifile);
            GetLabelsAndUnits(labelsAndUnits.substr(0,23), xLabel, xUnits);
            GetLabelsAndUnits(labelsAndUnits.substr(23,labelsAndUnits.size()-23),
                              yLabel, yUnits);

            // If the unit strings begin or end with (), strip it off.
            if(xUnits.find("(") == 0 && xUnits.rfind(")") == xUnits.size()-1)
                xUnits = xUnits.substr(1, xUnits.size() - 2);
            if(yUnits.find("(") == 0 && yUnits.rfind(")") == yUnits.size()-1)
                yUnits = yUnits.substr(1, yUnits.size() - 2);
 
            // Get the min, max line.
            ifile >> xmin;
            ifile >> xmax;
            ifile >> ymin;
            ifile >> ymax;
        }

        // Get the number of points 2 per line.
#ifdef MDSERVER
        // Don't save the data on the mdserver.
        float f0, f1;
        for(a = 0; a < nPoints; ++a)
        {
            ifile >> f0 >> f1;
        }
#else
        // Save the data if we're not on the mdserver.
        data = new float[2 * nPoints];
        for(a = 0; a < nPoints; ++a)
        {
            ifile >> data[(a*2)] >> data[(a*2)+1];
        }
#endif
        retval = !ifile.bad();
    }

    return retval;
}
