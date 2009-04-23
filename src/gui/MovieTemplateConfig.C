/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <MovieTemplateConfig.h>
#include <algorithm>

#include <DebugStream.h>
#include <MovieSequenceFactory.h>
#include <MovieSequence.h>
#include <MovieUtility.h>

#include <visit-config.h>
#include <snprintf.h>

static const char *VIEWPORTS_MISSING_WRONG_TYPE = 
    "VIEWPORTS node not found or wrong type.";

// ****************************************************************************
// Method: MovieTemplateConfig::MovieTemplateConfig
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:08:59 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieTemplateConfig::MovieTemplateConfig() : ConfigManager(), sequences()
{
    root = 0;
    activeViewport = 0;
    activeSequence = 0;

    CreateDefaultConfig();
}

// ****************************************************************************
// Method: MovieTemplateConfig::~MovieTemplateConfig
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:09:47 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieTemplateConfig::~MovieTemplateConfig()
{
    delete root;

    // Delete the sequence objects.
    StringMovieSequenceMap::iterator it = sequences.begin();
    for(; it != sequences.end(); ++it)
        delete it->second;
}

// ****************************************************************************
// Method: MovieTemplateConfig::CreateDefaultConfig
//
// Purpose: 
//   Creates a default config.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:09:58 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieTemplateConfig::CreateDefaultConfig()
{
    const char *mName = "MovieTemplateConfig::CreateDefaultConfig: ";
    debug2 << mName << "start" << endl;

    if(root != 0)
        delete root;

    root = new DataNode("TemplateOptions");
    root->AddNode(new DataNode("VERSION", std::string(VERSION)));
    root->AddNode(new DataNode("TITLE", std::string("New template")));
    root->AddNode(new DataNode("DESCRIPTION", std::string("Description")));
    root->AddNode(new DataNode("PREVIEWIMAGE", std::string("")));

    // Add the "visitmovietemplate.py" template file.
    root->AddNode(new DataNode("TEMPLATEFILE", GetVisItMovieTemplateBaseClass()));

    // Add empty VIEWPORTS node.
    root->AddNode(new DataNode("VIEWPORTS"));

    // Add SEQUENCEDATA node that contains the Window sequences.
    DataNode *sNode = new DataNode("SEQUENCEDATA");
    CreateSequenceObjects(sNode, "", 16);
    root->AddNode(sNode);
    debug2 << mName << "end" << endl;
}

// ****************************************************************************
// Method: MovieTemplateConfig::CreateSequenceObjects
//
// Purpose: 
//   Creates sequence objects using the sequence data stored in the data nodes.
//
// Arguments:
//   sNode    : The parent SEQUENCEDATA node.
//   filename : The filename that yielded the sequence data.
//   maxseq   : not so important anymore.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:10:27 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
MovieTemplateConfig::CreateSequenceObjects(DataNode *sNode, 
    const char *filename, int maxseq)
{
    const char *mName = "MovieTemplateConfig::CreateSequenceObjects: ";

    // Remove all sequences "Window %d" with new versions to
    // prevent the config file from corrupting us in the event
    // that it contains non-CurrentPlot sequences with those names.
    for(int i = 1; i <= 16; ++i)
    {
        char tmp[20];
        SNPRINTF(tmp, 20, "Window %d", i);
        std::string nodeName(tmp);

        if(sNode->GetNode(nodeName) != 0)
            sNode->RemoveNode(nodeName);

        if(i <= maxseq)
        {
            DataNode *wn = new DataNode(nodeName);
            wn->AddNode(new DataNode("sequenceType",
                std::string("CurrentPlots")));
            sNode->AddNode(wn);
            debug5 << mName << "Added new node " << nodeName.c_str() << endl;
        }
    }

    //
    // Now that we've done a first pass at validation, let's
    // create the MovieSequence objects for each sequence data node.
    //
    MovieSequenceFactory *f = MovieSequenceFactory::Instance();
    DataNode **children = sNode->GetChildren();
    for(int i = 0; i < sNode->GetNumChildren(); ++i)
    {
        if(children[i]->GetNodeType() == INTERNAL_NODE)
        {
            DataNode *seqType = children[i]->GetNode("sequenceType");
            if(seqType != 0 && seqType->GetNodeType() == STRING_NODE)
            {
                int id = -1;
                if(f->SequenceNameToId(seqType->AsString(), id))
                {
                    // Create the new movie sequence.
                    MovieSequence *seq = f->Create(id);
                    seq->SetName(children[i]->GetKey());

                    // Give it an opportunity to init itself.
                    seq->InitializeFromValues(filename, children[i]);

                    // Save it in the map.
                    sequences[children[i]->GetKey()] = seq;
                }
                else
                {
                    debug1 << mName << seqType->AsString().c_str()
                           << " is not a recognized sequence type."
                           << endl;
                }
            }
            else
            {
                debug1 << mName << "The \"sequenceType\" node was "
                    "not found or it was the wrong type." << endl;
            }
        }
        else
        {
            debug1 << mName << "Skipping field "
                   << children[i]->GetKey().c_str()
                   << endl;
        }
    }
}

