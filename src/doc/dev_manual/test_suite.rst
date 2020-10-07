Regression Testing
============================

Overview
--------
VisIt_ has a large and continually growing test suite. VisIt_'s test
suite involves a combination python scripts
in `src/test`, raw data and data generation sources in `src/testdata`
and of course the VisIt_ sources themselves. Regression tests are
run on a nightly basis. Testing exercises VisIt_'s viewer,
mdserver, engine and cli but not the GUI.


Running regression tests
--------------------------------

Where nightly regression tests are run
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The regression suite is run on `LLNL's Pascal Cluster <https://hpc.llnl.gov/hardware/platforms/pascal>`_. Pascal runs the TOSS3 operating system, which is a flavor of Linux. If you are going to run the regression suite yourself you should run on a similar system or there will be differences due to numeric precision issues.

The regression suite is run on Pascal using a cron job that checks out VisIt_ source code, builds it, and then runs the tests.

How to run the regression tests manually
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The regression suite relies on having a working VisIt_ build and test data available on your local computer.
Our test data and baselines are stored using git lfs, so you need to setup git lfs and pull to have all the necessary files. 

The test suite is written in python and to source is in `src/test`. 
When you configure VisIt_, a bash script is generated in the build directory that you can use to run the test
suite out of source with all the proper data and baseline directory arguments. ::

    cd visit-build/test/
    ./run_visit_test_suite.sh


Here is an example of the contents of the generated `run_visit_test_suite.sh` script ::

    /Users/harrison37/Work/github/visit-dav/visit/build-mb-develop-darwin-10.13-x86_64/thirdparty_shared/third_party/python/2.7.14/darwin-x86_64/bin/python2.7  
    /Users/harrison37/Work/github/visit-dav/visit/src/test/visit_test_suite.py \
       -d /Users/harrison37/Work/github/visit-dav/visit/build-mb-develop-darwin-10.13-x86_64/build-debug/testdata/  \
       -b /Users/harrison37/Work/github/visit-dav/visit/src/test/../../test/baseline/   \
       -o output \
       -e /Users/harrison37/Work/github/visit-dav/visit/build-mb-develop-darwin-10.13-x86_64/build-debug/bin/visit "$@"


Once the test suite has run, the results can be found in the `output/html` directory. Open `output/html/index.html` in a web browser to view the test suite results.

Accessing regression test results
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The nightly test suite results are posted to: http://portal.nersc.gov/project/visit/.

In the event of failure on the nightly run
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If any tests
fail, ''all'' developers who updated the code from the last time all
tests successfully passed will receive an email indicating what failed.
In addition, failed results should be available on the web.  

If the results fail to post, the visit group on NERSC's systems may be over quota.
If you have a NERSC account you can check usage by sshing to NERSC and running the following command::

    prjquota visit

How regression testing works
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The workhorse script that manages the testing is `visit_test_suite.py` in 
`src/test`. Tests can be run in a variety of ways called *modes*.
For example, VisIt_'s nightly testing is run in `serial`, `parallel`
and `scalable,parallel` modes. Each of these modes represents a fundamental and
relatively global change in the way VisIt_ is doing business
under the covers during its testing. For example, the difference
between `parallel` and `scalable,parallel` modes is whether the scalable
renderer is being used to render images. In the `parallel` mode,
rendering is done in the viewer. In `scalable,parallel` mode, it
is done, in parallel, on the engine and images from each processor
are composited. Typically, the entire test suite is run in each
mode specified by the regression test policy.

There are a number
of command-line options to the test suite. `./run_visit_test_suite.sh -help`
will give you details about these options. Until we are
able to get re-baselined on the systems available outside of LLNL firewalls,
options enabling some filtering of image differences will be very useful.
Use of these options on platforms other than the currently adopted testing
platform (pascal.llnl.gov) will facilitate filtering big
differences (and probably real bugs that have been introduced)
from differences due to platform where tests are run. See the section on
filtering image differences.

