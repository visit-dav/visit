package llnl.visit;

import java.io.File;
import java.lang.ArrayIndexOutOfBoundsException;
import java.lang.InstantiationException;
import java.lang.IllegalAccessException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.MalformedURLException;

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
        boolean success = true;

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

        // Create a class path based on the type of plugin that we
        // want to read.
        String classPath = System.getProperty("java.class.path");
        String[] pathList = classPath.split(java.io.File.pathSeparator);
        if(pathList.length > 0)
        {
            // Create the URL list that we'll use to create a class loader.
            URL[] URLlist = new URL[pathList.length];

            // Check each path to make sure that it exists and that we can read it.
            // If we can read it, add it to the URL list.
            for(int i = 0; i < pathList.length; ++i)
            {
                File path = new File(pathList[i]);
                if(!path.exists() || !path.canRead())
                {
                    if(verbose)
                         System.out.println("The classpath directory "+path+" cannot be read.");
                    return false;
                }
                else
                {
                    try
                    {
                        URLlist[i] = path.toURL();
                    }
                    catch(MalformedURLException e4)
                    {
                        if(verbose)
                            System.out.println("Could not read the "+pluginType+" plugin directory!");
                        return false;
                    }
                }
            } 
    
            // Create a new class loader that reads classes from the
            // specified plugin directory.
            loader = new URLClassLoader(URLlist);
        }
        else
        {
            if(verbose)
                System.out.println("The class path is empty - cannot determine VisIt plugin directory!");
            return false;
        }

        // Store the plugin names and versions and try to load the
        // plugin using a class loader.
        int index = 0;
        for(int i = 0; i < count && success; ++i)
        {
            String name = loadList[i];

            try
            {
                // Create the expected class name.
                String className = new String("llnl.visit."+pluginType+"s."+name+"Attributes");

                // Try using this object's class loaded to load the
                // plugin class.
                Class c = loader.loadClass(className);

                // If the new object has a Plugin interface then
                // create an instance of it and store it.
                Object obj = c.newInstance();

                // Check the version of the plugin. If it matches the version that was
                // sent from the viewer then we have a valid plugin and we can continue.
                Plugin p = (Plugin)obj;
                pluginNames[index] = new String(p.GetName());
                pluginVersions[index] = new String(p.GetVersion());

                // Save the instance that we created because it is the
                // one that we'll use to communicate with the viewer.
                pluginAtts[index] = p;

                if(verbose)
                    System.out.println("Loaded "+pluginNames[index]+" version "+pluginVersions[index]);

                ++index;
            }
            catch(ClassNotFoundException e)
            {
                success = false;
                if(verbose)
                    System.out.println("The "+name+" plugin could not be loaded. ClassNotFoundException.");
            }
            catch(InstantiationException e2)
            {
                success = false;
                if(verbose)
                    System.out.println("The "+name+" plugin could not be instantiated. InstantiationException.");
            }
            catch(IllegalAccessException e3)
            {
                success = false;
                if(verbose)
                    System.out.println("The "+name+" plugin could not be create. IllegalAccessException.");
            }
        } // for i

        return success;
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
    private URLClassLoader loader;
}
