// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: InteractorAttributes
//
// Purpose:
//    This class contains attributes associated with the main window.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class InteractorAttributes extends AttributeSubject
{
    private static int InteractorAttributes_numAdditionalAtts = 6;

    // Enum values
    public final static int NAVIGATIONMODE_TRACKBALL = 0;
    public final static int NAVIGATIONMODE_DOLLY = 1;
    public final static int NAVIGATIONMODE_FLYTHROUGH = 2;

    public final static int BOUNDINGBOXMODE_ALWAYS = 0;
    public final static int BOUNDINGBOXMODE_NEVER = 1;
    public final static int BOUNDINGBOXMODE_AUTO = 2;


    public InteractorAttributes()
    {
        super(InteractorAttributes_numAdditionalAtts);

        showGuidelines = true;
        clampSquare = false;
        fillViewportOnZoom = true;
        navigationMode = NAVIGATIONMODE_TRACKBALL;
        axisArraySnap = true;
        boundingBoxMode = BOUNDINGBOXMODE_AUTO;
    }

    public InteractorAttributes(int nMoreFields)
    {
        super(InteractorAttributes_numAdditionalAtts + nMoreFields);

        showGuidelines = true;
        clampSquare = false;
        fillViewportOnZoom = true;
        navigationMode = NAVIGATIONMODE_TRACKBALL;
        axisArraySnap = true;
        boundingBoxMode = BOUNDINGBOXMODE_AUTO;
    }

    public InteractorAttributes(InteractorAttributes obj)
    {
        super(obj);

        showGuidelines = obj.showGuidelines;
        clampSquare = obj.clampSquare;
        fillViewportOnZoom = obj.fillViewportOnZoom;
        navigationMode = obj.navigationMode;
        axisArraySnap = obj.axisArraySnap;
        boundingBoxMode = obj.boundingBoxMode;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return InteractorAttributes_numAdditionalAtts;
    }

    public boolean equals(InteractorAttributes obj)
    {
        // Create the return value
        return ((showGuidelines == obj.showGuidelines) &&
                (clampSquare == obj.clampSquare) &&
                (fillViewportOnZoom == obj.fillViewportOnZoom) &&
                (navigationMode == obj.navigationMode) &&
                (axisArraySnap == obj.axisArraySnap) &&
                (boundingBoxMode == obj.boundingBoxMode));
    }

    // Property setting methods
    public void SetShowGuidelines(boolean showGuidelines_)
    {
        showGuidelines = showGuidelines_;
        Select(0);
    }

    public void SetClampSquare(boolean clampSquare_)
    {
        clampSquare = clampSquare_;
        Select(1);
    }

    public void SetFillViewportOnZoom(boolean fillViewportOnZoom_)
    {
        fillViewportOnZoom = fillViewportOnZoom_;
        Select(2);
    }

    public void SetNavigationMode(int navigationMode_)
    {
        navigationMode = navigationMode_;
        Select(3);
    }

    public void SetAxisArraySnap(boolean axisArraySnap_)
    {
        axisArraySnap = axisArraySnap_;
        Select(4);
    }

    public void SetBoundingBoxMode(int boundingBoxMode_)
    {
        boundingBoxMode = boundingBoxMode_;
        Select(5);
    }

    // Property getting methods
    public boolean GetShowGuidelines() { return showGuidelines; }
    public boolean GetClampSquare() { return clampSquare; }
    public boolean GetFillViewportOnZoom() { return fillViewportOnZoom; }
    public int     GetNavigationMode() { return navigationMode; }
    public boolean GetAxisArraySnap() { return axisArraySnap; }
    public int     GetBoundingBoxMode() { return boundingBoxMode; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteBool(showGuidelines);
        if(WriteSelect(1, buf))
            buf.WriteBool(clampSquare);
        if(WriteSelect(2, buf))
            buf.WriteBool(fillViewportOnZoom);
        if(WriteSelect(3, buf))
            buf.WriteInt(navigationMode);
        if(WriteSelect(4, buf))
            buf.WriteBool(axisArraySnap);
        if(WriteSelect(5, buf))
            buf.WriteInt(boundingBoxMode);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetShowGuidelines(buf.ReadBool());
            break;
        case 1:
            SetClampSquare(buf.ReadBool());
            break;
        case 2:
            SetFillViewportOnZoom(buf.ReadBool());
            break;
        case 3:
            SetNavigationMode(buf.ReadInt());
            break;
        case 4:
            SetAxisArraySnap(buf.ReadBool());
            break;
        case 5:
            SetBoundingBoxMode(buf.ReadInt());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + boolToString("showGuidelines", showGuidelines, indent) + "\n";
        str = str + boolToString("clampSquare", clampSquare, indent) + "\n";
        str = str + boolToString("fillViewportOnZoom", fillViewportOnZoom, indent) + "\n";
        str = str + indent + "navigationMode = ";
        if(navigationMode == NAVIGATIONMODE_TRACKBALL)
            str = str + "NAVIGATIONMODE_TRACKBALL";
        if(navigationMode == NAVIGATIONMODE_DOLLY)
            str = str + "NAVIGATIONMODE_DOLLY";
        if(navigationMode == NAVIGATIONMODE_FLYTHROUGH)
            str = str + "NAVIGATIONMODE_FLYTHROUGH";
        str = str + "\n";
        str = str + boolToString("axisArraySnap", axisArraySnap, indent) + "\n";
        str = str + indent + "boundingBoxMode = ";
        if(boundingBoxMode == BOUNDINGBOXMODE_ALWAYS)
            str = str + "BOUNDINGBOXMODE_ALWAYS";
        if(boundingBoxMode == BOUNDINGBOXMODE_NEVER)
            str = str + "BOUNDINGBOXMODE_NEVER";
        if(boundingBoxMode == BOUNDINGBOXMODE_AUTO)
            str = str + "BOUNDINGBOXMODE_AUTO";
        str = str + "\n";
        return str;
    }


    // Attributes
    private boolean showGuidelines;
    private boolean clampSquare;
    private boolean fillViewportOnZoom;
    private int     navigationMode;
    private boolean axisArraySnap;
    private int     boundingBoxMode;
}

