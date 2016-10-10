Decimate operator
~~~~~~~~~~~~~~~~~

The Decimate operator, shown in
, removes nodes and cells from an input mesh, reducing the cell count while trying to maintain the overall shape of the original mesh. The Decimate operator can currently operate only on the external surfaces of the input geometry. This means that in order to apply the Decimate operator, you must first apply the ExternalSurfaces operator, which will be covered later in this chapter.

Using the Decimate operator
"""""""""""""""""""""""""""

The Decimate operator simplifies mesh geometry. This can be useful for producing models that have lower polygon counts than the model before the Decimate operator was applied. Models with lower polygon count can be useful for speeding up operations such as rendering. The Decimate operator has a single knob that influences how many cells are removed from the input mesh. The
**Target Reduction**
value is a floating point number in the range (0,1) and it can be set in the
**Decimate operator attrbutes window**
(see
). The number specified is the proportion of number of polygonal cells in the output dataset "over" the number of polygonal cells in the original dataset. As shown in
, higher values for
**Target Reduction**
value cause VisIt to simplify the mesh even more.

