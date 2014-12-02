# This file builds the SWFExtractor.class file

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/SWFExtractor.class
                   COMMAND javac -d ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/SWFExtractor.java)
add_custom_target(SWFExtractor.class
                  ALL DEPENDS ${PROJECT_BINARY_DIR}/SWFExtractor.class)
