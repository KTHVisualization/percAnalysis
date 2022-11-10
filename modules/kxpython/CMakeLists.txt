#--------------------------------------------------------------------
# Inviwo KTH Python Module
ivw_module(KxPython)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    curvefitting.h
    gaussianrandomfieldgenerator.h
    netcdfsource.h
    percolationevolutionplot.h
    percolationplot.h
)
#~ ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    curvefitting.cpp
    gaussianrandomfieldgenerator.cpp
    netcdfsource.cpp
    percolationevolutionplot.cpp
    percolationplot.cpp
)

ivw_group("Sources" ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
