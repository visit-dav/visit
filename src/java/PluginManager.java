package llnl.visit;

import java.lang.ArrayIndexOutOfBoundsException;
import java.lang.InstantiationException;
import java.lang.IllegalAccessException;

// ****************************************************************************
// Class: PluginManager
//
// Purpose:
//   This is a plugin manager that dynamically loads class files based on
//   the plugins that are loaded in the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 17:02:52 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Nov 6 11:12:03 PDT 2002
//   I fixed it so that multiple paths in the class path are supported. This
//   allows the plugin manager to load classes from jar files.
//
//   Brad Whitlock, Fri Dec 13 08:43:10 PDT 2002
//   I fixed it so that the plugin list is sorted when it is created to
//   guarantee that the plugins are always loaded in the proper order.
//
//   Brad Whitlock, Mon Jun 6 18:20:35 PST 2005
//   I made it use the right path separator so it works on Windows.
//
//   John Carlson, Tue Oct 25 15:34:23 PST 2005
//   Made it use Class.forName instead of a URL class loader. This makes sure
//   that the class can load its plugins when it is used in an applet.
//
//   Brad Whitlock, Thu Oct 27 10:29:23 PDT 2005
//   I made it try up to 3 different class names to account for some new
//   variability in class naming conventions for operator state objects. In
//   addition, the class can now successfully skip loading a plugin without
//   preventing all plugins from failing to load.
//
// ****************************************************************************

class PluginManager
{
    public PluginManager(String t)
    {
        pluginType = new String(t);
        pluginsLoaded = false;
        verbose = false;
    }

    public void SetVerbose(boolean val)
    {
        verbose = val;
    }

    public String GetPluginName(int i) throws ArrayIndexOutOfBoundsException
    {
        if(i < 0 || i >= pluginAtts.length)
            throw new ArrayIndexOutOfBoundsException();
        return pluginNames[i];
    }

    public String GetPluginVersion(int i) throws ArrayIndexOutOfBoundsException
    {
        if(i < 0 || i >= pluginAtts.length)
            throw new ArrayIndexOutOfBoundsException();
        return pluginVersions[i];
    }

    public Plugin GetPluginAttributes(int i) throws ArrayIndexOutOfBoundsException
    {
        if(i < 0 || i >= pluginAtts.length)
            throw new ArrayIndexOutOfBoundsException();
        return pluginAtts[i];
    }

    public Plugin CreatePluginAttributes(int i) throws ArrayIndexOutOfBoundsException,
        InstantiationException, IllegalAccessException
    {
        if(i < 0 || i >= pluginAtts.length)
            throw new ArrayIndexOutOfBoundsException();
        return (Plugin)pluginAtts[i].getClass().newInstance();
    }

    public int GetNumPlugins()
    {
        return pluginsLoaded ? pluginNames.length : 0;
    }

    public int IndexFromName(String name)
    {
        int retval = -1;
        for(int i = 0; i < pluginNames.length; ++i)
        {
            if(name.equals(pluginNames[i]))
            {
                retval = i;
                break;
            }
        }

        return retval;
    }

    private void PopulatePluginList(PluginManagerAttributes atts, String[] loadList)
    {
        int nPlugins = 0;

        // Look through the list of plugins in the atts and add them sorted to
        // arrays that are passed in.
        for(int i = 0; i < atts.GetName().size(); ++i)
        {
            if(WantPlugin(atts, i))
            {
                String name = (String)atts.GetName().elementAt(i);

                // Look for a place in the array for the new item.
                int place = -1;                
                for(int j = 0; j < nPlugins; ++j)
                {
                    if(name.compareTo(loadList[j]) < 0)
                    {
                        place = j;
                        break;
                    }
                }

                if(place == -1)
                    place = nPlugins;
                else
                {
                    // Make room for the item.
                    for(int k = nPlugins; k > place; --k)
                        loadList[k] = new String(loadList[k-1]);
                }

                // Insert item into the list
                loadList[place] = new String(name);

                ++nPlugins;
            }
        }        
    }

