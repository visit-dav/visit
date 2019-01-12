Saving movies
-------------

In addition to allowing you to save images of your visualization window for the
current time state, VisIt also allows you to save movies and sets of images for
your visualizations that vary over time. There are multiple methods for saving
movies with VisIt. This section introduces the Save movie wizard and explains
how to use it to create movies from within VisIt's GUI. The 
:ref:`Animation` chapter explains some auxiliary methods that
can be used to create movies.

.. _save_movie_wizard_1:

.. figure:: images/savemoviewizard1.png 
   
   Save movie wizard (screen 1)


The **Save movie wizard** (see :numref:`Figure %s<save_movie_wizard_1>`) is
available in the **Main Window's Files** menu. The **Save movie wizard's**
purpose is to lead you through a set of simple questions that allow VisIt to
gather the information required to create movies of your visualizations. 
For example, the **Save movie wizard** asks which image and movie formats
you want to generate, where you want to store the movies, what you want to
call the movies, etc. Each of these questions appears on a separate screen
in the **Save movie wizard** and once you answer the question on the current
screen, clicking the **Next (Continue for OSX)** button advances you to the next screen. You can
cancel saving a movie at any time by clicking on the **Cancel** button. If you
advance to the last screen in the **Save movie wizard** then you have
successfully provided all of the required information that VisIt needs to make
your movie. Clicking the **Finish** button at that point invokes VisIt's
movie-making script to make the movie. If you want to make subsequent movies,
you can choose to use the settings for the movies that you just made or you can
choose to create a new movie and provide new information.

Choosing movie formats
~~~~~~~~~~~~~~~~~~~~~~

.. _save_movie_wizard_2:

.. figure:: images/savemoviewizard2.png 
   
   Save movie wizard (screen 2)


The **Save movie wizard's**
second screen, shown in :numref:`Figure %s<save_movie_wizard_2>`, allows you
to pick the types of movies that you want to create. You can select as many
image and movie formats as you want and you can even specify multiple
resolutions of the same movie. VisIt allows you to order multiple versions of
your movie because it is often easier to create different versions of the movie
all at once as opposed to doing it later once it is discovered that you need
a new version to play on a laptop computer or a tiled display wall.

The **Save movie wizard's** second screen is divided vertically into two main
areas. On the left you will find the **Format and resolution** area, which
displays the format and resolution for the current movie. On the right, you
will find the **Output** area, which lists the formats and resolutions for all
of the movies that you have ordered. By default no movie formats are present
in the **Output** area's list of movies. You cannot proceed to the next screen
until you add at least one movie format to the list of movies in the **Output**
area.

To add a movie format to the list of movies in the **Output** area, first
choose the desired movie format from the **Format** combo box in the 
**Format and resolution** area. Next, choose the movie resolution. The movie
resolution can be specified in terms of the visualization window's current
size or it can be specified in absolute pixels. The default movie resolution
uses the visualization window's current size with a scale of 1. You can change
the scale to shrink or grow the movie while keeping the visualization window's
current aspect ratio. If you want to specify an absolute pixel size for the
movie, click on the **Specify movie size** radio button and type the desired
movie width and height into the **Width** and\ **Height** text fields. Note
that if you specify a width and height that causes the movie's shape to differ
from the visualization window's shape, you might want to double-check that the
view used for the visualization window's plots does not change appreciably.

The **Save movie wizard** allows you to create stereo movies if you check the 
**Stereo movie** box and select a stereo type from the **Stereo type** drop-down
menu. The default is to create non-stereo movies because stereo movies are not 
widely supported. 

.. note:: "Streaming movie" format is an LLNL format

The only movie format that VisIt produces that is compatible with stereo movies
is the "Streaming movie" format, which is an LLNL format commonly used for
tiled displays. The "Streaming movie" format can support stereo movies where
the image will flicker between left and right eye versions of the movie,
causing a stereo effect if you view the movie using suitable liquid-crystal
goggles. The stereo option has no effect when used with other movie formats.
However, if you choose to save a stereo movie in any of VisIt's supported image
formats, VisIt will save images for the left eye and images for the right eye.
You can then take the left and right images into your favorite stereo movie
creation software to create your own stereo movie.

