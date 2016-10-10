Databases
---------

One main use of a visualization tool such as VisIt is to compare multiple related simulation databases. Simulations are often run over and over with a variety of different settings or physics models and this results in several versions of a simulation database that all describe essentially the same object or phenomenon. Simulations are also often run using different simulation codes and it is important for a visualization tool to compare the results from both simulations for validation purposes. You can use VisIt to open any number of databases at the same time so you can create plots from different simulation databases in the same window or in separate visualization windows that have been locked together.

Activating a database
~~~~~~~~~~~~~~~~~~~~~

VisIt can have any number of databases open simultaneously but there is still an active database that is used to create new plots. Each time you open a database, the newly opened database becomes the active database and consequently the source database for any new plots that you decide to create. If you want to create a plot using a database that is open but is not your active database, you must activate that database so it becomes your active database. When you activate a database, its variables are added to the menus for the various plot types.

There are two ways to activate a database and the method that you choose depends on how you have configured the
**File panel**
to display files. By default, the
**File panel**
shows the
**Selected files**
list and the
**Open**
,
**ReOpen**
,
**Activate**
,
**Replace**
, and
**Overlay**
buttons. However, you can configure the
**File panel**
so it does not show the
**Selected files**
list. You can read more about configuring the
**File panel**
in the Preferences chapter of this book. If the
**Selected files**

list is shown in the
**File panel**
, you can click on a database in the
**Selected files**
list that you've previously opened and click the
**Activate**
button to set the active database to the highlighted file. The
**Activate**
button, shown in Figure
, is located in the same place as the
**Open**
or
**ReOpen**
buttons but it is only displayed if you have clicked on a file that you have previously opened that is not already the active database.

If you have configured VisIt's
**File panel**
so the
**Selected files**
list is not shown then you have to use the second method of database activation in order to activate a database since the controls used in the first method are not available. The second method for activating a database is to select a database from the
**Source**
combo box, which is located just above the plot list and contains the list of opened databases (see
).

Multiple time sliders
~~~~~~~~~~~~~~~~~~~~~

When your open databases all have only a single time state, the
**Time slider **
in the
**Main Window**
is disabled. When you have one database that has multiple time states, the
**Time slider**
is enabled and can be used exclusively to change time states for the database that has multiple time states; the database does not even have to be the active database. Things get a little more complicated when you have opened more than one time-varying database - especially if you have plots from more than one of them.

When you open a database in VisIt, it becomes the active database. If the database that you open has multiple time states, VisIt creates a new logical time slider for it so you can end up having a separate time slider for every open database with multiple time states. When VisIt has to create a time slider for a newly opened database, it also makes the new database's (also the active database) be the active time slider. There is only one
**Time slider**
control in the
**Main Window**
so when there are multiple logical time sliders, VisIt displays an
**Active time slider**
combo box (see
) that lets you choose which logical time slider to affect when you change time using the
**Time slider**
.

Since VisIt allows each time-varying database to have its own logical time slider, you can create plots from more than one time-varying database in a single visualization window and change time independently for each database, an impossible operation in MeshTV and early versions of VisIt. Another benefit of having multiple logical time sliders is that the databases plotted in the visualization windows are free to have different numbers of time states. Suppose you have opened time-varying databases A and B and created plots from both databases in the same visualization window. Assuming you opened database A and then database B, database B will be the active database. If you want to change time states for database A but not for database B, you can select database A from the
**Active time slider**
combo box and then change the time state using the
**Time slider**
. If you then wanted to change time states for database B, you could select it in the
**Active time slider **
combo box and then change the time state using the
**Time slider**
. If you wanted to change time states for both A and B at the same time, you have to use database correlations, which are covered next.
