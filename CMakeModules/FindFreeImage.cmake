#.rst:
# FindFreeImage
# -------------
#
# Find the FreeImage Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``FreeImage::FreeImage``
#   The ``FreeImage`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FREEIMAGE_INCLUDE_DIRS``
#   where to find FreeImage headers.
# ``FREEIMAGE_LIBRARIES``
#   the libraries to link against to use FreeImage.
# ``FREEIMAGE_FOUND``
#   true if the FreeImage headers and library were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_FREEIMAGE QUIET freeimage)

# Look for the header file.
find_path(FREEIMAGE_INCLUDE_DIR
	NAMES FreeImage.h
	HINTS ${PC_FREEIMAGE_INCLUDE_DIRS})

# Look for the library.
# Allow FREEIMAGE_LIBRARY to be set manually, as the location of the FreeImage library
if(NOT FREEIMAGE_LIBRARY)
	find_library(FREEIMAGE_LIBRARY
		NAMES freeimage
		HINTS ${PC_FREEIMAGE_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage
	REQUIRED_VARS FREEIMAGE_LIBRARY FREEIMAGE_INCLUDE_DIR)

if(FREEIMAGE_FOUND)
	set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_INCLUDE_DIR})

	if(NOT FREEIMAGE_LIBRARIES)
		set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
	endif()

	if(NOT TARGET FreeImage::FreeImage)
		add_library(FreeImage::FreeImage UNKNOWN IMPORTED)
		set_target_properties(FreeImage::FreeImage PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FREEIMAGE_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${FREEIMAGE_LIBRARY}")
	endif()
endif()

mark_as_advanced(FREEIMAGE_INCLUDE_DIR FREEIMAGE_LIBRARY)
