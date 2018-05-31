cmake_minimum_required(VERSION 3.7)

project(nro-thumbnailer VERSION 0.0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

add_executable(nro-thumbnailer
	src/CLI11.hpp
	src/switch_structs.h
	src/switch_border.h
	src/main.cpp)

target_compile_features(nro-thumbnailer PUBLIC cxx_std_11)

find_package(FreeImage REQUIRED)
target_link_libraries(nro-thumbnailer FreeImage::FreeImage)

include(GNUInstallDirs)
install(TARGETS nro-thumbnailer
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES integration/nro.thumbnailer
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/thumbnailers)
install(FILES integration/switch-executable-nro.xml
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages)