// ****************************************************************************
// Method: MovieTemplateConfig::ReadConfigFile
//
// Purpose: 
//   Reads the template specification file and creates the sequence objects
//
// Arguments:
//   filename : The filename to open.
//
// Returns:    A pointer to the root data node.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:12:12 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 10:52:22 PDT 2008
//   Added debugging statements and code to skip the XML tag. Make the
//   TemplateOptions node be the root node.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

DataNode *
MovieTemplateConfig::ReadConfigFile(const char *filename)
{
    const char *mName = "MovieTemplateConfig::ReadConfigFile: ";
    DataNode *node = 0;

    // Try and open the file for reading.
    if((fp = fopen(filename, "rt")) == 0)
    {
        debug1 << mName << "Could not open " << filename << endl;
        return node;
    }
    else
        debug1 << mName << "Opened " << filename << endl;

    // Skip the XMl tag
    FinishTag();

    // Read the real data.
    root = new DataNode("root");
    ReadObject(root);
    fclose(fp); fp = 0;
    debug1 << mName << "The objects were read " << endl;

    // Reparent the TemplateOptions node from the "root" node and 
    // make it the new root node.
    DataNode **children = root->GetChildren();
    for(int i = 0; i < root->GetNumChildren(); ++i)
    {
        DataNode *thisChild = children[i];
        if(thisChild->GetKey() == "TemplateOptions")
        { 
            root->RemoveNode(thisChild, false);
            delete root;
            root = thisChild;
            debug1 << mName << "Setting TemplateOptions as the new root" << endl;
            break;
        }
    }

    // Print the data node tree to the logs so we can see its structure.
    if(DebugStream::Level5())
        root->Print(DebugStream::Stream5());

    // Remove all of the sequences.
    SequenceRemoveAll();

    // Iterate over the new SEQUENCEDATA node and create sequence objects
    // for each of them. We'll access the sequence data via the sequence
    // objects.
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *sNode = r->GetNode("SEQUENCEDATA");
        if(sNode != 0 && sNode->GetNodeType() == INTERNAL_NODE)
        {
            // Add the default "Window %d" sequences.
            debug1 << mName << "Adding the default Window sequences" << endl;
            CreateSequenceObjects(sNode, filename, 16);
        }
        else
        {
            debug1 << mName << "The SEQUENCEDATA node could not be found "
                "or it was the wrong type. Consequently, no movie sequence "
                "data could be obtained." << endl;
        }
    }
    else
        debug1 << mName << "Root node is NULL!" << endl;

    return GetRootNode();
}

