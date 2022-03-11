visit_utils
=============

visit_utils is a pure python module distributed along with Visit_'s Python interface.
It provides a simple interface to encode movie files and methods that wrap more complex VisIt Python command sequences to simplify a few common use cases.
It also provides a stand alone PySide based annotation rendering API.

Here we provide details on the encoding and engine launching modules:

visit_utils.encoding
----------------------

``visit_utils.encoding`` provides methods that allow you to use movie encoders (e.g ffmpeg) to encode movies from sequences of  image files and extract image files from movie files.

Methods:


.. py:function:: visit_utils.encoding.encode(ipattern, ofile, fdup=None, etype=None, stereo=False, input_frame_rate=None, output_frame_rate=None)

   Encodes a sequence of images into a movie.
   
   Example Usage:
   
   ::

     from visit_utils.encoding import *
     encode("input.%04d.png","output.mpg")
     encode("input.%04d.png","output.wmv",fdup=5)
     encode("input.%04d.png","output.sm")
     encode("input.%04d.png","output.sm",stereo=True)

   :param ipattern: Input file pattern. Requires a printf style # format like "file%04d.png".
   :param ofile: Output file name
   :param fdup: Allows you to set an integer number of times to duplicate the input frames as they are passed to the encoder. (The duplication actually happens via symlinks) [Default = None]
   :param etype: Allows to select which encoder to use ( If not passed the file extension is used to select an encoder ) [Default = None] 
   :param input_frame_rate:  Allows you to set the input frame rate, in frames
       per second, that the encoder uses. [Default = None]
   :param output_frame_rate:  Allows you to set the output frame rate, in frames per second, that the encoder uses. Note output formats
      typically only support a few output fps values. To obtain a perceived fps, the input_frame_rate is a better option to try. [Default = None]


.. py:function:: visit_utils.encoding.extract(ifile,opattern)

    Extracts a sequence of images from a a movie.

    Example:

    ::

      extract("movie.mpg","output%04d.png")

   :param ifile: Input file. 
   :param opattern: Output file pattern. Requires a printf style # format like "file%04d.png".

.. py:function:: visit_utils.encoding.encoders()

  :return: A list of strings of the available encoders.



visit_utils.engine
----------------------

``visit_utils.engine`` provides an interface to launch VisIt engines that uses installed host profiles.

Methods:


.. py:function:: visit_utils.engine.open(nprocs, method, ppn=1, part=None, bank=None, rtime=None, vdir=None)

   Launch VisIt compute engine on the current host.

   Example usage:

    Launch engine with 36 MPI tasks using default options for this host:

    :: 
        
        engine.open(nprocs=36)

    Launch engine with 36 MPI tasks using a specific partition:

    :: 
        
      engine.open(nprocs=36, part="pbatch")

    Launch engine with 36 MPI tasks, ask for 60 minute time limit:

    :: 
        
      engine.open(nprocs=36, rtime=60)

    If you already have a slurm batch allocation, you can use:

    :: 
        
      engine.open(method="slurm")

    This reads the ``SLURM_JOB_NUM_NODES`` and ``SLURM_CPUS_ON_NODE``
    env vars and uses these values to launch with srun.

    If you already have a lsf batch allocation, you can use:

    :: 
        
      engine.open(method="lsf")

    This reads the ``LSB_DJOB_NUMPROC`` env var and uses it
    the to launch with mpirun.

  :param nprocs: Number of MPI tasks
  :param methods: Launch Method (`srun`, etc)
  :param ppn: MPI tasks per node
  :param part: Partition
  :param rtime: Job time
  :param vdir: Path to VisIt install

.. py:function:: visit_utils.engine.close(ename=None)

   Closes VisIt's Compute Engine.
  
   :param ename: Engine name to close (optional) 


.. py:function:: visit_utils.engine.supported_hosts()

  :return: A list of the names of supported hosts. 


