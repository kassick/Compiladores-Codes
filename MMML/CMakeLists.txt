add_custom_target(mmmlc_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/mmml.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/mmml.html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MMML.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package mmml)

AddAntlrTarget(mmml_parser ${GRAMMAR_FILE} "" ${GENERATED_PATH})

include_directories(${GENERATED_PATH})
add_executable(mmmlc mmmlc.cpp ${mmml_parser_GENERATED_SRC})
add_dependencies(mmmlc mmmlc_html generate_mmml_parser)
target_link_libraries(mmmlc antlr4static)
