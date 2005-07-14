import java.lang.ArrayIndexOutOfBoundsException;
import llnl.visit.AttributeSubject;
import llnl.visit.ColorAttribute;
import llnl.visit.LightList;
import llnl.visit.LightAttributes;
import llnl.visit.SimpleObserver;
import llnl.visit.View3DAttributes;

import llnl.visit.plots.PseudocolorAttributes;

// ****************************************************************************
// Class: TryLighting
//
// Purpose:
//   This example program sets the view and turns on some colored lights. It
//   also shows how to observe the viewer's state objects so actions can be
//   performed when new state arrives.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 15 16:10:44 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 24 09:08:51 PDT 2002
//   I fixed the example so the lights work as intended.
//
//   Brad Whitlock, Thu Dec 12 10:44:31 PDT 2002
//   Updated because of changse to color table methods.
//
//   Eric Brugger, Wed Aug 27 09:06:38 PDT 2003
//   I modified it to use the new view interface.
//
//   Brad Whitlock, Mon Jun 6 17:25:34 PST 2005
//   I made it use GetDataPath to locate the data.
//
//   Brad Whitlock, Thu Jul 14 12:08:42 PDT 2005
//   I made it set the Pseudocolor plot atts's color table to "Default".
//
// ****************************************************************************

public class TryLighting extends RunViewer implements SimpleObserver
{
    public TryLighting()
    {
        super();
        doUpdate = true;

        // Make this object observe the light attributes.
        viewer.GetLightList().Attach(this);
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            viewer.AddPlot("Pseudocolor", "w");

            // Set the pseudocolor attributes
            PseudocolorAttributes p = (PseudocolorAttributes)viewer.GetPlotAttributes("Pseudocolor");
            p.SetColorTableName("Default");
            p.SetOpacity(1.);
            p.Notify();
            viewer.SetPlotOptions("Pseudocolor");
            viewer.DrawPlots();

            // Set the colortable to one that has white at the bottom values.
            viewer.SetActiveContinuousColorTable("calewhite");

            // Set the view
            View3DAttributes v = viewer.GetView3D();
            v.SetViewNormal(0.456808, 0.335583, 0.823839);
            v.SetFocus(-0.927295, -1.22113, 1.01159);
            v.SetViewUp(-0.184554, 0.941716, -0.281266);
            v.SetParallelScale(15.7041);
            v.SetNearPlane(-34.641);
            v.SetFarPlane(34.641);
            v.Notify();
            viewer.SetView3D();

            LightList ll = viewer.GetLightList();
            ll.SetAllEnabled(false);

            // Create a red light
            System.out.println("Setting up red light.");
            LightAttributes newLight1 = new LightAttributes();
            newLight1.SetType(LightAttributes.LIGHTTYPE_OBJECT);
            newLight1.SetDirection(0,0,-1);
            newLight1.SetColor(new ColorAttribute(255,0,0));
            newLight1.SetEnabledFlag(true);
            ll.SetLight0(newLight1);
            ll.Notify();
            viewer.SetLightList();
            viewer.SaveWindow();

            // Create a green light
            System.out.println("Setting up green light.");
            LightAttributes newLight2 = new LightAttributes();
            newLight2.SetType(LightAttributes.LIGHTTYPE_OBJECT);
            newLight2.SetDirection(-1,0,0);
            newLight2.SetColor(new ColorAttribute(0,255,0));
            newLight2.SetEnabledFlag(true);
            ll.SetLight1(newLight2);
            ll.Notify();
            viewer.SetLightList();
            viewer.SaveWindow();

            // Create a blue light
            System.out.println("Setting up blue light.");
            LightAttributes newLight3 = new LightAttributes();
            newLight3.SetType(LightAttributes.LIGHTTYPE_OBJECT);
            newLight3.SetDirection(0,-1,0);
            newLight3.SetColor(new ColorAttribute(0,0,255));
            newLight3.SetEnabledFlag(true);
            ll.SetLight2(newLight3);
            ll.Notify();
            viewer.SetLightList();
            viewer.SaveWindow();
        }
        else
            System.out.println("Could not open the database!");
    }

    public void Update(AttributeSubject s)
    {
        LightList ll = (LightList)s;
        printLights(ll);
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    protected void printLights(LightList ll)
    {
        printLight(0, ll.GetLight0());
        printLight(1, ll.GetLight1());
        printLight(2, ll.GetLight2());
        printLight(3, ll.GetLight3());
        printLight(4, ll.GetLight4());
        printLight(5, ll.GetLight5());
        printLight(6, ll.GetLight6());
        printLight(7, ll.GetLight7());
        System.out.println("");
    }

    protected void printLight(int index, LightAttributes l)
    {
        System.out.println("Light["+index+"] = {");
        System.out.println("    enabled = "+l.GetEnabledFlag());
        int t = l.GetType();
        double[] d = l.GetDirection();
        switch(t)
        {
        case LightAttributes.LIGHTTYPE_AMBIENT:
            System.out.println("    type = AMBIENT");
            break;
        case LightAttributes.LIGHTTYPE_OBJECT:
            System.out.println("    type = OBJECT");
            System.out.println("    direction = {"+d[0]+", "+d[1]+", "+d[2]+"}");
            break;
        case LightAttributes.LIGHTTYPE_CAMERA:
            System.out.println("    type = CAMERA");
            System.out.println("    direction = {"+d[0]+", "+d[1]+", "+d[2]+"}");
        }
        ColorAttribute c = l.GetColor();
        System.out.println("    color = {"+c.Red()+", "+c.Green()+", "+c.Blue()+"}");
        System.out.println("    brightness = "+l.GetBrightness());
        System.out.println("}");
    }

    public static void main(String args[])
    {
        TryLighting r = new TryLighting();
        r.run(args);
    }

    private boolean doUpdate;
}