There are a number of different categories of tests. The test
categories are the names of all the directories under
`src/test/tests`. The .py files in this directory tree are all
the actual test driver files that drive VisIt_'s CLI and
generate images and text to compare with baselines. In addition,
the `src/test/visit_test_main.py` file defines a number of helper Python
functions that facilitate testing including two key functions;
`Test()` for testing image outputs and `TestText()` for testing text
outputs. Of course, all the .py files in `src/test/tests` subtree
are excellent examples of test scripts.

When the test suite 
finishes, it will have created a web-browseable HTML tree in
the html directory. The actual image and text raw results
will be in the current directory and difference images will
be in the diff directory. The difference images are essentially
binary bitmaps of the pixels that are different and not the
actual pixel differences themselves. This is to facilitate
identifying the location and cause of the differences.

Adding a test involves a) adding a .py file to the appropriate
subdirectory in `src/test/tests`, b) adding the expected baselines
to `test/baselines` and, depending on the test, c) adding
any necessary input data files to `src/testdata`. 
The test suite will find your added .py files the next time it runs. 
So, you don't have to do anything special other than adding the .py file.

One subtlety about the current test modality is what we call
`mode specific baselines`. In theory, it should not matter what
mode VisIt_ is run in to produce an image. The image should be
identical across modes. In practice there is a long list of
things that can contribute to a handful of pixel differences
in the same test images run in different modes. This has lead
to mode specific baselines. In the baseline directory, there
are subdirectories with names corresponding to modes we currently
run. When it becomes necessary to add a mode specific baseline,
the baseline file should be added to the appropriate baseline
subdirectory.

In some cases, we skip a test in one mode but
not in others. Or, we temporarily disable a test by skipping it
until a given problem in the code is resolved. This is handled
by the ``--skiplist`` argument to the test suite. We maintained list of the
tests we currently skip and update it as necessary.
The default skip list file is `src/test/skip.json`.

Filtering Image Differences
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
There are many alternative ways for both compiling and even running VisIt_ to
produce any given image or textual output. Nonetheless, we expect results to
be nearly if not perfectly identical. For example, we expect VisIt_ running on
two different implementations of the GL library to produce by and large the same
images. We expect VisIt_ running in serial or parallel to produce the same
images. We expect VisIt_ running on Ubuntu Linux to produce the same images as
it would running on Mac OSX. We expect VisIt_ running in client-server mode to
produce the same images as VisIt_ running entirely remotely.

In many cases, we expect outputs produced by these alternative approaches to be
nearly the same but not always bit-for-bit identical. Minor variations such as
single pixel shifts in position or slight variations in color are inevitable
and ultimately unremarkable.

When testing, it would be nice to be able to ignore variations in results
attributable to these causes. On the other hand, we would like to be alerted
to variations in results attributable to changes made to the source code.

To satisfy both of these goals, we use bit-for-bit identical matching to
track the impact of changes to source code but *fuzzy* matching for anything
else. We maintain a set of several thousand version-controlled, baseline results
computed for a specific, fixed *configuration and test mode* of VisIt_. Nightly
testing of key branches of development reveals any results that are not
bit-for-bit identical to their baseline.

These *failures* are then corrected in one of two ways. Either the new result
is wrong and additional source code changes are required to ensure VisIt_
continues to produce the original baseline. Or, the original baseline is wrong
and it must be updated to the new result. In this latter situation, it is also
prudent to justify the new result with a plausible explanation as to why it is
expected, better or acceptable as well as to include such explanation in the
commit comments.

