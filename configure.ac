#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([LAS], [development,] )
AC_PREREQ(2.57)
#AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([LasTgBoard/src/LasTgBoardSupervisor.cc])
#AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


echo '';
echo 'LAS configuration: '; 
echo '---------------------------------------------------------';

fromwhere=;

# -------------------------------------------------------------------------------------------
#
#    Required Settings 
#
# -------------------------------------------------------------------------------------------

xdaq_root=;
AC_ARG_WITH(xdaq-path, 
            [AC_HELP_STRING([--with-xdaq-path],[specify the path to your XDAQ installation or set $XDAQ_ROOT])],  
            [xdaq_root=$withval; fromwhere="cmd-line";], 
            []) 
AS_IF([test "x$xdaq_root" == xno],
      [xdaq_root=;],
      [])
AS_IF([test "x$xdaq_root" == x], 
       AS_IF([test "x$XDAQ_ROOT" != x], 
             [xdaq_root=$XDAQ_ROOT; fromwhere="environment";], 
             [echo ''; 
              echo "************************************************************************************";
  	      echo "XDAQ libraries are required. Please set the xdaq path using --with-xdaq-path=<path>"; 
	      echo "or using the XDAQ_ROOT environment variable."; 
	      echo "************************************************************************************"; 
              echo ''; exit; 
            ])
     )	
echo "XDAQ root            : $xdaq_root (from $fromwhere) "


xdaq_os=;
AC_ARG_WITH(xdaq-os, 
            [AC_HELP_STRING([--with-xdaq-os],[specify your os set XDAQ_OS (default linux)])],  
            [xdaq_os=$withval; fromwhere="cmd-line";], 
            []) 
AS_IF([test "x$xdaq_os" == xno],
      [xdaq_os=;],
      [])
AS_IF([test "x$xdaq_os" == x], 
       AS_IF([test "x$XDAQ_OS" != x], 
             [xdaq_os=$XDAQ_OS; fromwhere="environment";], 
             [echo ''; 
              echo "************************************************************************************";
              echo "XDAQ OS must be specified. Please set the xdaq os using --with-xdaq-os=<os>"; 
	      echo "or using the XDAQ_OS environment variable."; 
	      echo "************************************************************************************"; 
              echo ''; exit; 
            ]) 
     )	
echo "XDAQ OS              : $xdaq_os (from $fromwhere) "


xdaq_platform=;
AC_ARG_WITH(xdaq-platform, 
            [AC_HELP_STRING([--with-xdaq-platform],[specify your platform or set XDAQ_PLATFORM (default x86_slc4)])],  
            [xdaq_platform=$withval;fromwhere="cmd-line";], 
            []) 
AS_IF([test "x$xdaq_platform" == xno],
      [xdaq_platform=;],
      [])
AS_IF([test "x$xdaq_platform" == x], 
       AS_IF([test "x$XDAQ_PLATFORM" != x], 
             [xdaq_platform=$XDAQ_PLATFORM; fromwhere="environment";], 
             [echo ''; 
              echo '************************************************************************************';
	      echo 'XDAQ PLATFORM must be specified. Please set the xdaq platform using --with-xdaq-platform=<ptfrm>'; 
 	      echo 'or using the XDAQ_PLATFORM environment variable.'; 
	      echo '************************************************************************************'; 
              echo ''; exit; 
           ]) 
    )	
echo "XDAQ platform        : $xdaq_platform (from $fromwhere) "


hal_root=;
AC_ARG_WITH(hal-path, 
           [AC_HELP_STRING([--with-hal-path],[specify the path to HAL or set $ENV_CMS_TK_HAL_ROOT])],  
	   [hal_root=$withval; fromwhere="cmd-line";], 
           []) 
AS_IF([test "x$hal_root" == xno],
      [hal_root=;],
      [])
AS_IF([test "x$hal_root" == x], 
       AS_IF([test "x$ENV_CMS_TK_HAL_ROOT" != x], 
             [hal_root=$ENV_CMS_TK_HAL_ROOT; fromwhere="environment";], 
             [echo ''; 
              echo '************************************************************************************';
	      echo 'HAL libraries are required. Please set the hal path using --with-hal-path=<path>'; 
	      echo 'or using the ENV_CMS_TK_HAL_ROOT environment variable.'; 
	      echo '************************************************************************************'; 
              echo ''; exit; 
            ])
     )	
