Index Select operator
~~~~~~~~~~~~~~~~~~~~~

The Index select operator selects a subset of a 2D or 3D structured mesh based on ranges of cell indices. Structured meshes have an implied connectivity that allows each cell in the mesh to be specified by an i,j or i,j,k index depending on the dimension of the mesh. The Index select operator allows you to specify different ranges for each mesh dimension. The ranges are used to
select a brick of cells from the mesh. In addition to indices, the Index select operator uses stride to select cells from the mesh. Stride is a value that allows the operator to count by 2's, 3's, etc. when iterating through the range indices. Stride is set to 1 by default. When higher values are used, the resulting mesh is more coarse since it contains fewer cells in each dimension. The Index select operator attempts to preserve the size of the mesh when non-unity stride values are used. An example of the Index select operator appears in
.

Setting a selection range
"""""""""""""""""""""""""

The
**Index select operator attributes window**
, shown in
, contains nine spin boxes that allow you to enter minimum and maximum ranges for i,j,k. To select all cells in the
**X**
dimension whose index is greater than 10, you would enter 10 into the spin box in the
**I**
row and
**Min**
column. Then you would enter max into the spin box in the
**Max**
column in the
**I**
row. Finally, you would enter a stride of 1 into the spin box in the
**Incr**
column in the
**I**
row. If you wanted to subselect cell ranges for the Y dimension, you could follow a similar procedure using the spin boxes in the
**J**
row and so forth.

To set a range, first select the maximum number of dimensions to which the Index select operator will apply. To set the dimension, click on the
**1D**
,
**2D**
,
**3D**
radio buttons. Note that if the chosen number of dimensions is larger than the number of dimensions in the database, the extra dimension ranges are ignored. It is generally best to select the same number of dimensions as the database. The three range text fields are listed in i,j,k order from top to bottom. To restrict the number of cells in the X-dimension, use spin boxes in the
**I**
row. To restrict the number of cells in the Y-dimension, use the spin boxes in the
**J**
row. To restrict the number of cells in the Z-dimension, use the spin boxes in the
**K**
row.

Block number
""""""""""""

Some databases are composed of multiple meshes, often called domains or blocks. The
**Index select operator attributes window**
calls these submeshes blocks. Often when examining a database, you might want to look at only one block at a time. By default, the Index select operator is applied to all blocks in the database. This means that each index range is applied to each block in the database and will probably result in an image featuring several small chunks of cells. When the Index select operator is set to apply to just one block, the index ranges are relative to the specified block. To make the Index select operator apply to just one block, click on the
**Block**
radio button and type a new block number into the
**Block**
text field.

Group number
""""""""""""

Some databases are composed of multiple groups of meshes, which are often called groups. The
**Index select operator attributes window**
calls these groups of meshes groups. The Index select operator can be used to examine parts of a group of meshes by clicking the
**Groups**
radio button and typing a group number into the
**Group**
text field.

