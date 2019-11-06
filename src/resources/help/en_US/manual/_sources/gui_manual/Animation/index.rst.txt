.. _Animation:

Animation
=========

This chapter discusses how to use VisIt to create animations. There are three
ways of creating animations using VisIt: flipbooks, keyframing, and scripting.
For complex animations with perhaps hundreds or thousands of database time
steps, it is often best to use scripting. VisIt provides Python and Java
language interfaces that allow you to program animation and save image files
that get converted into a movie. The flipbook approach is strictly for static
animations in which only the database time step changes. This method allows
database behavior over time to be quickly inspected without the added
complexity of scripting or keyframing. Keyframed animation can exhibit complex
behavior of the view, plot attributes, and database time states over time. 
This chapter emphasizes the flipbook and keyframe approaches and explains how
to create animations both ways.

.. toctree::
    :maxdepth: 2

    Animation_basics
    Keyframing
    Scripting
    Movie_tools