echo "HAL root             : $hal_root (from $fromwhere) "



caen_root=;
AC_ARG_WITH(caen-path, 
            [AC_HELP_STRING([--with-caen-path],[specify the CAEN path or set $ENV_CMS_TK_CAEN_ROOT])],  
            [caen_root=$withval; fromwhere="cmd-line";], 
            []) 
AS_IF([test "x$caen_root" == xno],
      [caen_root=;],
      [])
AS_IF([test "x$caen_root" == x], 
      AS_IF([test "x$ENV_CMS_TK_CAEN_ROOT" != x], 
            [caen_root=$ENV_CMS_TK_CAEN_ROOT; fromwhere="environment";], 
            [echo ''; 
             echo '************************************************************************************';
	     echo 'CAEN libraries are required. Please set the CAEN path using --with-caen-path=<path>'; 
	     echo 'or using the ENV_CMS_TK_CAEN_ROOT environment variable.';
	     echo '************************************************************************************'; 
             echo ''; exit; 
	   ])
   )	
echo "CAEN software root   : $caen_root (from $fromwhere) "



xerces_root=;
AC_ARG_WITH(xerces-path, 
            [AC_HELP_STRING([--with-xerces-path],[specify the path to Xerces or set $XERCESCROOT])],  
            [xerces_root=$withval; fromwhere="cmd-line";], 
            []) 
AS_IF([test "x$xerces_root" == x], 
      AS_IF([test "x$XERCESCROOT" != x], 
            [xerces_root=$XERCESCROOT; fromwhere="environment";],  
	    [echo ''; 
             echo '************************************************************************************';
	     echo 'Xerces libraries are required. Please set the xerces path using --with-xerces-path=<path>'; 
	     echo 'or using the XERCESCROOT environment variable.';
	     echo '************************************************************************************'; 
             echo ''; exit; 
	   ])
    )	
AS_IF([test "x$xerces_root" == xno],
      [xerces_root=;],
      [])
echo "Xerces-c root        : $xerces_root (from $fromwhere) "



# -------------------------------------------------------------------------------------------
#
#    Optional Settings 
#
# -------------------------------------------------------------------------------------------

xdaq_rpmbuild=;
AC_ARG_ENABLE(xdaq_rpmbuild, 
              [AC_HELP_STRING([--enable-xdaq_rpmbuild],[specify if building against an XDAQ rpm. (default yes)])],  
              [xdaq_rpmbuild=$enableval; fromwhere="cmd-line"; ], 
              [xdaq_rpmbuild=yes; fromwhere="default";]) 
echo "enable XDAQ rpmbuild : $xdaq_rpmbuild (from $fromwhere) "
AS_IF([test "x$xdaq_rpmbuild" == xno], 
      [xdaq_rpmbuild='';],
      [xdaq_rpmbuild=$xdaq_rpmbuild;])


# -------------------------------------------------------------------------------------------
#
#    Now, substitute
#
# -------------------------------------------------------------------------------------------

AC_SUBST(xdaq_root, $xdaq_root)
AC_SUBST(xdaq_base, $xdaq_root)
AC_SUBST(xdaq_os, $xdaq_os)
AC_SUBST(xdaq_platform, $xdaq_platform)
AC_SUBST(xdaq_rpmbuild, $xdaq_rpmbuild)
AC_SUBST(hal_root, $hal_root)
#AC_SUBST(hal_version, `cat $hal_root/thisVersion.txt`)
AC_SUBST(caen_root, $caen_root)
AC_SUBST(xerces_root, $xerces_root)

echo '---------------------------------------------------------';
AC_CONFIG_FILES([Makefile 
		 LasTgBoard/Makefile 
		 LasLsBoard/Makefile])
AC_OUTPUT
echo '---------------------------------------------------------';
echo '';
echo '';
echo '';
echo '';
