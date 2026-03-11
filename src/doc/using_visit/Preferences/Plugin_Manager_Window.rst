.. _Plugin Manager Window:

Plugin Manager Window
---------------------

The **Plugin Manager** window , shown in
:numref:`Figure %s<Preferences-PluginManager>`, allows the user to see which
plug-ins are available for plots, operators, and databases.  Not all plug-ins
have to be loaded, in fact, many operator plug-ins are not loaded by default.
The **Plugin Manager** window allows the user to specify which plug-ins are
loaded when VisIt_ is started. The **Plugin Manager** window is brought up by
selecting **Plugin Manager** from the **Main** window's **Options** menu.

.. _Preferences-PluginManager:

.. figure:: images/PluginManager.png

   The plug-in manager window
   
.. _Preferences_Enabling_Plugins:

Enabling and Disabling Plugins
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All of VisIt_'s plots, operators, and database readers are implemented as
plug-ins that are loaded when VisIt_ first starts up. Some plug-ins are not
likely to be used by most people so they should not be loaded. The
**Plugin Manager** window provides a mechanism to turn plug-ins on and off.
The window has three tabs: **Plots** , **Operators** , and **Databases**.
Each tab displays a list of plug-ins that can be loaded by VisIt_. If a
plug-in is enabled, it has a check by its name.

Plugins can be turned on and off by checking or unchecking the check box
next to a plug-in's name. Plugins are loaded at startup, so enabling or
disabling plug-ins will not take effect unless the preferences are saved
and VisIt_ is restarted.

If plots or operators are disabled, they will not appear in the **Add**,
**Operator**, **PlotAtts** and **OpAtts** menus. Similarly, disabled
databases will not show up in the list of **Open file type as** menu in
the **File open** window.