    public boolean LoadPlugins(PluginManagerAttributes atts)
    {
	if(verbose)
            System.out.println("Loading plugins");

        // Count the number of plugins that match the type we're after.
        int count = CountMatchingTypes(atts);

        // Allocate the arrays that hold the plugin information.
        pluginNames = new String[count];
        pluginVersions = new String[count];
        pluginAtts = new Plugin[count];
        pluginsLoaded = true;

        // Extract the sorted plugin list from the atts.
        String[] loadList = new String[count];
        PopulatePluginList(atts, loadList);
        if(verbose)
            System.out.println("Load List length "+loadList.length);

        // Store the plugin names and versions and try to load the
        // plugin using a class loader.
        for(int i = 0; i < count; ++i)
        {
            String name = loadList[i];

            // Add default values for this plugin in case it can't be loaded.
            pluginNames[i] = new String("");
            pluginVersions[i] = new String("");
            pluginAtts[i] = null;

            // Create a list of possible names for the 
            int nPossibleClassNames = 2;
            String[] classNames = new String[3];
            classNames[0] = new String("llnl.visit."+pluginType+"s."+name+"Attributes");
            classNames[1] = new String("llnl.visit."+pluginType+"s."+name+"PluginAttributes");
            try
            {
                String c0 = pluginType.substring(0,1).toUpperCase();
                String prefix = c0 + pluginType.substring(1, pluginType.length());
                classNames[2] = new String("llnl.visit."+pluginType+"s."+name+prefix+"Attributes");
                ++nPossibleClassNames;
            }
            catch(IndexOutOfBoundsException e0)
            {
                // ignore.
            }

            boolean classLoaded = false;
            for(int index = 0; index < nPossibleClassNames && !classLoaded; ++index)
            {
                try
                {
                    // Try to load the plugin class.
                    Class c = Class.forName(classNames[index]);

                    // If the new object has a Plugin interface then
                    // create an instance of it and store it.
                    Object obj = c.newInstance();

                    // Check the version of the plugin. If it matches the version that was
                    // sent from the viewer then we have a valid plugin and we can continue.
                    Plugin p = (Plugin)obj;
                    pluginNames[i] = new String(p.GetName());
                    pluginVersions[i] = new String(p.GetVersion());

                    // Save the instance that we created because it is the
                    // one that we'll use to communicate with the viewer.
                    pluginAtts[i] = p;

                    if(verbose)
                        System.out.println("Loaded "+pluginNames[i]+" version "+pluginVersions[i] + " from " + classNames[index]);

                    classLoaded = true;
                }
                catch(ClassNotFoundException e1)
                {
                    if(verbose)
                    {
                        e1.printStackTrace();
                        System.out.println("The "+name+" plugin could not be loaded from " + classNames[index]);
                    }
                }
                catch(InstantiationException e2)
                {
                    if(verbose)
                    {
                        e2.printStackTrace();
                        System.out.println("The "+name+" plugin could not be instantiated. InstantiationException.");
                    }
                }
                catch(IllegalAccessException e3)
                {
                    if(verbose)
                    {
                        e3.printStackTrace();
                        System.out.println("The "+name+" plugin could not be created. IllegalAccessException.");
                    }
                }
            }
        } // for i

        return true;
    }

    private boolean WantPlugin(PluginManagerAttributes atts, int i)
    {
        boolean want = false;

        if(i >= 0 && i < atts.GetName().size())
        {
            String t = (String)atts.GetType().elementAt(i);
            boolean enabled = (((Integer)atts.GetEnabled().elementAt(i)).intValue()>0);

            want = (t.equals(pluginType) && enabled);
        }

        return want;
    }

    private int CountMatchingTypes(PluginManagerAttributes atts)
    {
        int count = 0;
        for(int i = 0; i < atts.GetType().size(); ++i)
        {
            if(WantPlugin(atts, i))
                ++count;
        }

        return count;
    }

    private String         pluginType;
    private String[]       pluginNames;
    private String[]       pluginVersions;
    private Plugin[]       pluginAtts;
    private boolean        pluginsLoaded;
    private boolean        verbose;
}
