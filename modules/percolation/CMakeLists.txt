#--------------------------------------------------------------------
# Inviwo Percolation Module
ivw_module(Percolation)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/percolationanalysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/rawpercolationloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scalartransform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/shufflechannel.h
)
#~ ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/percolationanalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/rawpercolationloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scalartransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/shufflechannel.cpp
)
ivw_group("Sources" ${SOURCE_FILES} ${HEADER_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/percolationprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/percolation-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
