Revolve operator
~~~~~~~~~~~~~~~~

The Revolve operator is for creating 3D geometry from 2D geometry by revolving the 2D about an axis. The Revolve operator is useful for incorporating 2D simulation data into a visualization along with existing 3D data. An example of the Revolve operator is shown in
.

Using the Revolve operator
""""""""""""""""""""""""""

To use the Revolve operator, the first thing to do is pick an axis of revolution. The axis of revolution is specified as a 3D vector in the
**Axis of revolution**
text field (see
) and serves as the axis about which your 2D geometry is revolved. If you want to revolve 2D geometry into 3D geometry without any
holes in the middle, be sure to pick an axis of revolution that is incident with an edge of your 2D geometry. If you want 3D geometry where the initial 2D faces do not meet, be sure to specify start and stop angles in degrees in the
**Start angle**
and
**Stop angle**
text fields. Finally, the number of steps determines how many times the initial 2D geometry is revolved along the way from the start angle to the stop angle. You can specify the number of steps by entering a new value into the
**Number of steps**
text field.

