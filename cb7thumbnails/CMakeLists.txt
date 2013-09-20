cmake_minimum_required(VERSION 2.8)
project(cb7-thumbnails)

set(cb7thumbnails_SRCS
    cb7creator.cpp)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

kde4_add_plugin(cb7thumbnails ${cb7thumbnails_SRCS})
target_link_libraries(cb7thumbnails ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS})
if (UNIX)
   target_link_libraries(cb7thumbnails ${KDE4_KPTY_LIBRARY})
endif (UNIX)

install(TARGETS cb7thumbnails DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES cb7thumbnails.desktop DESTINATION ${SERVICES_INSTALL_DIR})