#--------------------------------------------------------------------
# Inviwo KTH Tools Module
ivw_module(KxTools)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/amirameshvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filelistproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/iotexthelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/performancetimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/setminmaxdatamap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeinterpolation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeseriesfromslices.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumesourceseries.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumesourceseriesdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumestitching.h
)
#~ ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/amirameshvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filelistproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iotexthelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/performancetimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setminmaxdatamap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeinterpolation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeseriesfromslices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumesourceseries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumesourceseriesdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumestitching.cpp
)

ivw_group("Sources" ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# RunForAll

#~ configure_file(
                #~ ${CMAKE_CURRENT_SOURCE_DIR}/RunForAll.tcl
                #~ ${CMAKE_CURRENT_BINARY_DIR}/RunForAll.tcl
                #~ COPYONLY)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})