.. _reporting_issues:

Reporting Issues
~~~~~~~~~~~~~~~~

When reporting issues, we ask users to please make every effort to collect and provide as much of the information identified below as possible.
We understand that thorough issue reporting can be onerous.
At the same time, being as thorough as possible in issue reporting is a key way users can reciprocate for the free and open source software projects they benefit from.
In addition, the more information that is included in an issue report, the more likely the issue can be diagnoseed quickly and a work-around or resolution developed.

For these reasons, we urge users to please provide as much of the following information as practical...

* Operating system name and version (e.g. ``macOS 12.7 Monterey``).
* Version number of VisIt_.
* The specific release asset (e.g. ``visit3_3_3.linux-x86_64-rhel7-wmesa.tar.gz``) installed or if VisIt_ was built from sources, the ``build_visit`` command-line used (often available in the ``build_visit_log`` file where VisIt_ was built).
* If you are running `client/server <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/using_visit/ClientServer/index.html?highlight=client>`__, then provide the above 3 items of information for both the client and the server machines.
* If reporting issues involving the CLI and you are not running the Python installed with VisIt_, then please provide the python version information.
* The plot(s) and operator(s), if any, being used.
* The database plugin type, if any, being used.
* Any `expression(s) <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/using_visit/Quantitative/Expressions.html>`__ in use at the time the issue was encountered. 
* Any `subset selection <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/using_visit/Subsetting/Using_the_Subset_Window.html?highlight=SIL#using-the-subset-window>`__ in effect, if any, at the time the issue was encountered.
* The `rendering mode <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/using_visit/Preferences/Rendering_Options_Window.html?highlight=rendering%20mode#rendering-options-window>`__ VisIt_ was in at the time of the issue.
* Error messages that were observed (Please cut-n-paste actual text whenever possible instead of taking a screenshot).
* VisIt_ debug logs (by running and reproducing the issue with ``-debug 5`` on the command line...which will produce ``.vlog`` files). Please see `debugging tips <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/dev_manual/DebuggingTips.html?highlight=debugging>`__.
* Stack dumps if you are able to provide them.
* Any other advanced features you have been using that may be contributing to the issue.

Not all items above will be relevant in all circumstances so please use your best judgement.
