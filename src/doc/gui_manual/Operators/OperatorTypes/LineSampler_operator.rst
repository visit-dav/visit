.. _LineSampler operator:

LineSampler operator
~~~~~~~~~~~~~~~~~~~~

The Line Sampler operator is used for sampling a 3D/2D dataset in much
the same way as the Lineout tool except in a much more defined
manner. That is the user can define a series of "arrays" (e.g. planes)
that consists of one or more "channels" for sampling. For each array
the orientation of the plane can be defined. Whereas for each channel
its orirentation within the plane and the sampling type and spacing
can be defined. For instance, the sampling can be a series of lines
through the data, or a series of points integrated over time.

The genesis of the Line Sampler operator is from plasma physics and
the synthethic diagnostics performed in fusion simulations so to
compare against experimental results. As such the nomenclature is
based on this usage.

For more information on LineSampler, see the :ref:`LineSampler`
section in the :ref:`Quantitative` chapter.

