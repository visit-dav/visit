.. _Masonry_Execution:

Running Masonry Scripts
-----------------------

bootstrap_visit.py
~~~~~~~~~~~~~~~~~~

The ``bootstrap_visit.py`` file contains all of the logic to execute the necessary steps for creating the macOS Disk Image File (DMG).
It takes the JSON configuration file as an argument::

   python3 bootstrap_visit.py opts/<file-name>.json


masonry_view_log.py
~~~~~~~~~~~~~~~~~~~

Once masonry is running, it will produce log files in the ``_logs`` directory. To view the logs in HTML format (see :numref:`Figure %s<masonry_log_file>`), run the ``masonry_view_log.py`` script. This script takes the log file as an argument::

   python3 masonry_view_log.py _logs/<log-fle>.json

.. _masonry_log_file:

.. figure:: images/log.png
   :width: 50%
   :align: center

   Mansonry Logs in HTML format

Note that if you already have a web browser running on your system, that
the web browser may not be able to connect to the local web server created
to serve up the log file. In this case you should exit your existing web
server and try again.

