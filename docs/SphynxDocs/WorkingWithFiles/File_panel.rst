File panel
----------

The
**File panel**
, near the top of the
**Main Window,**
displays the files in the
**Selected files list**
and contains controls for opening files and playing animations. The
**File panel**
always tries to display the
**Selected files list**
in the most intuitive manner. If all of the selected files come from the same directory on the local machine, the
**File panel**
will display the selected files in a simple list like the one shown in
.


When the
**Selected files list**
contains files from multiple directories, the
**File panel**
displays the list as a file tree that displays the name of the computer where the files are located as well as the selected files and a minimal amount of directory information needed to uniquely identify files. Directories are identified with a folder icon and they cannot be opened for visualization. Files have no icons; instead they have a number next to them that indicates their index in the
**Selected files list**
. The file number is used in the
**Plot list**
to identify the file used by the plot. The file tree is always represented using the fewest possible number of nodes to avoid having to click through levels of empty directories. Displaying the
**Selected files list**
in tree form makes it easier to distinguish the directory from which files come. This view of the
**Selected files list**
is shown in
.

Opening a file
~~~~~~~~~~~~~~

To open a file, you want to visualize, click on its name in the
**File panel**
and then click on the
**Open**
button. You can also open files by double-clicking on the file in the
**File panel**
. Once a file is open, the
**Open**
button turns into the
**ReOpen**
button and the file can be visualized. If you click on a file that has been opened before, the
**Open**
button becomes the
**Activate**
button. Clicking on the
**Activate**
button causes the selected file to become the new open database, from which plots can be created.

When the
**ReOpen**
button is clicked, all cached information about the open database is deleted, the database is queried again for its information, and any plots that use that database are regenerated using the new information. This allows VisIt to access data that was added to the database after VisIt first opened it.

Virtual databases, unlike ".visit" files, are expanded in the
**File panel**
by default because the time states are usually known before the database is opened. Since the time states are known before the database is opened, and you can click on any of them, it is possible to highlight a later time state of a virtual database and open it at that later time state. Opening a virtual database at a later time state can be useful if the database has variables that are introduced later in the time series.

Opening a file on a remote computer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Opening a file on a remote computer works like opening a file on your local computer because the files from the remote computer have been placed into the
**File panel**
. The only difference is that when the
**File panel **
contains files from multiple computers, it displays the list of files from each computer under a small computer icon that represents the computer where the files are located. An example of this is shown in
.

Opening a time-varying database
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A time-varying database (a ".visit" file or virtual database) is opened the same way as a database with a single time step. A time varying database is created by either grouping files into a ".visit" file in the
**File Selection Window**
or by turning on automatic file grouping so VisIt creates virtual databases, which are databases made up of multiple single time step databases that have similar filenames. To open a time-varying database, click on it to highlight it and then click the
**Open**
button in the
**File panel**
. Double-clicking the time-varying database also will open it. The one difference between
opening a single time step database and a time-varying database is that if the time-varying database is also a virtual database then it can be opened at a later time step without having to first open it at the first time step.

The
**File panel**
displays time varying databases with a green database icon next to the name of the database. There often is also a small check box next to the database icon that expands to show the database's time steps. This is shown in
. When a time- varying database is open, the animation controls are activated so any time step in the database can be used. Note that the animation controls are only active when visualizing a time-varying database or when VisIt is in keyframe animation mode.

Setting the active time step
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Time-varying databases are composed of one or more time steps which contain data to be visualized. The active time step is the time step within a time-varying database that VisIt uses to generate plots. The
**File panel**
contains a group of animation controls below the
**Selected files list**
that allow you to set the active time step used for visualization. The
**Animation slider**
and the
**Animation text field**
show the active time step. To set the active time step, you can drag the
**Animation slider**
and release it when you get to the desired time step, or you can type in a
cycle number into the
**Animation text field**
. If you type in a cycle number that is not in the database, the active time step will be set to the time step with the closest cycle number to the cycle that was specified. Another method of setting the
active time step is to double click a cycle number under the database in the
**Selected file list**
.

Playing animations
~~~~~~~~~~~~~~~~~~

The
**File panel**
contains a set of
**VCR buttons**
that allow you to put VisIt into an animation mode that plays your visualization using all of the time steps in the database. The
**VCR buttons**
are only active when you have a time varying database. The leftmost VCR button moves the animation back one frame. The VCR button second from the left plays the animation in reverse. The middle VCR button stops the animation. The VCR button second from the right plays the animation. The VCR button farthest to the right advances the animation by one frame. As the animation progresses, the
**Animation Slider**
and the
**Animation Text Field**
are updated to reflect the active time step.

Reopening a database
~~~~~~~~~~~~~~~~~~~~

Sometimes it is useful to begin visualizing simulation data before the simulation has finished writing out data files for all time steps. When you open a database in VisIt and create plots and later want to visualize new time steps that have been generated since you first opened the database, you can reopen the database to force VisIt to get the data for the new time steps. To reopen a database, click the
**ReOpen**
button in the
**File panel**
. When VisIt reopens a database, it clears the geometry for all plots that used that database and cached information about the
database is erased so that when VisIt reopens the database, plots are regenerated using the new data files.

Replacing a database
~~~~~~~~~~~~~~~~~~~~

If you have created a plot with one database and want to see what it looks like using data from another database, you can replace the database using the
**File panel's**

**Replace**
button. To replace a database, first select a new database by clicking on a file in the
**File panel's Selected files list**
and then click the
**Replace**
button. This will make VisIt try to replace the databases used in the plots with the new database. If the replace operation is a success, the plots are regenerated using the new database and they are displayed in the visualization window.

Overlaying a database
~~~~~~~~~~~~~~~~~~~~~

Overlaying a database is a way to duplicate every plot in the plot list using a new database. To overlay plots, select a new database from the
**Selected files list**
in the
**File Panel**
and then click the
**Overlay**
button. This copies each plot in the
**Active plot list**
and replaces the database with the specified database. If the operation succeeds, the plots are generated and displayed in the visualization window. It is important to remember that each time the
**Overlay**
button is clicked, the number of plots in the plot list doubles.
