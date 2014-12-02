# Builds the SWFExtractor.class file
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/SWFExtractor.class
                   COMMAND javac -d ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/SWFExtractor.java)
add_custom_target(SWFExtractor.class
                  ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SWFExtractor.class)
