# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


SET(CPACK_BINARY_7Z "")
SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_FREEBSD "")
SET(CPACK_BINARY_IFW "")
SET(CPACK_BINARY_NSIS "")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_PRODUCTBUILD "")
SET(CPACK_BINARY_RPM "")
SET(CPACK_BINARY_STGZ "")
SET(CPACK_BINARY_TBZ2 "")
SET(CPACK_BINARY_TGZ "")
SET(CPACK_BINARY_TXZ "")
SET(CPACK_BINARY_TZ "")
SET(CPACK_BINARY_WIX "")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_BUILD_SOURCE_DIRS "/home/sa/MotionPlanningExplorerGUI/libs/ompl;/home/sa/MotionPlanningExplorerGUI/libs/ompl/build")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_COMPONENTS_ALL "ompl;python;morse")
SET(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
SET(CPACK_COMPONENT_MORSE_DEPENDS "python")
SET(CPACK_COMPONENT_MORSE_DESCRIPTION "The Blender/MORSE plugin allows one to plan paths using the MORSE robot simulator. MORSE is built on top of Blender and uses its built-in physics engine to compute physically realistic motions.")
SET(CPACK_COMPONENT_MORSE_DISPLAY_NAME "Blender/MORSE plugin")
SET(CPACK_COMPONENT_OMPL_DISPLAY_NAME "OMPL library, headers, and demos")
SET(CPACK_COMPONENT_OMPL_REQUIRED "ON")
SET(CPACK_COMPONENT_PYTHON_DEPENDS "ompl")
SET(CPACK_COMPONENT_PYTHON_DISPLAY_NAME "Python bindings")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "python2.7, libboost-serialization-dev, libboost-filesystem-dev, libboost-system-dev, libboost-program-options-dev, libboost-test-dev, libode-dev, libtriangle-dev")
SET(CPACK_GENERATOR "TGZ;ZIP")
SET(CPACK_IGNORE_FILES "/.hg;/.vscode;/build/;.pyc$;.pyo$;__pycache__;.so$;.dylib$;.orig$;.log$;.DS_Store;/html/;/bindings/;TODO;.registered$;binding_generator.py$")
SET(CPACK_INSTALLED_DIRECTORIES "/home/sa/MotionPlanningExplorerGUI/libs/ompl;/")
SET(CPACK_INSTALL_CMAKE_PROJECTS "")
SET(CPACK_INSTALL_PREFIX "/usr/local")
SET(CPACK_MODULE_PATH ";/cmake/Modules;/home/sa/MotionPlanningExplorerGUI/libs/ompl/CMakeModules")
SET(CPACK_NSIS_DISPLAY_NAME "ompl 0.1.1")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "ompl 0.1.1")
SET(CPACK_OUTPUT_CONFIG_FILE "/home/sa/MotionPlanningExplorerGUI/libs/ompl/build/CPackConfig.cmake")
SET(CPACK_PACKAGE_CONTACT "Mark Moll <mmoll@rice.edu>")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-3.10/Templates/CPack.GenericDescription.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Open Motion Planning Library (OMPL)")
SET(CPACK_PACKAGE_FILE_NAME "ompl-0.1.1-Source")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "ompl 0.1.1")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ompl 0.1.1")
SET(CPACK_PACKAGE_NAME "ompl")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "Rice University")
SET(CPACK_PACKAGE_VERSION "0.1.1")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_RESOURCE_FILE_LICENSE "/usr/share/cmake-3.10/Templates/CPack.GenericLicense.txt")
SET(CPACK_RESOURCE_FILE_README "/usr/share/cmake-3.10/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-3.10/Templates/CPack.GenericWelcome.txt")
SET(CPACK_RPM_PACKAGE_SOURCES "ON")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_7Z "")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
SET(CPACK_SOURCE_IGNORE_FILES "/.hg;/.vscode;/build/;.pyc$;.pyo$;__pycache__;.so$;.dylib$;.orig$;.log$;.DS_Store;/html/;/bindings/;TODO;.registered$;binding_generator.py$")
SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "/home/sa/MotionPlanningExplorerGUI/libs/ompl;/")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/sa/MotionPlanningExplorerGUI/libs/ompl/build/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "ompl-0.1.1-Source")
SET(CPACK_SOURCE_RPM "")
SET(CPACK_SOURCE_TBZ2 "")
SET(CPACK_SOURCE_TGZ "")
SET(CPACK_SOURCE_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_SOURCE_TXZ "")
SET(CPACK_SOURCE_TZ "")
SET(CPACK_SOURCE_ZIP "")
SET(CPACK_STRIP_FILES "")
SET(CPACK_SYSTEM_NAME "Linux")
SET(CPACK_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/home/sa/MotionPlanningExplorerGUI/libs/ompl/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
