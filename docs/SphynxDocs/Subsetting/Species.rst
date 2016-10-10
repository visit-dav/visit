Species
-------

VisIt adds species, which are components of materials, to the SIL when they are available. Air is a common material in simulations since many things in the real world are surrounded by air. The chemical composition of air on Earth is roughly 78% Nitrogen, 21% oxygen, 1% Argon. You can say that if air is a material then it has species: Nitrogen, Oxygen, and Argon with mass fractions 78%, 21%, 2%, respectively. Suppose one of the calculated quantities in a database with the afore-mentioned air material is atmospheric temperature. Now suppose that we are examining one cell that contains only the air material from the database and its atmospheric temperature is 100 degrees Fahrenheit. If we wanted to know how much the Nitrogen contributed to the atmospheric temperature, we could multiply its concentration of 78% times the 100 degrees Fahrenheit to yield: 78 degrees Fahrenheit. Species are often used to track chemical composition of materials and their effects on various calculated quantities.

When species are available, VisIt creates a scalar variable called
*Species*
and it is available in the variable menus for each plot that can accept scalar variables. The Species variable is a cell-centered scalar field defined over the whole mesh. When all species are turned on, the Species variable has the value of 1.0 over the entire mesh. When species are turned off, the Species variable is set to 1.0 minus the mass fraction of the species that was turned off. Using the previous example, if we plotted the Species variable and then turned off the air material's Nitrogen species, we would be left with only Oxygen's 21% and Argon's 1% so the species variable would be reduced to 22% or 0.22. When species are turned off, the amount of mass left to be multiplied by the plotted variable drops so the plotted variable's value in turn drops.

VisIt adds species to the SIL as a category that contains the various chemical constituents for all materials that have species. Since species are handled using the SIL, you can use VisIt's
**Subset Window**
to turn off species. Turning off species has quite a different effect than turning off entire materials. When materials are turned off, they no longer appear in the visualization. When species are turned off, no parts of the visualization disappear but the plotted data values may change due to drops in the Species variable.

Plotting species
~~~~~~~~~~~~~~~~

VisIt provides the Species scalar variable so you can plot or create expressions that involve species. If you create a Pseudocolor plot of the Species variable, the resulting plot will have a constant value of 1.0 over the entire mesh because when no species have been removed, they all sum to 1.0. Once you begin removing species by turning off species subsets in the
**Subset Window**
, the plotted value of Species changes, causing plots that use it to also change. If you remove all but one species, the plots that use the Species variable will show zero for all areas that do not contain the one selected species (see
). For example, if you had air for a material and then you removed every species except for oxygen, the plots that use the Species variable would show zero for every place that had no oxygen.

Turning off species
~~~~~~~~~~~~~~~~~~~

VisIt adds species information to the SIL as new subsets under a category called: Species. Since species are part of the SIL, you can use the
**Subset Window**
(see
) to turn off species. To access the list of species, select the Species category under the whole mesh. Once the Species category is clicked, the second pane in the
**Subset Window**
is populated with the species for all materials. Turn off the species that you don't want to look at by clicking off the check box next to the name of the species subset. When you apply your changes, the values for the Species variable are recalculated to include only the mass fractions for the species that are still turned on.