// ****************************************************************************
// Method: MovieTemplateConfig::WriteConfigFile
//
// Purpose: 
//   Writes the template specification to a file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:12:56 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::WriteConfigFile(const char *filename)
{
    // If we have no root then return.
    if(root == 0)
        return false;

    // Try to open the output file.
    if((fp = fopen(filename, "wt")) == 0)
        return false;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(GetRootNode());

    // close the file
    fclose(fp);
    fp = 0;

    return true;
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetRootNode
//
// Purpose: 
//   Get the root data node.
//
// Returns:    The root node or 0.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:29 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
MovieTemplateConfig::GetRootNode() const
{
    DataNode *ret = 0;
    if(root != 0)
        ret = root->SearchForNode("TemplateOptions");
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetTitle
//
// Purpose: 
//   Get/Set the template's title.
//
// Arguments:
//   title : The template title.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetTitle(std::string &title) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("TITLE");
        if(node != 0 && node->GetNodeType() == STRING_NODE)
        {
            ret = true;
            title = node->AsString();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetTitle(const std::string &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("TITLE");
        r->AddNode(new DataNode("TITLE", s));
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetDescription
//
// Purpose: 
//   Get/Set the template's description.
//
// Arguments:
//   description : The template description.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetDescription(std::string &description) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("DESCRIPTION");
        if(node != 0 && node->GetNodeType() == STRING_NODE)
        {
            ret = true;
            description = node->AsString();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetDescription(const std::string &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("DESCRIPTION");
        r->AddNode(new DataNode("DESCRIPTION", s));
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetPreviewImageFile
//
// Purpose: 
//   Get/Set the template's preview image filename.
//
// Arguments:
//   image : The template preview image filename.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetPreviewImageFile(std::string &image) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("PREVIEWIMAGE");
        if(node != 0 && node->GetNodeType() == STRING_NODE)
        {
            ret = true;
            image = node->AsString();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetPreviewImageFile(const std::string &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("PREVIEWIMAGE");
        r->AddNode(new DataNode("PREVIEWIMAGE", s));
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetTemplateFile
//
// Purpose: 
//   Get/Set the template's Python file.
//
// Arguments:
//   templateFile : The template Python file.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetTemplateFile(std::string &templateFile) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("TEMPLATEFILE");
        if(node != 0 && node->GetNodeType() == STRING_NODE)
        {
            ret = true;
            templateFile = node->AsString();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetTemplateFile(const std::string &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("TEMPLATEFILE");
        r->AddNode(new DataNode("TEMPLATEFILE", s));
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetSessionFile
//
// Purpose: 
//   Get/Set the template's session file.
//
// Arguments:
//   sessionFile : The template session file.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetSessionFile(std::string &sessionFile) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("SESSIONFILE");
        if(node != 0 && node->GetNodeType() == STRING_NODE)
        {
            ret = true;
            sessionFile = node->AsString();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetSessionFile(const std::string &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("SESSIONFILE");
        r->AddNode(new DataNode("SESSIONFILE", s));
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::Get/SetSources
//
// Purpose: 
//   Get/Set the template's session file.
//
// Arguments:
//   sources : The sources to use when using the template.
//
// Returns:    True on success; False othwerwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:13:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::GetSources(stringVector &sources) const
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        DataNode *node = r->GetNode("SOURCES");
        if(node != 0 && node->GetNodeType() == STRING_VECTOR_NODE)
        {
            ret = true;
            sources = node->AsStringVector();
        }
    }
    return ret;
}

bool
MovieTemplateConfig::SetSources(const stringVector &s)
{
    bool ret = false;
    DataNode *r = GetRootNode();
    if(r != 0)
    {
        r->RemoveNode("SOURCES");
        r->AddNode(new DataNode("SOURCES", s));
        ret = true;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovieTemplateConfig::GetViewport
//
// Purpose: 
//   Gets the root node of the named viewport.
//
// Arguments:
//   name : The viewport name.
//
// Returns:    The named viewport node.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:20:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
MovieTemplateConfig::GetViewport(const std::string &name) const
{
    const char *mName = "MovieTemplateConfig::GetViewport: ";
    DataNode *node = 0;
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        node = vpData->GetNode(name);
        if(node->GetNodeType() != INTERNAL_NODE)
            node = 0;
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return node;
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetActiveViewport
//
// Purpose: 
//   Gets the root node of the active viewport.
//
// Returns:    The active viewport node.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:20:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

DataNode *
MovieTemplateConfig::GetActiveViewport()
{
    const char *mName = "MovieTemplateConfig::GetActiveViewport: ";
    if(activeViewport == 0)
    {
        DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
        if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
        {
            DataNode **children = vpData->GetChildren();
            for(int i = 0; i < vpData->GetNumChildren(); ++i)
            {
                if(children[i]->GetNodeType() == INTERNAL_NODE)
                {
                    activeViewport = children[i];
                    break;
                }
            }
        }
        else
            debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;
    }
    return activeViewport;
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetNumberOfViewports
//
// Purpose: 
//   Get the number of viewports.
//
// Returns:    The number of viewports.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:21:08 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
MovieTemplateConfig::GetNumberOfViewports() const
{
    const char *mName = "MovieTemplateConfig::GetNumberOfViewports: ";
    int nvpt = 0;
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        DataNode **children = vpData->GetChildren();
        for(int i = 0; i < vpData->GetNumChildren(); ++i)
        {
            if(children[i]->GetNodeType() == INTERNAL_NODE)
                ++nvpt;
        }
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return nvpt;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetActiveName
//
// Purpose: 
//   Get the name of the active viewport.
//
// Arguments:
//   name : Return value for the viewport name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:21:31 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetActiveName(std::string &name)
{
    DataNode *vp = GetActiveViewport();
    if(vp != 0)
        name = vp->GetKey();
    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetIndexForName
//
// Purpose: 
//   Get the index of the viewport in the list given its name.
//
// Arguments:
//   name : The name of the viewport to find.
//   index : Return value for the index.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:22:07 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetIndexForName(const std::string &name,
    int &index) const
{
    const char *mName = "MovieTemplateConfig::ViewportGetIndexForName: ";
    int _index = 0;
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        DataNode **children = vpData->GetChildren();
        for(int i = 0; i < vpData->GetNumChildren(); ++i)
        {
            if(children[i]->GetNodeType() == INTERNAL_NODE)
            {
                if(children[i]->GetKey() == name)
                {
                    index = _index;
                    return true;
                }
                _index++;
            }
        }
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return false;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetNameForIndex
//
// Purpose: 
//   Gets the name of the index'th viewport.
//
// Arguments:
//   index : The index of the viewport whose name we want.
//   name  : Return value for the viewport name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:22:58 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetNameForIndex(int index, std::string &name) const
{
    const char *mName = "MovieTemplateConfig::ViewportGetNameForIndex";
    int _index = 0;
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        DataNode **children = vpData->GetChildren();
        for(int i = 0; i < vpData->GetNumChildren(); ++i)
        {
            if(children[i]->GetNodeType() == INTERNAL_NODE)
            {
                if(_index == index)
                {
                    name = children[i]->GetKey();
                    return true;
                }
                _index++;
            }
        }
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return false;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportActivate
//
// Purpose: 
//   Activates the named viewport.
//
// Arguments:
//   name : The name of the viewport to make active.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:23:52 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportActivate(const std::string &name)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
        activeViewport = vp;
    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportAdd
//
// Purpose: 
//   Adds a new viewport.
//
// Arguments:
//   name : Name of the new viewport.
//   llx : lower left x coordinate [0,1].
//   lly : lower left y coordinate [0,1].
//   urx : upper right x coordinate [0,1].
//   ury : upper right y coordinate [0,1].
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:24:25 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportAdd(const std::string &name, 
    float llx, float lly, float urx, float ury)
{
    const char *mName = "MovieTemplateConfig::ViewportAdd: ";
    bool retval = false;
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        // If the viewport already exists, delete it.
        DataNode *node = vpData->GetNode(name);
        if(node != 0)
            vpData->RemoveNode(name);

        // Add the new viewport.
        DataNode *vp = new DataNode(name);
        vpData->AddNode(vp);

        // Add the fields into the new vp node.
        float coordinates[4];
        coordinates[0] = llx;
        coordinates[1] = lly;
        coordinates[2] = urx;
        coordinates[3] = ury;
        vp->AddNode(new DataNode("coordinates", coordinates, 4));

        vp->AddNode(new DataNode("compositing", 0));
        vp->AddNode(new DataNode("opacity", 1.));
        int tcolor[] = {255,255,255};
        vp->AddNode(new DataNode("replaceColor", tcolor, 3));
        vp->AddNode(new DataNode("dropShadow", false));
        stringVector seqList;
        vp->AddNode(new DataNode("sequenceList", seqList));

        // Record that the new viewport is the active viewport.
        activeViewport = vp;

        retval = true;
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return retval;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportRemove
//
// Purpose: 
//   Removes the named viewport.
//
// Arguments:
//   name : The name of the viewport to remove.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:25:41 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportRemove(const std::string &name)
{
    const char *mName = "MovieTemplateConfig::ViewportRemove: ";
    bool retval = false;

    // Delete the viewport.
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        bool setActive = GetViewport(name) == GetActiveViewport();

        vpData->RemoveNode(name);

        // Set the active viewport to the first one.
        if(setActive)
        {
            activeViewport = 0;
            GetActiveViewport();
        }
        retval = true;
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return retval;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportRemoveAll
//
// Purpose: 
//   Removes all of the viewports.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:26:09 PDT 2006
//
// Modifications:
//   Cyrus Harrison & Brad Whitlock Mon Nov 10 09:59:39 PST 2008
//   Fixed problem with deletion altering the **children array in
//   the middle of removing all viewports, derailing the actual clear.
//
// ****************************************************************************

bool
MovieTemplateConfig::ViewportRemoveAll()
{
    const char *mName = "MovieTemplateConfig::ViewportRemoveAll: ";

    bool retval = false;

    // Delete the viewport.
    DataNode *vpData = GetRootNode()->GetNode("VIEWPORTS");
    if(vpData != 0 && vpData->GetNodeType() == INTERNAL_NODE)
    {
        DataNode **children = vpData->GetChildren();
        int nc = vpData->GetNumChildren();
        // since we will be removing children, and will alter the contents
        // of **children we need to make a copy of the pointers and use them
        // to loop though.
        
        DataNode **copies = new  DataNode*[nc]; 
        memcpy(copies,children,sizeof(DataNode*)*nc);
        for(int i = 0; i < nc; ++i)
        {
            debug5 << mName << "Removing " << copies[i]->GetKey().c_str() << endl;
            vpData->RemoveNode(copies[i]);
        }
        activeViewport = 0;

        delete [] copies;
        
        retval = true;
    }
    else
        debug5 << mName << VIEWPORTS_MISSING_WRONG_TYPE << endl;

    return retval;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetCoordinates
//
// Purpose: 
//   Sets the coordinates for the viewport.
//
// Arguments:
//   name : The name of the viewport to change.
//   llx : lower left x coordinate [0,1].
//   lly : lower left y coordinate [0,1].
//   urx : upper right x coordinate [0,1].
//   ury : upper right y coordinate [0,1].
//  
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:26:37 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetCoordinates(
    float llx, float lly, float urx, float ury)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetCoordinates(GetActiveViewport()->GetKey(),
            llx, lly, urx, ury);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetCoordinates(const std::string &name,
    float llx, float lly, float urx, float ury)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "coordinates" node.
        vp->RemoveNode("coordinates");

        float coords[4];
        coords[0] = llx;
        coords[1] = lly;
        coords[2] = urx;
        coords[3] = ury;
        vp->AddNode(new DataNode("coordinates", coords, 4));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetCoordinates
//
// Purpose: 
//   Get the viewport coordinates.
//
// Arguments:
//   name : The name of the viewport to get.
//   llx : lower left x coordinate [0,1].
//   lly : lower left y coordinate [0,1].
//   urx : upper right x coordinate [0,1].
//   ury : upper right y coordinate [0,1].
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:27:29 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetCoordinates(
    float &llx, float &lly, float &urx, float &ury)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetCoordinates(GetActiveViewport()->GetKey(),
            llx, lly, urx, ury);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetCoordinates(const std::string &name,
    float &llx, float &lly, float &urx, float &ury) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("coordinates");
        if(node != 0 && node->GetNodeType() == FLOAT_ARRAY_NODE)
        {
            const float *fptr = node->AsFloatArray();
            llx = fptr[0];
            lly = fptr[1];
            urx = fptr[2];
            ury = fptr[3];
            ret = true;
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetCompositing
//
// Purpose: 
//   Set the compositing mode for the viewport.
//
// Arguments:
//   name : The viewport that we're interested in.
//   comp : The new compositing mode.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:28:19 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetCompositing(int comp)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetCompositing(GetActiveViewport()->GetKey(), comp);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetCompositing(const std::string &name,
    int comp)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "compositing" node.
        vp->RemoveNode("compositing");
        vp->AddNode(new DataNode("compositing", comp));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetCompositing
//
// Purpose: 
//   Get the compositing mode for a viewport.
//
// Arguments:
//   name : The name of the viewport.
//   comp : Return value for the compositing mode.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:29:09 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetCompositing(int &comp)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetCompositing(GetActiveViewport()->GetKey(), comp);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetCompositing(const std::string &name,
    int &comp) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("compositing");
        if(node != 0 && node->GetNodeType() == INT_NODE)
        {
            comp = node->AsInt();
            ret = true;
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetOpacity
//
// Purpose: 
//   Set the viewport's opacity.
//
// Arguments:
//   name : The name of the viewport.
//   opacity : The new opacity value [0,1].
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:30:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetOpacity(float opacity)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetOpacity(GetActiveViewport()->GetKey(), opacity);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetOpacity(const std::string &name,
    float opacity)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "opacity" node.
        vp->RemoveNode("opacity");
        vp->AddNode(new DataNode("opacity", opacity));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetOpacity
//
// Purpose: 
//   Get a viewport's opacity.
//
// Arguments:
//   name : The name of the viewport to get.
//   opacity : Return value for the opacity.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:31:15 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetOpacity(float &opacity)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetOpacity(GetActiveViewport()->GetKey(), opacity);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetOpacity(const std::string &name,
    float &opacity) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("opacity");
        if(node != 0 && node->GetNodeType() == FLOAT_NODE)
        {
            opacity = node->AsFloat();
            ret = true;
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetDropShadow
//
// Purpose: 
//   Set a viewport's drop shadow flag.
//
// Arguments:
//   name : The name of the viewport.
//   dropShadow : The new drop shadow flag.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:32:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetDropShadow(bool dropShadow)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetDropShadow(GetActiveViewport()->GetKey(), dropShadow);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetDropShadow(const std::string &name,
    bool dropShadow)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "dropShadow" node.
        vp->RemoveNode("dropShadow");
        vp->AddNode(new DataNode("dropShadow", dropShadow));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetDropShadow
//
// Purpose: 
//   Get a viewport's dropshadow flag.
//
// Arguments:
//   name : Name of the viewport to get.
//   dropShadow : Return value for the drop shadow flag.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:32:53 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetDropShadow(bool &dropShadow)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetDropShadow(GetActiveViewport()->GetKey(), dropShadow);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetDropShadow(const std::string &name,
    bool &dropShadow) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("dropShadow");
        if(node != 0 && node->GetNodeType() == BOOL_NODE)
        {
            dropShadow = node->AsBool();
            ret = true;
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetReplaceColor
//
// Purpose: 
//   Set a viewport's replace color value.
//
// Arguments:
//   name         : Name of the viewport.
//   replaceColor : The int[3] color to replace.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:34:11 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetReplaceColor(const int *replaceColor)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetReplaceColor(GetActiveViewport()->GetKey(), replaceColor);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetReplaceColor(const std::string &name,
    const int *replaceColor)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "replaceColor" node.
        vp->RemoveNode("replaceColor");
        vp->AddNode(new DataNode("replaceColor", replaceColor, 3));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetReplaceColor
//
// Purpose: 
//   Gets a viewport's replace color.
//
// Arguments:
//   name : The name of the viewport.
//   replaceColor : The int[3] return value for the color.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:34:58 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetReplaceColor(int *replaceColor)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetReplaceColor(GetActiveViewport()->GetKey(), replaceColor);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetReplaceColor(const std::string &name,
    int *replaceColor) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("replaceColor");
        if(node != 0 && node->GetNodeType() == INT_ARRAY_NODE)
        {
            const int *iptr = node->AsIntArray();
            replaceColor[0] = iptr[0];
            replaceColor[1] = iptr[1];
            replaceColor[2] = iptr[2];
            ret = true;
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportSetSequenceList
//
// Purpose: 
//   Sets a viewport's sequence list.
//
// Arguments:
//   name : The name of the viewport.
//   sequenceList : The list of sequences that will be mapped to the viewport.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:36:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportSetSequenceList(const stringVector &sequenceList)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportSetSequenceList(GetActiveViewport()->GetKey(), sequenceList);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportSetSequenceList(const std::string &name,
    const stringVector &sequenceList)
{
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        // Remove the "sequenceList" node.
        vp->RemoveNode("sequenceList");
        vp->AddNode(new DataNode("sequenceList", sequenceList));
    }

    return vp != 0;
}

// ****************************************************************************
// Method: MovieTemplateConfig::ViewportGetSequenceList
//
// Purpose: 
//   Get a viewport's sequence list.
//
// Arguments:
//   name         : The name of the viewport.
//   sequenceList : The return value for the sequence list.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:36:51 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::ViewportGetSequenceList(stringVector &sequenceList)
{
    bool ret = false;
    if(GetActiveViewport() != 0)
    {
        ret = ViewportGetSequenceList(GetActiveViewport()->GetKey(), sequenceList);
    }
    return ret;
}

bool
MovieTemplateConfig::ViewportGetSequenceList(const std::string &name,
    stringVector &sequenceList) const
{
    bool ret = false;
    DataNode *vp = GetViewport(name);
    if(vp != 0)
    {
        DataNode *node = vp->GetNode("sequenceList");
        if(node != 0 && node->GetNodeType() == STRING_VECTOR_NODE)
        {
            sequenceList = node->AsStringVector();
            ret = true;
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovieTemplateConfig::GetNumberOfSequences
//
// Purpose: 
//   Get the number of sequences.
//
// Returns:    Gets the number of sequences.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:38:42 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
MovieTemplateConfig::GetNumberOfSequences() const
{
    return sequences.size();
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceGetNameForIndex
//
// Purpose: 
//   Get's the i'th sequence name.
//
// Arguments:
//   index : The index of the sequence whose name we want.
//   name  : Return value for the name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:39:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceGetNameForIndex(int index, std::string &name) const
{
    StringMovieSequenceMap::const_iterator it = sequences.begin();
    int i = 0;
    for(int i = 0; it != sequences.end(); ++it, ++i)
    {
        if(index == i)
        {
            name = it->first;
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceGetIndexForName
//
// Purpose: 
//   Get the index of a sequence given its name.
//
// Arguments:
//   name : The name of the sequence.
//   index : Return value for the index.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:40:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceGetIndexForName(const std::string &name, 
    int &index) const
{
    StringMovieSequenceMap::const_iterator it = sequences.begin();
    int i = 0;
    for(int i = 0; it != sequences.end(); ++it, ++i)
    {
        if(it->first == name)
        {
            index = i;
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetActiveSequence
//
// Purpose: 
//   Get a pointer to the active sequence.
//
// Returns:    Pointer to the active sequence.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:41:09 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequence *
MovieTemplateConfig::GetActiveSequence()
{
    if(activeSequence == 0)
    {
        if(sequences.begin() != sequences.end())
            activeSequence = sequences.begin()->second;
    }
    return activeSequence;
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetSequence
//
// Purpose: 
//   Gets a pointer to the named sequence.
//
// Arguments:
//   name : The name of the sequence to get.
//
// Returns:    A pointer to the sequence or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:41:35 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequence *
MovieTemplateConfig::GetSequence(const std::string &name) const
{
    MovieSequence *ret = 0;
    StringMovieSequenceMap::const_iterator it = sequences.find(name);
    if(it != sequences.end())
        ret = it->second;
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceAdd
//
// Purpose: 
//   Adds a new sequence of the specified type.
//
// Arguments:
//   type : The sequence type.
//   name : The name of the new sequence.
//
// Returns:    True on success; False otherwise.
//
// Note:       Integers are appended after the name to ensure that the
//             sequence names are unique.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:42:12 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceAdd(int type, std::string &name)
{
    const char *mName = "MovieTemplateConfig::SequenceAdd1";

    bool ret = false;
    MovieSequence *s = MovieSequenceFactory::Instance()->Create(type);
    if(s != 0)
    {
        // Make up a name
        char tmp[200];
        int index = 1;
        do
        {
            SNPRINTF(tmp, 200, "%s%d", s->SequenceName().c_str(), index++);
        }
        while(GetSequence(tmp) != 0);

        // Add the sequence to the map.
        name = std::string(tmp);
        s->SetName(name);
        sequences[name] = s;
        activeSequence = s;

        ret = true;
    }
    return ret;
}

bool
MovieTemplateConfig::SequenceAdd(const std::string &name, int type)
{
    const char *mName = "MovieTemplateConfig::SequenceAdd2";
    bool ret = false;
    MovieSequence *s = MovieSequenceFactory::Instance()->Create(type);
    if(s != 0)
    {
        // Remove the sequence just in case.
        if(SequenceRemove(name))
        {
            debug1 << mName << "The sequence " << name.c_str()
                   << " was already in the map. We removed it and added "
                   << "it again." << endl;
        }
 
        // Add the sequence to the map.
        s->SetName(name);
        sequences[name] = s;
        activeSequence = s;

        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceRemove
//
// Purpose: 
//   Remove the named sequence.
//
// Arguments:
//   name : The name of the sequence.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:44:19 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceRemove(const std::string &name)
{
    bool ret = false;
    bool setActive = GetSequence(name) == GetActiveSequence();
    StringMovieSequenceMap::iterator it = sequences.find(name);
    if(it != sequences.end())
    {
        delete it->second;
        sequences.erase(it);

        if(setActive)
        {
            activeSequence = 0;
            GetActiveSequence();
        }

        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceRemoveAll
//
// Purpose: 
//   Removes all sequences.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:44:49 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceRemoveAll()
{
    activeSequence = 0;
    StringMovieSequenceMap::iterator it = sequences.begin();
    for(; it != sequences.end(); ++it)
        delete it->second;
    sequences.clear();

    return true;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceActivate
//
// Purpose: 
//   Activate the named sequence.
//
// Arguments:
//   name : The name of the sequence.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:48:11 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceActivate(const std::string &name)
{
    bool ret = false;
    StringMovieSequenceMap::iterator it = sequences.find(name);
    if(it != sequences.end())
    {
        activeSequence = it->second; 
        ret = true;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceRename
//
// Purpose: 
//   Rename a sequence.
//
// Arguments:
//   name    : Current sequence name.
//   newName : New sequence name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:48:51 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceRename(const std::string &name,
    const std::string &newName)
{
    bool ret = false;
    if(newName.size() > 0)
    {
        StringMovieSequenceMap::iterator it = sequences.find(name);
        if(it != sequences.end())
        {
            // Rename the sequence in the sequences map
            MovieSequence *m = it->second;
            sequences.erase(it);
            sequences[newName] = m;
            m->SetName(newName);

            // Rename the sequence in the viewport seqList.
            std::string vpName;
            if(SequenceGetViewport(name, vpName))
            {
                stringVector seqList;
                if(ViewportGetSequenceList(vpName, seqList))
                {
                    stringVector::iterator pos = std::find(
                        seqList.begin(), seqList.end(), name);
                    if(pos != seqList.end())
                    {
                        *pos = newName;
                        ViewportSetSequenceList(vpName, seqList);
                    }
                }
            }

            ret = true;
        }
        return ret;
    }
    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceUnmap
//
// Purpose: 
//   Unmap the sequence from its viewport.
//
// Arguments:
//   seqName : The name of the sequence.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:49:35 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceUnmap(const std::string &seqName)
{
    bool ret = false;

    for(int i = 0; i < GetNumberOfViewports(); ++i)
    {
        std::string vpName;
        if(ViewportGetNameForIndex(i, vpName))
        {
            stringVector seqList;
            if(ViewportGetSequenceList(vpName, seqList))
            {
                // Look for the sequence name in the list
                stringVector::iterator pos =
                    std::find(seqList.begin(), seqList.end(), seqName);

                // If it was found, erase it.
                if(pos != seqList.end())
                {
                    seqList.erase(pos);
                    ret |= ViewportSetSequenceList(vpName, seqList);
                }
            }
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceMapToViewport
//
// Purpose: 
//   Maps a sequence to a viewport.
//
// Arguments:
//   seqName : The name of the sequence.
//   vpName  : The name of the viewport to which the sequence will be mapped.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:50:12 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceMapToViewport(const std::string &seqName,
    const std::string &vpName)
{
    bool ret = false;

    // If the sequence exists then
    if(GetSequence(seqName) != 0)
    {
        // Unmap the sequence in case it is already mapped.
        ret = SequenceUnmap(seqName);

        stringVector seqList;
        if(ViewportGetSequenceList(vpName, seqList))
        {
            seqList.push_back(seqName);
            ret |= ViewportSetSequenceList(vpName, seqList);
        }
    }

    return ret;
}

// ****************************************************************************
// Method: MovieTemplateConfig::SequenceGetViewport
//
// Purpose: 
//   Get the name of the viewport to which a sequence is mapped.
//
// Arguments:
//   seqName : The name of the sequence.
//   vpName  : Return value for the viewport name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:50:56 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieTemplateConfig::SequenceGetViewport(const std::string &seqName,
    std::string &vpName)
{
    bool ret = false;

    for(int i = 0; i < GetNumberOfViewports(); ++i)
    {
        if(ViewportGetNameForIndex(i, vpName))
        {
            stringVector seqList;
            if(ViewportGetSequenceList(vpName, seqList))
            {
                // Look for the sequence name in the list
                stringVector::iterator pos =
                    std::find(seqList.begin(), seqList.end(), seqName);

                // If it was found, erase it.
                if(pos != seqList.end())
                {
                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;    
}

// ****************************************************************************
// Method: MovieTemplateConfig::GetNumberOfMappedSequences
//
// Purpose: 
//   Get the number of mapped sequences.
//
// Returns:    Number of mapped sequences.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:51:42 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
MovieTemplateConfig::GetNumberOfMappedSequences() const
{
    int c = 0;
    for(int j = 0; j < GetNumberOfViewports(); ++j)
    {
        std::string vpName;
        stringVector seqList;
        if(ViewportGetNameForIndex(j, vpName) &&
           ViewportGetSequenceList(vpName, seqList))
        {
            if(seqList.size() > 0)
                ++c;
        }
    }

    return c;
}

