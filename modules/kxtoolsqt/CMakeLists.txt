#--------------------------------------------------------------------
# KxTools Qt Widgets
ivw_module(KxToolsQt)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/filelistpropertywidgetqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/kxtoolsqtmoduledefine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kxtoolsqtmodule.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/filelistpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kxtoolsqtmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-kxtoolsqt PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
ivw_make_package(InviwoKxToolsQtModule inviwo-module-kxtoolsqt)
ivw_qt_add_to_install(kxtoolsqt Qt5Core Qt5Gui Qt5Widgets)
