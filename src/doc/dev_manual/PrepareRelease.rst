Preparing for a Release
=======================

Updating copyright notice dates
-------------------------------

At the beginning of every calendar year, the copyright notice needs to be updated.
There are only a handful of files that still contain copyright dates, including "/src/LICENSE".
There is a script called "update_copyright" in "/src/tools/dev/scripts" that can be used but may need updating if copyright dates were added to any new file types.
This should be kept to a minimum.

Preparing for a Patch Release
-----------------------------

Preparing for a minor release is pretty straightforward and consists of
updating a few files. These consist of ::

    VERSION
    INSTALL_NOTES
    gui/Splashscreen.C

A ticket should be created and assigned so that the release can be tested for any obsolete code that should be removed.
Testing for obsolete code involves configuring with the CMake var **VISIT_REMOVE_OBSOLETE_CODE** turned on, then compiling and looking for compile errors of the form:  *This code is obsolete in this version. Please remove it.*

Preparing for a Minor Release
-----------------------------

Preparing for a minor release consists of performing all the steps involved
in preparing for a patch release, along with some additional ones, such as
creating the release candidate branch and updating the splash screen.

Creating the Release Candidate Branch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creating a release candidate branch is just like creating a normal branch.
Here are the steps used to create the 3.1RC. ::

    git checkout develop
    git pull
    git checkout -b 3.1RC
    git push --set-upstream origin 3.1RC

Updating the Splashscreen
~~~~~~~~~~~~~~~~~~~~~~~~~

The splashscreen is the first thing the user sees when running VisIt_ so the
version number included in the splashscreen image should be up to date.
Updating the splashscreen usually means just updating the version number in
the current splashscreen images but in the event of a major or minor release
(when the first or second digit in the version changes), the splashscreen
images should be redesigned to showcase new features.

There are two image files associated with the splashscreen, both of which
are `XCF <https://xcf.berkeley.edu>`_. XCF files are the native image format
of the `GIMP <https://www.gimp.org>`_ image-editing program. One is for the
splashscreen and the second is for the icon on MacOS X. They are both used
as the first step in the process to create the splashscreen and icon.

The rest of this section will be focused on updating the version number.
Changing the images would be the same in terms of the mechanics involved
except that it would involve more editing of the image files.

Changing the version on the splashscreen
""""""""""""""""""""""""""""""""""""""""

Follow these steps to update the version on the splashscreen.

1. Go to the ``src/common/icons`` directory.
2. The splashscreen image's XCF files are named ``VisIt3.0.xcf``, ``VisIt3.1.xcf``, etc.
3. Copy the file from the last version to the new name for the current version.
4. Open the file in GIMP.

.. figure:: images/Release-GimpImage.png

   The splashscreen in GIMP.

You'll see that the file has several layers to it. There are four layers for
each of the four splash screen images that get randomly choosen from when
starting VisIt_ or are cycled through when you select *About* in the *Help*
menu.

4. Select the text layer containing the version number and change it.
5. Save the file.

Now you are ready to create the png images that are actually read in
by Qt. When you open the XCF file all the layers corresponding to the four
different splashscreen images will be enabled. When you save the first image
you will have them all shown. To save the second image you will hide the
layer corresponding to the first splashscreen image. You will successively
hide one additional layer until you have saved all four of the png images.

6. Go to *File->Export As* and change *Name* to ``VisIt1.png``.
7. Click on *Export*. 
8. Click on *Export* on the window that pops up to allow you to set the save options.
9. Hide ``Background1``.

.. figure:: images/Release-GimpLayers.png

   Hiding the Background1 layer in GIMP.

10. Repeat steps 6 - 9, saving images ``VisIt2.png``, ``VisIt3.png`` and ``VisIt4.png``.

The images saved by GIMP result in warning messages when read by Qt. To
modify the images so that the warning message disappears do the following. ::

    convert VisIt1.png VisIt1a.png
    convert VisIt2.png VisIt2a.png
    convert VisIt3.png VisIt3a.png
    convert VisIt4.png VisIt4a.png
    mv VisIt1a.png VisIt1.png
    mv VisIt2a.png VisIt2.png
    mv VisIt3a.png VisIt3.png
    mv VisIt4a.png VisIt4.png

11. Copy the files to ``src/resources/images``.

Changing the version on the MacOS X icon
""""""""""""""""""""""""""""""""""""""""

When VisIt_ starts on MacOS X systems, it adds an icon into the Mac
application dock. The icon that we use is based on the splashscreen but
is stored in MacOS X icon format.

Follow these steps to update the version on the MacOS X icon.

