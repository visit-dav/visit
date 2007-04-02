import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.*;

import java.util.Vector;

import llnl.visit.ViewerProxy;
import llnl.visit.SaveWindowAttributes;
import llnl.visit.ColorAttribute;
// For setting the Curve plot's attributes.
import llnl.visit.plots.CurveAttributes;

// ****************************************************************************
// Class: NIFGUI
//
// Purpose:
//   This class uses the VisIt Java ViewerProxy class in a GUI setting to
//   create images that can be imported as GUI elements into a Java
//   application.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 8 08:51:58 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jun 6 17:29:15 PST 2005
//   I made it use GetDataPath to locate some default data.
//
// ****************************************************************************

public class NIFGUI extends JPanel implements Runnable, ActionListener, ItemListener
{
    // Constants
    protected String newline = "\n";
    protected static String imagepath = "images/";

    // GUI components
    protected JTextArea output;
    protected JScrollPane scrollPane;
    protected ImageIcon visitButtonIcon1, visitButtonIcon2, visitButtonIcon3;
    protected JButton visitButton1, visitButton2, visitButton3;

    // VisIt data
    protected ViewerProxy viewer;
    protected boolean     visitLaunched;
    protected boolean     visitVisible;
    protected String      visitBinPath;
    protected String      visitDatabase;
    protected String      visitPlotVar;
    protected int         visitImageWidth;
    protected int         visitImageHeight;
    protected int         visitImageCount;
    protected boolean     visitOpenDatabaseAtStart;
    protected int         visitActiveButton;