Mode specific baselines
"""""""""""""""""""""""
VisIt_ testing can be run in a variety of modes; serial, parallel,
scalable-parallel, scalable-parallel-icet, client-server, etc. For a fixed
configuration, in most cases baseline results computed in one mode agree
bit-for-bit identically with the other modes. However, this is not always
true. About 2% of results vary with the execution mode. To handle these cases,
we also maintain *mode-specific* baseline results as the need arises.

The need for a mode-specific baseline is discovered as new tests are added.
When testing reveals that VisIt computes slightly different results in 
different modes, a single mode-agnostic baseline will fail to match in all
test modes. At that time, mode-specific baselines are added.

Changing Baseline Configuration
"""""""""""""""""""""""""""""""
One weakness with this approach to testing is revealed when it becomes
necessary to change the configuration used to compute the baselines. For example,
moving VisIt_'s testing system to a different hardware platform or updating to a
newer compiler or third-party library such as VTK, may result in a slew of minor
variations in the results. Under these circumstances, we are confronted with
having to individually assess possibly thousands of *minor* image differences
to rigorously determine whether the new result is in fact *good* or whether some
kind of issue or bug is being revealed.

In practice, we use fuzzy matching (see below) to filter out *minor* variations
from *major* ones and then focus our efforts only on fully understanding the
*major* cases. We summarily *accept* all minor variations as the *new*
baselines.

Promise of Machine Learning
"""""""""""""""""""""""""""
In theory, we should be able to develop a machine-learning approach to
filtering VisIt_'s test results that enable us to more effectily attribute
variations in results to various causes. A challenge here is in developing
a sufficiently large and fully labeled set of example results to prime the
machine learning. This would make for a great summer project.