1. Go to the ``src/common/icons`` directory.
2. Create the directory ``VisItIcon.iconset``.
3. Open the file ``VisIt3.x-square.xcf`` in GIMP.
4. Select the text layer containing the version number and change it.
5. Go to *Image->Scale Image*.
6. Change the *Image Size* *Width* and *Height* to ``1024``.
7. Click on *Scale*.
8. Go to *File->Export As* and change *Name* to ``VisItIcon.iconset/icon_512x512@2x.png``.
9. Click on *Export*. 
10. Click on *Export* on the window that pops up to allow you to set the save options.

Now you need to create several sizes of the file. You will use ImageMagick
for this. ::

    cd VisItIcon.iconset
    convert -geometry 512x512 icon_512x512@2x.png icon_512x512.png
    convert -geometry 512x512 icon_512x512@2x.png icon_256x256@2x.png
    convert -geometry 256x256 icon_512x512@2x.png icon_256x256.png
    convert -geometry 256x256 icon_512x512@2x.png icon_128x128@2x.png
    convert -geometry 128x128 icon_512x512@2x.png icon_128x128.png
    convert -geometry 64x64 icon_512x512@2x.png icon_32x32@2x.png
    convert -geometry 32x32 icon_512x512@2x.png icon_32x32.png
    convert -geometry 32x32 icon_512x512@2x.png icon_16x16@2x.png
    convert -geometry 16x16 icon_512x512@2x.png icon_16x16.png

Now you will use iconutil to create the icns file. Note that iconutil
is only available on the Mac. ::

    cd ..
    iconutil --convert icns VisItIcon.iconset

Creating a new release notes file
"""""""""""""""""""""""""""""""""

A final step in making a release is to create the release notes file for the *next*
release. To do this, you must be reasonably certain what the next release's version
number will be. Typically, we do 3-4 patch releases for each minor release. So,
if the release you are *just now* making is version ``3.1.2``, then the *next* release
is likely to be ``3.1.3``. However, if the current release is ``3.1.3``, the next
release might be ``3.1.4`` or it might be ``3.2``.

In any event, to make the release notes file for the *next* release, you need to create
an new, empty release notes file by going to ``src/resources/help/en_US`` and copying
either the *minor* release notes template, ``relnotes_minor_templ.html``, or the *major*
release notes template, ``relnotes_major_templ.html`` to a file name of the form
``relnotesA.B.C.html`` where ``A.B.C`` is the version number for the *next* release.
The ``.C`` part of the file name is missing for *minor* releases.

Patch release notes should go on the RC branch (e.g. ``3.1RC``) and minor release notes
should go on ``develop``. *Always* assume there will be another patch release
and just create the next patch release file. If there isn't another patch release, the
notes from the patch release can be incorporated into the minor release notes file.
When finishing a minor release, create the files for the next minor release *and* the
next patch release.

Manual Smoke Check Testing Check List
-------------------------------------

The following is a list of manual tests to perform once a release has been packaged.

GUI Checks
~~~~~~~~~~

1. Plot Pseudocolor and Mesh plots for nodal data from curv2D.silo.
2. Plot Pseudocolor and Mesh plots for zonal data from multi_ucd3d.silo.
3. Test Navigation mode (rotate, pan, zoom).
4. Test rubberband zoom.
5. Execute a Node and Zone Pick.
6. Execute a Pick Query.
7. Execute a Lineout.
8. Check for Release Notes and Help.
9. Check VisIt manual was populated in Help.
10. Test "Make Movie" with dba00.pdb.

CLI Checks
~~~~~~~~~~
1. Start VisIt with CLI and check that `import numpy` works.
2. Test `import visitmodule`.

Additional macOS Checks
~~~~~~~~~~~~~~~~~~~~~~~

1. Check install names for ``@rpath``.
2. Test Parallel Launch by plotting procid expr on multi_ucd3d.silo.
3. Make sure to test both the DMG / App Bundle package and the ``tar.gz`` package.
4. Under Options->Appearance, make sure the GUI style has the macintosh option.
5. Verify OSpray is installed (look at the 'Advanced' tab under Options->Rendering...).
6. Verify that the xmledit tool works from the bundle (/Application/VisIt.app/Contents/Resources/bin/xmledit).
7. Verify that the DMG has been signed with a Developer ID and works properly.
8. Try descending into ``Downloads``, ``Documents`` and ``Desktop`` from an instance launched by double-clicking the icon and from an instance launched from the Terminal command line.

Preparing for a Major Release
-----------------------------

Preparing for a major release is the same as preparing for a minor release
with the addition of putting VisIt_ through the Information Management
software release process.
