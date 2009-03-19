dnl                            Copyright 2000 - 2008
dnl                 The Regents of the University of California.
dnl                             All rights reserved.
dnl
dnl This work was produced at the University of California, Lawrence Livermore
dnl National Laboratory (UC LLNL)  under contract no.  W-7405-ENG-48 (Contract
dnl 48) between  the U.S. Department of Energy  (DOE)  and The Regents  of the
dnl University  of  California  (University)  for  the  operation of  UC LLNL.
dnl Copyright   is  reserved to  the   University for  purposes  of controlled
dnl dissemination,   commercialization  through  formal licensing,   or  other
dnl disposition under  terms of Contract   48; DOE policies,  regulations  and
dnl orders; and  U.S. statutes.  The    rights of the  Federal Government  are
dnl reserved under Contract 48 subject to  the restrictions agreed upon by DOE
dnl and University.
dnl
dnl                                 DISCLAIMER
dnl
dnl This software was prepared as an account of work sponsored by an agency of
dnl the United States Government. Neither the United States Government nor the
dnl University  of California nor any of  their employees, makes any warranty,
dnl express or implied,  or assumes  any liability  or responsiblity  for  the
dnl accuracy, completeness, or    usefullness of any  information,  apparatus,
dnl product, or  process   disclosed, or represents that  its   use would  not
dnl infringe privately    owned  rights. Reference   herein  to  any  specific
dnl commercial  products, process,  or    service by trade   name,  trademark,
dnl manufacturer, or otherwise, does  not necessarily constitute or imply  its
dnl endorsement, recommendation, or  favoring by the United  States Government
dnl or the   University  of  California.  The views   and  opinions of authors
dnl expressed herein do not  necessarily state or  reflect those of the United
dnl States Government  or the University of  California, and shall not be used
dnl for advertising or product endorsement purposes.
dnl
dnl Macros related to the Tuvok volume rendering library.
dnl
dnl Modifications:
dnl
dnl   Tom Fogal, Thu Mar  5 11:26:51 MST 2009
dnl   s/Tuvok/tuvok/, where appropriate.

dnl provide an --enable-tuvok option and an environment variable to enable
dnl Tuvok.  If Tuvok is enabled:
dnl     . subst TUVOK_LIB to the appropriate linker options,
dnl     . subst TUVOK_INCLUDE to the appropriate CPPFLAGS,
dnl     . subst BUILD_TUVOK to "Tuvok",
dnl     . define USE_TUVOK in the configuration file.
AC_DEFUN([AX_TUVOK], [
    AC_MSG_CHECKING([if Tuvok should be used])

    # Use the option if given on the configure line, else use TUVOK_ENABLE in
    # the environment (should it be set).
    AC_ARG_ENABLE([tuvok],
        [AS_HELP_STRING([--enable-tuvok],
                        [Enable Tuvok-based volume renderers.])],
        [],
        [AS_IF([test -n "${TUVOK_ENABLE}"],
               [enable_tuvok="${TUVOK_ENABLE}"],
               [enable_tuvok="no"])
        ]
    )

    BUILD_TUVOK=
    TUVOK_LIB=
    TUVOK_INCLUDE=
    AS_IF([test "x${enable_tuvok}" != "xno"],
          [
            AC_SUBST([BUILD_TUVOK], ["tuvok"])
            AC_SUBST([TUVOK_LIB], ["-ltuvok"])
            AC_SUBST([TUVOK_INCLUDE], ["-I$""(TOPDIR)/include/tuvok"])
            AC_DEFINE([USE_TUVOK], [1], [Define to enable Tuvok-based code])
            report_tuvok="yes"
          ],
          [report_tuvok="no"]
    )

    AC_MSG_RESULT([$report_tuvok])
])