Fuzzy Matching Metrics
""""""""""""""""""""""
Image difference metrics are reported on terminal output and in HTML reports.

Total Pixels (``#pix``) :
    Count of all pixels in the test image

Non-Background (``#nonbg``) :
    Count of all pixels which are not background either by comparison to constant
    background color or if a non-constant color background is used to same pixel in background
    image produced by drawing with all plots hidden. Note that if a plot produces a pixel which
    coincidentally winds up being the same color as the background, our accounting logic would
    count it as *background*. We think this situation is rare enough as to not cause serious issues.

Different (``#diff``) :
    Count of all pixels that are different from the current baseline image.

% Diff. Pixels (``~%diff``) :
    The *precentage* of different pixels computed as ``100.0*#diff/#nonbg``

Avg. Diff (``avgdiff``) :
    The average *luminance* (gray-scale, obtained by weighting RGB channels by 1/3rd
    and summing) difference. This is the sum of all pixel luminance differences
    divided by ``#diff``.

Fuzzy Matching Thresholds
"""""""""""""""""""""""""
There are some command-line arguments to run tests that control *fuzzy* matching.
When computed results match bit-for-bit with the baseline, a **PASS** is reported
and it is colored green in the HTML reports. When a computed result fails the
bit-for-bit match but passes the fuzzy match, a **PASS** is reported on the terminal
and it is colored yellow in the HTML reports.

Pixel Difference Threshold (``--pixdiff``) :
    Specifies the acceptable threshold for the ``#diff`` metric as a *percent*. Default
    is zero which implies bit-for-bit identical results.

Average Difference Threshold (``--avgdiff``) :
    Specifies the acceptable threshold for the ``avgdiff`` metric. Note that this threshold
    applies *only* if the ``--pixdiff`` threshold is non-zero. If a test is above the
    ``pixdiff`` threshold but below the ``avgdiff`` threshold, it is considered a **PASS**.
    The ``avgdiff`` option allows one to specify a second tolerance for the case when
    the ``pixdiff`` tolerance is exceeded.

Numerical (textual) Difference Threshold (``--numdiff``) :
    Specifies the acceptable *relative* numerical difference threshold in computed,
    non-zero numerical results. The relative difference is computed as the ratio of the
    magnitude of the difference between the current and baseline results and the minimum
    magnitude value of the two results.

The command-line with ``--pixdiff=0.5 --avgdiff=0.1`` means that any result with *fewer*
than 0.5% of pixels that are different is a **PASS** and anything with more than 0.5% of
pixels different but where the average pixel gray-scale difference is less than .1 is
still a **PASS**.

Testing on Non-Baseline Configurations
""""""""""""""""""""""""""""""""""""""

When running the test suite on platforms other than the currently adopted baseline
platform or when running tests in modes other than the standard modes, the ``--pixdiff``
and ``--avgdiff`` command-line options will be very useful.

For numerical textual results, there is also a ``--numdiff`` command-line option
that specifies a *relative* numerical difference tolerance in numerical textual
results. The command-line option ``--numdiff=0.01`` means that if a numerical
result is different but the magnitude of the difference divided by the magnitude of
the expected value is less than ``0.01`` it is considered a **Pass**.

When specified on the command-line to a test suite run, the above tolerances wind
up being applied to *all* test results computed during a test suite run. It is
also possible to specify these tolerances in specific tests by passing them as
arguments, for example ``Test(pixdiff=4.5)`` and ``TestText(numdiff=0.01)``, in
the methods used to check test outputs.

Finally, it may make sense for developers to generate (though not ever commit) a
complete and validated set of baselines on their target development platform and
then use those (uncommitted) baselines to enable them to run tests and track code
changes using an exact match methodology.
 
Tips on writing regression tests 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Test images in which plots occupy a small portion of the total image are fraught with peril and should be avoided. Images with poor coverage are more likely to produce false positives (e.g. passes that should have failed) or to exhibit somewhat random differences as test scenario is varied.

* Except in cases where annotations are being specifically tested, remember to call TurnOffAllAnnotations() as one of the first actions in your test script. Otherwise, you can wind up producing images containing machine-specific annotations which will produce differences on other platforms.

* When writing tests involving text differences and file pathnames, be sure that all pathnames in the text strings passed to `TestText()` are absolute. Internally, VisIt_ testing system will filter these out and replace the machine-specific part of the path with `VISIT_TOP_DIR` to facilitate comparison with baseline text. In fact, the .txt files that get generated in the `current` dir will have been filtered and all pathnames modified to have `VISIT_TOP_DIR` in them.

* Here is a table of python tests scripts which serve as examples of some interesting and lesser known VisIt_/Python scripting practices:

+-----------------------------------+--------------------------------------------------------------------+
| Script                            | What it demonstrates                                               |
+===================================+====================================================================+
|tests/faulttolerant/savewindow.py  |  * uses python exceptions                                          |
+-----------------------------------+--------------------------------------------------------------------+
| tests/databases/itaps.py          |  * uses OpenDatabase with specific plugin                          |
|                                   |  * uses SIL restriction via names of sets                          |
+-----------------------------------+--------------------------------------------------------------------+
|tests/databases/silo.py            |  * uses OpenDatabase with virtual database and a specific timestep |
+-----------------------------------+--------------------------------------------------------------------+
|tests/rendering/scalable.py        |  * uses OpenComputeEngine to launch a parallel engine              |
+-----------------------------------+--------------------------------------------------------------------+
|tests/rendering/offscreensave.py   |  * uses Test() with alternate save window options                  |
+-----------------------------------+--------------------------------------------------------------------+
|tests/databases/xform_precision.py |  * uses test-specific enviornment variable settings                |
+-----------------------------------+--------------------------------------------------------------------+


Rebaselining Test Results
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A python script, `rebase.py`, at `src/tests` dir can be used to rebaseline large numbers of results. In particular, this script enables a developer to rebase test results without requiring access to the test platform where testing is performed. This is becase the PNG files uploaded (e.g. posted) to VisIt_'s test results dashboard are suitable for using as baseline results. To use this script, run `./rebase.py --help.` Once you've completed using rebase.py to update image baselines, don't forget to commit your changes back to the repository.

 
Using VisIt_ Test Suite for Sim Code Testing
----------------------------------------------
VisIt_'s testing infrastructure can also be used from a VisIt_ install by simulation codes 
how want to write their own Visit-based tests.
For more details about this, see:  `Leveraging VisIt_ in Sim Code RegressionTesting <http://visitusers.org/index.php?title=Leveraging_VisIt_in_Sim_Code_Regression_Testing>`_ 


.. CYRUS NOTE: This info seems to old to be relevant, but keeping here commented out just in case. 
.. 
.. == Troubleshooting ==
..
.. === Mesa stub issue ===
.. IMPORTANT NOTE: After the cmake transition, there is no mesa-stub issue because the viewer does not compile in a stub for mesa since doing so was non-portable. Thus, if you are using the svn trunk version of VisIt_, you cannot run into this issue. This section is being preserved for 1.12.x versions of VisIt_.
..
.. If all of your tests fail, you have likely run into the Mesa stub issue.  The regression suite is set up to do "screen captures", but default VisIt_ cannot do screen captures in "-nowin" mode.  If you run a test with the "-verbose" command and see:
..  Rendering window 1...
..  VisIt: Message - Rendering window 1...
..  VisIt: Warning - Currently, you cannot save images when in nowin mode using screen capture
..  and Mesa has been stubbed out in the viewer.  Either disable screen capture, or rebuild
..  without the Mesa stub library.  Note that the Mesa stub library was in place to prevent
..  compatibility problems with some graphics drivers.
..  Saving window 1...
..
.. then you have gotten bit by this problem.
..
.. You can correct it by running configure with:
..  --enable-viewer-mesa-stub=no
..
.. In fact, the typical configure line on davinci is:
..  ./configure CXXFLAGS=-g MAKE=gmake --enable-parallel --enable-visitmodule --enable-viewer-mesa-stub=no --enable-buildall
..
..
.. IMPORTANT NOTE: this will not automatically touch the files that need to be recompiled.  Your best bet is to touch viewer/main/*.C and recompile that directory.
..
.. You can test the Mesa stub issue with:
..   % visit -cli -nowin
..  >>> sw = SaveWindowAttributes()
..  >>> sw.screenCapture = 1
..  >>> SetSaveWindowAttributes(sw)
..  >>> SaveWindow()
..
.. If VisIt_ complains about an empty window, you do *not* have a Mesa stub issue and you *can* run regression tests.  If it complain about Mesa stubs, then you *do* have the issue and you *can't* run regression tests.
..
.. === PIL on MacOS X ===
.. If you attempt to execute runtest and it gives errors indicating that it assumed the test crashed then you might have problems with your PIL installation. These manifest as an error with text like ''"The _imaging C module is not installed"'', which can be obtained if you add the '''-v''' argument to ''runtest''.
..
.. PIL, as installed by build_visit, can pick up an invalid jpeg library on certain systems. If you run ''python -v'' and then try to ''import _imaging'' then Python will print out the reason that the library failed to import. This can often be due to missing jpeg library symbols. It is also possible to observe this situation even when libjpeg is available in /sw/lib but is compiled for a different target architecture (e.g. not x86_64) that what build_visit is using. The effect of this is that when _imaging.so library is linked, there is an error message saying saying something like...
..
..  ld: warning: ignoring file /opt/local/lib/libz.dylib, file was built for x86_64
..     which is not the architecture being linked (i386): /opt/local/lib/libz.dylib
..  ld: warning: ignoring file /sw/lib/libjpeg.dylib, file was built for i386
..    which is not the architecture being linked (x86_64): /sw/lib/libjpeg.dylib
..
.. . Later, when Python trys to import _imaging module, the dlopen fails due to unresolved jpeg symbol. Either way, the best solution the following:
..
.. # Build your own jpeg library
.. # Edit PIL's setup.py, setting JPEG_ROOT=libinclude("/path/to/my/jpeg")
.. # python ./setup.py build
.. # Look through the console output for the command that links the ''_imaging.so'' library and paste it back into the console as a new command. Edit the command so it uses /path/to/my/jpeg/lib/libjpeg.a instead of the usual -L/path -ljpeg business so it really picks up your jpeg library.
.. # python ./setup.py install
..
.. That is a painful process to be sure but it should be enough to produce a working PIL on Mac.
..
..
.. Here is a slightly easier way that I (Cyrus) was able to get PIL working on OSX:
.. * Build your own jpeg library
.. * Edit PIL's setup.py, do not modify JPEG_ROOT, instead directly edit the darwin case:
.. <source lang="python">
..         elif sys.platform == "darwin":
..             add_directory(library_dirs, "/path/to/your/jpeg/v8/i386-apple-darwin10_gcc-4.2/lib")
..             add_directory(include_dirs, "/path/to/your/jpeg/v8/i386-apple-darwin10_gcc-4.2/include")
..             # attempt to make sure we pick freetype2 over other versions
..             add_directory(include_dirs, "/sw/include/freetype2")
.. </source>
.. * python setup.py build
.. * python setup.py install
..
.. == Skeleton for future content ==
..
.. === Modes ===
..
.. ==== Mode specific baselines ====
..
.. == Compiler Warning Regression Testing ==
..
.. [[Category: Developer documentation]]
..
.. The ultimate aim of compiler warning testing is to improve the quality of the code by averting ''would-be'' problems. However, in the presence of an already robust, run-time test suite, compiler warnings more often than not alert us to ''potential'' problems and not necessarily any real bugs that manifest for users.
..
.. Totally eliminating compiler warnings is a good goal. But, it is important to keep in mind that that goal is really only ''indirectly'' related to improving code quality. Its also important to keep in mind that all warnings are not equal nor are all compilers equal to the task of detecting and reporting them. For example, an ''unused variable'' warning in a code block may be a potential code maintenance nuisance but will not in any way manifest as a bug for a user.
..
.. As developers, when we ''fix'' warnings we typically take action by adjusting code. But, we are doing so in response to one compiler's (often myopic) view of the code and typically not to any real bug encountered by a user. We need to take care the the adjustments we make lead to improved quality. In particular, adjusting code for no other purpose except to silence a given compiler warning seems an unproductive exercise. Besides, there are many other options for managing unhelpful compiler warnings apart from adjusting actual code.
..
.. Finally, we're introducing compiler warning checking into a code that has been developed for many years by many developers without having payed significant attention to this issue. As of this writing, the existing code generates thousands of warnings. To make matters worse, we are dialing up compiler options to report as many warnings as possible. This leads to two somewhat distinct problems. One is to resolve warning issues in the existing code. The other, and the more important long term goal, is to prevent further warning issues from being introduced into the code.
..
.. If we take the appraoch that we must achieve the first '''before''' we can start on the second, we wind up holding our long term goal hostage to the laborious and resource intensive task of addressing existing warning issues. Or, we hold a gun to everyone's head to drop whatever they are doing and spend time addressing existing warnings to eliminate ''noise'' from useful warnings.
..
.. But, we don't have to do either of these. Instead, we can add logic to our regression testing framework to detect the introduction of ''new'' warning issues apart from existing warnings and then only fail the test when ''new'' warnings are introduced.
..
.. Here's how it works. A new unit test was added, <tt>test/tests/unit/compiler_warnings.py</tt>. That test checks for the existence of a file <tt>make.err</tt> just ''above'' the <tt>src, test and data</tt> dirs (thats because thats where the <tt>regressiontest_edge</tt> shell script puts it). If <tt>../make.err</tt> is not found, the test immediately exits with the ''skip'' error code indication. It is assumed that <tt>../make.err</tt> was produced from the ''current'' source code with compiler warnings dialed up (e.g. <tt>-Wall -Wextra -pedantic</tt>) and <tt>stderr</tt> output from an entire ''clean'' build of the source is captured with a version of make supporing the <tt>--output-sync=lines</tt> option (or make was not run with a -j option).
..
.. The compiler_warnings.py python script examines make.err for lines containing warning. For each source file that produces a warning, a count of all warnings produced by the file is computed. A text string result suitable for input to the TestText method of VisIt_'s regression testing framework is assembled. Source filenames are sorted and then emitted along with their warning counts. The resulting text string is also a JSON string. It is this single text result that is checked for ''changes''. Note that any changes, up or down, in compiler warning counts for any source file, as well as introduction or elimination of a source file from compiler warning list, will result in a test failure.
..
.. If enough files were changed in the previous day's work, it's conceivable changes from multiple developer's commits will result in changes (some improvements and some not) to various lines of this text output. Improvements should be re-baselined. Non-improvements should be checked and ''fixed''.
..
.. To re-basline the warning count for a given source file, simply edit the <tt>compiler_warnings_by_file.txt</tt> file as appropriate. Its structure is designed for easy editing with any text editor.
..
.. To ''fix'' a new warning, there are several options. The first is to adjust the code that generated the warning. Its probably something minor and probably should be fixed. However, if the warning is itself unhelpful and fixing it will not improve the code, you can add the warning to a skip list. There is a file, <tt>compiler_warning_skips.json</tt> which contains skips for specific source files and skips for all (e.g. global) source files. This json file is read in as a python dictionary. You can simply cut the text for the warning that gets posted in the html to this file. Finally, as a last resort, you can also elect to bump up the warning count for the given source file. But, these later actions should be taken with care and perhaps vetted with other developers first.

