Operators
---------

An operator is a filter applied to a database variable before the compute engine uses that variable to generate a plot. VisIt provides several standard operator types that allow various operations to be performed on plot data. The standard operators perform data restriction operations like planar slicing, spherical slicing, and thresholding, as well as more sophisticated operations like peeling off mesh layers. All of VisIt's operators are plugins and you can write your own operator plugins to extend VisIt in new ways. See the wiki at
*www.visitusers.org*
for more details on creating new operator plugins or send an e-mail inquiry to
*visit-users@elist.ornl.gov*
.

Managing operators
~~~~~~~~~~~~~~~~~~

When an operator is applied to a plot, it modifies the data that the plot uses to generate a visualization. Any number of operators can be applied to a plot. Each operator added to a plot restricts or modifies the data that is supplied to the plot. By using a series of operators, you can create very sophisticated visualizations.

Since operators are applied to plots and the controls for managing plots are in the
**Main Window's Active plots area**
, the controls for the operator are found in the same location as the plot controls. The
**Plot list**
, which displays the list of plots found in the current visualization window, also displays the operators applied to each plot. Each entry in the
**Plot list**
displays the database being plotted, the plot type, the variable, and all operators that are applied to the plot. When an operator is applied to a plot, the name of the operator is inserted in front of the plot variable. If multiple operators are applied to a plot, the most recently added operator appears first when reading left to right while the operator that was applied first appears just to the left of the variable name. Plot list entries can also be expanded to allow you to change the order or operators or remove operators from any place in the pipeline.

In addition to containing controls for managing plots, the
** Plot list**
contains controls that manage operators. Operators can be added, removed, and have their attributes set using controls found in the
**Active Plots area**
.

Adding an operator
""""""""""""""""""

You add operators by making a selection from the
**Operators**
menu, shown in
, which you activate by clicking on the
**Operators**
option in the
**Plots and Operators**
menu
. If you do not see an operator listed in this chapter in the
**Operators**
menu then the operator might not be loaded by default. If you want to enable additional operators, use the
**Plugin Manager Window**
. When you select an operator, it applies operator to the selected plots in the Plot list unless the
**Apply operators to all plots**
check box is checked, in which case, the selected operator is applied to all plots in the
**Plot list**
. By default, operators are applied to all plots in the
**Plot list**
.

When you add an operator to a plot, the name of the operator appears in the plot list entry to the left of the variable or any previously applied operator. If you apply an
operator to an already generated plot, the plot is regenerated immediately with the applied operator. If a plot has not yet been generated (its entry is green), then the operator does not take effect until the plot is generated. This provides time for setting the operator attributes.

It is also possible to apply an operator by clicking an operator attributes window's
**Apply**
button. If you click the
**Apply**
button when there is no operator of the specified type has been applied to the selected plot, you can apply the operator and any changes you made by clicking the
**Yes**
button in a dialog window (see
) that asks whether or not the operator should be applied to the plot.

Expanding plots
"""""""""""""""

Plot list entries are normally collapsed by default so the operators applied to plots are shown in the
**Plot list**
as a series of nested operators, which finally take a variable as an argument. The
**Plot list**
allows plot list entries to be expanded on a per-plot basis so you can get to each individual operator that is applied to a plot. To expand a plot list entry, click on its expand button, shown in
. When a plot list entry is expanded, the plot's database, all operators, and finally the plot get their own line in the plot list entry. This is significant because it allows operators to have additional controls to let you reposition them in the pipeline or remove them from the middle of the pipeline without having to first remove other operators.

Changing the order of operators
"""""""""""""""""""""""""""""""

Sometimes when you've applied several operators, it is useful to change the order of the operators around. For example, you might want to apply a Slice operator before a Reflect operator instead of after it so you can reduce the amount of data that VisIt must process in order to draw your plot. The order in which operators are applied often has a significant impact on the visualization. Using the previous example, suppose a plot is sliced before it is reflected. The resulting visualization is likely to have a reflected slice of the original data. If the order of the operators was reversed so the Reflect operator came first, the Slice operator's slice plane might not intersect the reflected data in the same way, which could result in a totally different looking visualization.

You must expand a plot list entry in order to change the order of its operators. Once the plot list entry is expanded, each operator is listed in the order in which they were applied and each operator has small buttons to the right of the its name that allow you to move the operator up or down in the pipeline. To move an operator closer to the database so it is executed before it would have been executed before, click on the
**Up**
button next to an operator's name. Moving the operator closer to the database in the pipeline is called demoting the operator. If you click the
**Down**
button next to an operator's name, that operator is moved to a later stage of the pipeline. Moving an operator to a later stage of the pipeline is known as promoting the operator since the operator appears closer to the plot in the expanded plot entry. Operators in the plot list entry that can only be moved in one direction have only the
**Up**
button or the
**Down**
button while operators in the middle of the pipeline have both the
**Up**
button and the
**Down**
button.

Removing operators
""""""""""""""""""

You do not remove operators by clicking the
**Delete**
button as you do with plots. There are two ways that you can delete an operator from a plot. As its last two options, the
**Operators**
menu has options that remove one or more operators. To remove only the last applied operator, select the
**Remove last**
option from the
**Operators**
menu. To remove all operators applied to a plot, select the
**Remove all**
option from the
**Operators**
menu. Unless the
**Apply operator to all plots**
check box is checked, operators are only removed from selected plots. If you remove
operators using the controls in the
**Operators**
menu, the plots that are in the completed state are immediately recalculated by the compute engine and redisplayed using the modified list of operators.

The controls in the
**Operators**
menu allow you to remove operators from the end of a plot's operator list or remove all of a plot's operators. VisIt also provides controls that let you remove specific operators from the middle of a plot's operator list. First expand the plot list entry by clicking its
**Expand**
button and then click on the red
**X**
button next to the operator that you want to delete. The red
**X**
button deletes the operator to which it is attached. When an operator is deleted using the red
**X**
buttons, the plot is reset back to the new state so you must click the
**Draw**
button to tell VisIt to regenerate the plot. See
for an example of deleting an operator from the middle of a plot's operator list.

Setting operator attributes
"""""""""""""""""""""""""""

Each operator type has its own operator attributes window used to set attributes for that operator type. You activate Operator attribute windows by selecting the operator type from the
**OpAtts**
(Operator attributes) menu shown in
.

When there is only one operator of a given type in
a plot's operator list, setting the attributes for that operator type will affect that one operator. When there are multiple instances of the same type of operator in a plot's operator list, only the active operator's attributes are set if the active operator is an operator of the type whose attributes are being set. The active operator is the operator whose attributes are set when using an operator attributes window and can be identified in an expanded plot entry by the highlight that is drawn around it (see Figure4-8). To set the active operator, expand a plot entry and then click on an operator in the expanded plot entry's operator list.

Setting the active operator is useful when you have multiple operators of the same type applied to the same plot. For example, you might have applied two Transform operators so you can scale a plot with one operator and then rotate the plot with the second Transform operator. If there was no way to set an active operator, changing the attributes for the Transform operator would cause both instances of the operator to get the same operator attributes. You can make sure the first operator only gets scaling information by making it the active operator. To set the attributes in the second instance of the Transform operator, you can click on that second Transform operator in the expanded plot entry, to make it the active operator, and then set the rotation attributes for that second Transform operator.
