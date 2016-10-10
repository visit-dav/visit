Using the Subset Window
-----------------------

You can open the
**Subset Window**
, shown in
, by clicking on the
**Subset**
option in the
**Main Window's Controls**
menu or by clicking on the
**Subset**
icon next to the name of a plot in the
**Plot lists's**
plot entries. VisIt's
**Subset Window**
displays subset relations and provides controls that allow you to alter a plot's SIL restriction by turning off parts of the database.

The
**Subset Window**
initially has three panels that display the sets contained in a database though the window can display an unlimited number of panels as subsets are browsed. Each
successive panel serves to further subset the database. The leftmost panel contains the top level set for the database being examined. The top level set, which includes all elements in the database, can usually be decomposed in various ways. For example, it can be decomposed by material, processor domain, etc. The various ways in which a database can be decomposed are called
subset categories. The subset categories will vary according to the file format in which the database was written as well as the organization and naming conventions used in the database.

Browsing subsets
~~~~~~~~~~~~~~~~

To browse the subsets for a database, you must first have created a plot. Once a plot is created and selected, open the
**Subset Window**
. The left panel in the
**Subset Window**
contains the database's top level set and may also list some subset categories. Some simple databases lack subset categories thus VisIt cannot create subsets. To start browsing the available subsets, click on one of the subset categories to display the subsets of that category. For instance, clicking on a "Material" subset category will list all of the database's materials in the next panel to the right. The materials are subsets of the top level set and decompose the top level set based on material. Double clicking on a set or clicking on a green turndown arrow lists any subset categories that can be used to further break down the set.

Changing a SIL restriction
~~~~~~~~~~~~~~~~~~~~~~~~~~

Each set in the
**Subset Window**
has a small check box next to it that allows you to turn the set on or off. The check box not only displays whether a set is on or off, but it also displays whether or not a set is partially on. When a set is partially on, it means that at least one (but not all) of the subsets that comprise the set is turned on. When a set is partially on, its check box shows a small slash instead of a check or an empty box. Uncheck the check box next to a set name to turn the set off.

Suppose you have a database that contains 4
domains (subsets that correspond to computer processor) numbered 1 through 4. If you want to turn off the subset that is domain1, first click on the "domains" subset category to list the subsets in that category. Next, click the check box next to the subset name "domain1" and click the
**Apply**
button. The result of this operation, shown in
, removes the "domain1" subset from the visualization. Note that the
**Subset Window**
"domain1" set's check box is unchecked and the top level set's check box has a slash through it to show that some subsets are turned off.

Creating complex subsets
~~~~~~~~~~~~~~~~~~~~~~~~

When visualizing a database, it is often useful to look at subsets created by subsetting more than one category. Suppose you have a database that has two subset categories: "Materials", and "domains" and that you want to turn off the "domain1" subset but you also want to turn off a material in the "domain4" subset. You can do this by clicking on the "domains" category and then unchecking the "domain1" check box in the second panel. Now, to turn off a material in the "domain4" subset, you click on the "domains" category in the left panel. Next, double-click on the "domain4" subset in the second panel. Select the "Materials" subset category in the second panel to make the third panel list the materials that you can remove from the "domain4" subset. Turning off a couple materials from the list in the third panel will only affect the "domain4" subset. An example of a complex subset is shown in
and the state of the
**Subset window**
is shown in
.


Turning multiple sets on and off
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When databases contain large numbers of subsets, it is convenient to turn groups of them on and off at the same time. You can select ranges of subsets by clicking on the name of a subset using the left mouse button and dragging the mouse up or down to other subsets in the list while still holding down the left mouse button. Alternatively, you can click on a subset to select it and then you can click on another subset while holding down the
*Shift*
key to select all of the subsets in the middle. Finally, you can select a group of multiple nonconsecutive subsets by holding down the
*Ctrl*
key while you click on the subsets that you want to select.

Once you have selected a group of subsets, you can use the buttons at the bottom of the pane whose subsets you selected. There are two action buttons at the bottom of each subset pane. The top button applies an action to all of the sets in the pane regardless of how they have been selected. The bottom button applies an action to only the subsets that you have selected. Each action button has three possible actions: Turn on, Turn off, and Reverse. You can change the action for an action button by clicking on the down-arrow button to its right and selecting one of the
**Turn on**
,
**Turn off**
, and
**Reverse**
menu options. When you use an action button that is set to
**Turn on**
, the appropriate subsets will be turned on. When you use an action button that is set to
**Turn off**
, the appropriate subsets will be turned off. When you use an action button that is set to
**Reverse**
, which is the default action, the appropriate subsets' enabled state will be reversed.