Once you have selected the desired movie format, width, and height, click on
the right-arrow button that separates the **Format and resolution** area from
the **Output** area. Clicking the right-arrow button adds your movie to the
list of movies that you want to make. Once you have at least one movie in the
**Output** area, the screen's Next button will become active. Click the
**Next** button to go to the next screen in the **Save movie wizard**

Choosing movie length
~~~~~~~~~~~~~~~~~~~~~

.. _save_movie_wizard_3:

.. figure:: images/savemoviewizard3.png

   Save movie wizard (screen 3)

It is possible to specify the range of time states to use for the movie, as well
as specify a stride if you have too many time states saved (see :numref:`Figure %s<save_movie_wizard_3>`). 
The wizard will automatically set the range of time states.

Choosing the movie name
~~~~~~~~~~~~~~~~~~~~~~~

.. _save_movie_wizard_4:

.. figure:: images/savemoviewizard4.png 
   
   Save movie wizard (screen 4)

Once you have specified options that tell VisIt what kinds of movies that you
want to make, you must provide the base name and location for your movies. By
default, movies are saved to the directory in which you started VisIt. If you
want to specify an alternate directory, you can either type in a new directory
path into the **Output directory** text field 
(see :numref:`Figure %s<save_movie_wizard_4>`) or you can select a directory 
from the **Choose directory** dialog box activated by clicking on
the *"..."* button.

The base filename for the movie is the name that is prepended to all of the
movies that you generate. When generating multiple movies with differing
resolutions, the movie resolution is often encoded into the filename. VisIt may
generate many different movies with different names but they will all share the
same base filename that you provided by typing into the **Base filename** text
field.

Choosing e-mail notification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _save_movie_wizard_5:

.. figure:: images/savemoviewizard5.png 
   
   Save movie wizard (screen 5)

If you want to be notified by e-mail when the movie creation is complete, then 
select the **Yes** option and enter the appropriate e-mail address (see :numref:`Figure %s<save_movie_wizard_5>`). 
By default, no e-mail notification is sent once the movie creation is complete. 

Choosing movie generation method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _save_movie_wizard_6:

.. figure:: images/savemoviewizard6.png 
   
   Save movie wizard (screen 6)

After all movie options are specified, VisIt prompts you how you
would like your movie made. At this point, you can click the **Finish**/**Done** button
to make VisIt start generating your movie. You can change how VisIt creates
your movie by clicking a different movie generation method on the
**Save movie wizard's** sixth screen, shown in Figure
:numref:`Figure %s<save_movie_wizard_6>` .


The default option for movie creation allows VisIt to use your current VisIt
session to make your movies. This has the advantage that it uses your current
compute engine and allocated processors, which makes movie generation start
immediately. When you use this movie generation method, VisIt will launch its
command line interface (CLI) and execute Python movie-making scripts in order
to generate your movie. This means that you have both the VisIt GUI and CLI
controlling the viewer. If you use this movie generation method, you will be
able to watch your movie as it is generated. You can track the movie's progress
using the **Movie progress dialog** , shown in 
:numref:`Figure %s<save_movie_progress>` . The downside to using your
currently allocated processors is that movie generation takes over your VisIt
session until the movie is complete. If you want to regain control over your
VisIt session, effectively cancelling the movie generation process, you can
click the **Movie progress dialog's** **Cancel** button.

.. _save_movie_progress:

.. figure:: images/movieprogress.png 
   
   Movie progress dialog


The second movie generation method will cause VisIt to save out a session file
containing every detail about your visualization so it can be recreated by a
new instance of VisIt. This method works well if you want to create a movie
without sacrificing your current VisIt session but you cannot watch the movie
as it is generated and you may have to wait for the second instance's compute
engine to be scheduled to run. The last movie generation option simply makes
VisIt display the command that you would have to type at a command prompt in
order to make VisIt generate a movie of your current visualizations.