    //
    // Constructor
    //
    public NIFGUI(String[] args)
    {
        output = null;

        viewer = new ViewerProxy();
        // Default values
        visitLaunched = false;
        visitVisible = false;
        visitBinPath = new String("/usr/gapps/visit/bin");
        visitDatabase = new String(viewer.GetDataPath() + "noise.silo");
        visitPlotVar  = new String("hgslice");
        visitImageWidth = 300;
        visitImageHeight = 300;
        visitImageCount = 0;
        visitOpenDatabaseAtStart = false;
        visitActiveButton = -1;

        // Pass command line options to the viewer viewer
        boolean verbose = false;
        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("-path") && ((i + 1) < args.length))
            {
                viewer.SetBinPath(args[i + 1]);
                ++i;
            }
            else if(args[i].equals("-o") && ((i + 1) < args.length))
            {
                visitDatabase = new String(args[i + 1]);
                visitOpenDatabaseAtStart = true;
                ++i;
            }
            else if(args[i].equals("-help"))
            {
                printUsage();
            }
            else if(args[i].equals("-verbose"))
            {
                verbose = true;
            }
            else
                viewer.AddArgument(args[i]);
        }

        // Set the viewer proxy's verbose flag.
        viewer.SetVerbose(verbose);
    }

    //
    // Print program usage.
    //
    protected void printUsage()
    {
        System.out.println("Options:");
        System.out.println("    -verbose   Prints extra VisIt viewer information to the console.");
        System.out.println("    -path dir  Sets the directory that is searched for the visit script.");
        System.out.println("    -help      Displays options.");
    }

    //
    // Creates the menu bar options.
    // 
    public JMenuBar createMenuBar()
    {
        JMenuBar menuBar;
        JMenu menu, submenu;
        JMenuItem menuItem;
        JRadioButtonMenuItem rbMenuItem;
        JCheckBoxMenuItem cbMenuItem;

        // Create the menu bar.
        menuBar = new JMenuBar();

        // Build the File menu.
        menu = new JMenu("File");
        menu.setMnemonic(KeyEvent.VK_A);
        menu.getAccessibleContext().setAccessibleDescription(
                "Open a database or quit the application");
        menuBar.add(menu);

        // Open file option
        menuItem = new JMenuItem("Open file ...",
                                 KeyEvent.VK_O);
        menuItem.setAccelerator(KeyStroke.getKeyStroke(
                KeyEvent.VK_O, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription(
                "Open a file");
        menuItem.setActionCommand("open");
        menuItem.addActionListener(this);
        menu.add(menuItem);

        // Separate open from quit.
        menu.addSeparator();

        // Quit option.
        menuItem = new JMenuItem("Quit",
                                 KeyEvent.VK_Q);
        menuItem.setAccelerator(KeyStroke.getKeyStroke(
                KeyEvent.VK_Q, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription(
                "Quit the application");
        menuItem.setActionCommand("quit");
        menuItem.addActionListener(this);
        menu.add(menuItem);

        return menuBar;
    }

    //
    // Creates the main application page body
    //
    public Container createContentPane()
    {
        //Create the content-pane-to-be.
        JPanel contentPane = new JPanel(new BorderLayout());
        contentPane.setOpaque(true);

        Box box = new Box(BoxLayout.Y_AXIS);
        contentPane.add(box);

        visitButtonIcon1 = GetImageIcon(ButtonFileName(1));
        visitButton1 = new JButton("2D plot", visitButtonIcon1);
        visitButton1.setVerticalTextPosition(AbstractButton.TOP);
        visitButton1.setHorizontalTextPosition(AbstractButton.CENTER);
        visitButton1.setActionCommand("update1");
        visitButton1.addActionListener(this);
        box.add(visitButton1);

        visitButtonIcon2 = GetImageIcon(ButtonFileName(2));
        visitButton2 = new JButton("Lineout plot", visitButtonIcon2);
        visitButton2.setVerticalTextPosition(AbstractButton.TOP);
        visitButton2.setHorizontalTextPosition(AbstractButton.CENTER);
        visitButton2.setActionCommand("update2");
        visitButton2.addActionListener(this);
        box.add(visitButton2);

        visitButtonIcon3 = GetImageIcon(ButtonFileName(3));
        visitButton3 = new JButton("Surface plot", visitButtonIcon3);
        visitButton3.setVerticalTextPosition(AbstractButton.TOP);
        visitButton3.setHorizontalTextPosition(AbstractButton.CENTER);
        visitButton3.setActionCommand("update3");
        visitButton3.addActionListener(this);
        box.add(visitButton3);

        //Create a scrolled text area.
        output = new JTextArea(5, 30);
        output.setEditable(false);
        scrollPane = new JScrollPane(output);

        //Add the text area to the content pane.
        box.add(scrollPane, BorderLayout.CENTER);

        return contentPane;
    }

    //
    // This method is called when a GUI component is used, clicked, whatever.
    //
    public void actionPerformed(ActionEvent e)
    {
        // Tell us which menu option was clicked.
        String s = "Action event detected: " + e.getActionCommand();
        addOutput(s);

        // Don't know a good Java/Swing way to do this...
        if(e.getActionCommand().equals("open"))
            OpenFile();
        else if(e.getActionCommand().equals("quit"))
            Quit();
        else if(e.getActionCommand().equals("update1"))
        {
            System.out.println("Button1 Clicked!\n");
            ButtonClicked1();
        }
        else if(e.getActionCommand().equals("update2"))
        {
            ButtonClicked2();
        }
        else if(e.getActionCommand().equals("update3"))
        {
            ButtonClicked3();
        }
    }

    //
    // Gets the name of an image file to use for the specified button. Note that
    // ideally the method should always return the same image file name for the
    // specified button but I could not get the button to change without also
    // changing the name of the file that we load for a button. Do this for now.
    //
    protected String ButtonFileName(int button)
    {
        String s = new String(imagepath + "button" + button + "_" + visitImageCount + ".jpeg");
        ++visitImageCount;
        return s;
    }

    //
    // Launches VisIt if it has not been launched yet.
    //
    protected boolean EnsureVisItExists()
    {
        if(!visitLaunched)
        {
            addOutput("Trying to launch VisIt");

            // Try and launch the viewer.
            if(viewer.Create(5600))
            {
                addOutput("ViewerProxy opened the viewer.");
                visitLaunched = true;

                // Tell the viewer proxy to start its input processing thread
                // so it can automatically read input from VisIt's viewer.
                viewer.StartProcessing();
            }
            else
                addOutput("ViewerProxy could not launch the viewer.");
        }

        return visitLaunched;
    }

    //
    // Tell VisIt to save the active window as a 300x300 JPEG image.
    //
    protected void SaveWindow(String imageName)
    {
        SaveWindowAttributes s = viewer.GetSaveWindowAttributes();
        s.SetWidth(visitImageWidth);
        s.SetHeight(visitImageHeight);
        s.SetFamily(false);
        s.SetScreenCapture(false);
        s.SetSaveTiled(false);
        s.SetFormat(s.FILEFORMAT_JPEG);
        s.SetOutputToCurrentDirectory(true);
        s.SetFileName(imageName);
        s.Notify();
        viewer.SaveWindow();
        viewer.Synchronize();
    }

    //
    // Tell VisIt to delete all of its plots.
    //
    protected void DeleteAllPlots()
    {
        int nplots = viewer.GetPlotList().GetNumPlots();
        if(nplots > 0)
        {
            int ids[] = new int[nplots];
            for(int i = 0; i < nplots; ++i)
                ids[i] = i;
            viewer.SetActivePlots(ids);
            viewer.DeleteActivePlots();
        }
    }

    //
    // Tell VisIt to delete all windows but window 1. This is in case the
    // user created more windows while VisIt was open interactively.
    //
    protected void DeleteAllButWindow1()
    {
        Vector windows = new Vector(viewer.GetGlobalAttributes().GetWindows());
        for(int i = 0; i < windows.size(); ++i)
        {
            int window = ((Integer)windows.elementAt(i)).intValue();
            if(window != 1)
            {
                viewer.SetActiveWindow(window);
                viewer.DeleteWindow();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    protected void ButtonClicked1()
    {
        if(!EnsureVisItExists())
        {
            addOutput("UpdateButton1: Do nothing. VisIt is not launched.");
            return;
        }

        if(!visitDatabase.equals(""))
        {
            if(visitVisible && visitActiveButton == 1)
            {
                UpdateButton1();
                DeleteAllButWindow1();
                DeleteAllPlots();
                viewer.HideAllWindows();
                visitVisible = false;
                visitActiveButton = -1;
            }
            else
            {
                visitActiveButton = 1;
                viewer.ShowAllWindows();
                viewer.Synchronize();

                visitVisible = true;
                DeleteAllButWindow1();
                DeleteAllPlots();
                SetupPlots1();
            }
        }
        else
            addOutput("UpdateButton1: You have not chosen a database!");
    }

    protected void SetupPlots1()
    {
        //
        // Tell VisIt to open the database and create a Pseudocolor plot. You
        // can programmatically tell VisIt to do complex things but you might
        // want to tell it to restore a session file here instead.
        //
        viewer.OpenDatabase(visitDatabase);
        viewer.AddPlot("Pseudocolor", visitPlotVar);
        viewer.DrawPlots();
    }

    protected void UpdateButton1()
    {
        // Save the image in VisIt
        String imageName = ButtonFileName(1);
        SaveWindow(imageName);

        // Get a new version of the icon for the button.
        visitButtonIcon1 = GetImageIcon(imageName);
        visitButton1.setIcon(visitButtonIcon1);
    }

    ////////////////////////////////////////////////////////////////////////////

    protected void ButtonClicked2()
    {
        if(!EnsureVisItExists())
        {
            addOutput("UpdateButton2: Do nothing. VisIt is not launched.");
            return;
        }

        if(!visitDatabase.equals(""))
        {
            if(visitVisible && visitActiveButton == 2)
            {
                UpdateButton2();
                DeleteAllButWindow1();
                DeleteAllPlots();
                viewer.HideAllWindows();
                visitVisible = false;
                visitActiveButton = -1;
            }
            else
            {
                visitActiveButton = 2;
                viewer.ShowAllWindows();
                viewer.Synchronize();

                visitVisible = true;
                DeleteAllButWindow1();
                DeleteAllPlots();
                SetupPlots2();
            }
        }
        else
            addOutput("UpdateButton2: You have not chosen a database!");
    }

    protected void SetupPlots2()
    {
        // Do the same plot as in window 1
        SetupPlots1();

        // Perform a lineout, which will result in a second vis window getting generated.
        Vector vars = new Vector();
        vars.addElement(new String("default"));
        viewer.Lineout(-4.17647, 9.14941, -0.905884, -9.60353, vars);
        viewer.Synchronize();

        try
        {
            // Go to window 2
            viewer.SetActiveWindow(2);

            // Set the Curve plot attributes
            CurveAttributes curveAtts = (CurveAttributes)viewer.GetPlotAttributes("Curve"); 
            curveAtts.SetLineWidth(2);
            curveAtts.SetShowPoints(true);
            curveAtts.SetColor(new ColorAttribute(0,0,255,255));
            curveAtts.Notify();
            viewer.SetPlotOptions("Curve");
        }
        catch(ArrayIndexOutOfBoundsException e)
        {
            // Delete all of the plots.
            addOutput("Could not get the Curve plot attributes.");
        }
 
        viewer.DrawPlots();
    }

    protected void UpdateButton2()
    {
        // Save the image in VisIt
        String imageName = ButtonFileName(2);
        SaveWindow(imageName);

        // Get a new version of the icon for the button.
        visitButtonIcon2 = GetImageIcon(imageName);
        visitButton2.setIcon(visitButtonIcon2);
    }

    ////////////////////////////////////////////////////////////////////////////

    protected void ButtonClicked3()
    {
        if(!EnsureVisItExists())
        {
            addOutput("UpdateButton3: Do nothing. VisIt is not launched.");
            return;
        }

        if(!visitDatabase.equals(""))
        {
            if(visitVisible && visitActiveButton == 3)
            {
                UpdateButton3();
                DeleteAllButWindow1();
                DeleteAllPlots();
                viewer.HideAllWindows();
                visitVisible = false;
                visitActiveButton = -1;
            }
            else
            {
                visitActiveButton = 3;
                viewer.ShowAllWindows();
                viewer.Synchronize();

                visitVisible = true;
                DeleteAllButWindow1();
                DeleteAllPlots();
                SetupPlots3();
            }
        }
        else
            addOutput("UpdateButton3: You have not chosen a database!");
    }

    protected void SetupPlots3()
    {
        //
        // Tell VisIt to open the database and create a Surface plot. You
        // can programmatically tell VisIt to do complex things but you might
        // want to tell it to restore a session file here instead.
        //
        viewer.OpenDatabase(visitDatabase);
        viewer.AddPlot("Surface", visitPlotVar);
        viewer.DrawPlots();
    }

    protected void UpdateButton3()
    {
        // Save the image in VisIt
        String imageName = ButtonFileName(3);
        SaveWindow(imageName);

        // Get a new version of the icon for the button.
        visitButtonIcon3 = GetImageIcon(imageName);
        visitButton3.setIcon(visitButtonIcon3);
    }

    ////////////////////////////////////////////////////////////////////////////

    //
    // This method is called when we open a new file using the file dialog so we
    // can make VisIt recreate all of its plots and update the buttons.
    //
    protected void UpdateAllButtons()
    {
        SetupPlots1();
        UpdateButton1();

        SetupPlots2();
        UpdateButton2();

        SetupPlots3();
        UpdateButton3();
    }

    //
    // This method adds messages to the output area.
    //
    protected void addOutput(String s)
    {
        if(output == null)
            System.out.println(s + newline);
        else
        {
            output.append(s + newline);
            output.setCaretPosition(output.getDocument().getLength());
        }
    }

    public void itemStateChanged(ItemEvent e)
    {
        JMenuItem source = (JMenuItem)(e.getSource());
        String s = "Item event detected."
                   + newline
                   + "    Event source: " + source.getText()
                   + " (an instance of " + getClassName(source) + ")"
                   + newline
                   + "    New state: "
                   + ((e.getStateChange() == ItemEvent.SELECTED) ?
                     "selected":"unselected");
        addOutput(s);
    }

    //
    // Returns just the class name -- no package info.
    //
    protected String getClassName(Object o)
    {
        String classString = o.getClass().getName();
        int dotIndex = classString.lastIndexOf(".");
        return classString.substring(dotIndex+1);
    }

    //
    // Called when we need to open a file.
    //
    protected void OpenFile()
    {
        addOutput("Open a file!!!");
        JFileChooser fc = new JFileChooser();
        int returnVal = fc.showOpenDialog(NIFGUI.this);
        if (returnVal == JFileChooser.APPROVE_OPTION)
        {
            File file = fc.getSelectedFile();
            // This is where a real application would open the file.
            addOutput("Opening: " + file.getAbsolutePath() + ".");

            // Set the VisIt database and try to update all of the buttons.
            visitDatabase = file.getAbsolutePath();
            UpdateAllButtons();
        }
        else
        {
            addOutput("Open command cancelled by user.");
        }
    }

    //
    // Called when we want to quit the application.
    //
    protected void Quit()
    {
        addOutput("Quit the application!!!");

        // Tell VisIt to shut down.
        viewer.Close();

        // How do you shut down SWING?
    }

    //
    // Returns an ImageIcon, or null if the path was invalid.
    //
    protected static ImageIcon GetImageIcon(String path)
    {
        java.net.URL imgURL = NIFGUI.class.getResource(path);
        if (imgURL != null)
        {
            return new ImageIcon(imgURL);
        }
        else
        {
            java.net.URL noImageURL = NIFGUI.class.getResource(imagepath + "noimage.jpeg");
            if (noImageURL != null)
            {
                return new ImageIcon(noImageURL);
            }

            System.err.println("Couldn't find file: " + path);
            return null;
        }
    }

    //
    // Create the GUI and show it.  For thread safety,
    // this method should be invoked from the
    // event-dispatching thread.
    //
    private void createAndShowGUI()
    {
        //Make sure we have nice window decorations.
        JFrame.setDefaultLookAndFeelDecorated(true);

        //Create and set up the window.
        JFrame frame = new JFrame("NIFGUI");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setJMenuBar(createMenuBar());
        frame.setContentPane(createContentPane());

        //Display the window.
        frame.setSize(450, 260);
        frame.setVisible(true);
    }

    //
    // Entry point into the Runnable interface so we can start this class's
    // event loop using the javax.swing.SwingUtilities.invokeLater
    // method, which seems common for SWING applications.
    //
    public void run()
    {
        // Launch the viewer now and keep it around since VisIt is kinda slow
        // to launch currently.
        EnsureVisItExists();

        // Create and show the user interface.
        createAndShowGUI();

        // If we specified the -o option on the command line then update all
        // of the buttons using that database.
        if(visitOpenDatabaseAtStart)
            UpdateAllButtons();
    }

    //
    // Main application entry function.
    //
    public static void main(String[] args)
    {
        //Schedule a job for the event-dispatching thread:
        //creating and showing this application's GUI.
        NIFGUI obj = new NIFGUI(args);
        javax.swing.SwingUtilities.invokeLater(obj);
    }
}
