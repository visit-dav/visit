.. _Qt_X11:

Linux X11 Requirements for Qt
=============================

On Linux, Qt has specific X11 requirements that must be met for its Platform Plugin to be built successfully.
Many developers encounter errors building Qt due to missing support libraries related to X11 and xcb.

Here are a couple of Qt links that prove helpful in knowing which packages need to be installed for particular OS distros.

* For Qt 5
  * The libxcb section of `Building Qt5 from Git <https://wiki.qt.io/Building_Qt_5_from_Git>`_  
  * Qt5's `X11 Requirements <https://doc.qt.io/qt-5/linux-requirements.html>`__ page.

* For Qt 6 (only on develop branch)
  * Qt6's `X11 Requirements <https://doc.qt.io/qt-6/linux-requirements.html>`__ page.

VisIt_ also maintains a few Docker files that may prove useful:

* `centos8 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-centos8>`_

* `debian9 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-debian9>`_

* `debian10 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-debian10>`_

* `debian11 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-debian11>`_

* `fedora31 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-fedora31>`_

* `ubuntu18 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-ubuntu18>`_

* `ubuntu20 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-ubuntu20>`_

* `ubuntu21 <https://github.com/visit-dav/visit/blob/develop/scripts/docker/Dockerfile-ubuntu21>`_
