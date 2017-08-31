add_custom_target(antlr_listener_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/SExpr.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SExpr.html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SExpr.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package sexpr_parser)

AddAntlrTarget(sparser_antlr_listener ${GRAMMAR_FILE} "" ${GENERATED_PATH})

include_directories(${GENERATED_PATH})
add_executable(sparser_listener sparser_antlr_listener.cpp ${sparser_antlr_listener_GENERATED_SRC})
add_dependencies(sparser_listener antlr_html generate_sparser_antlr_listener)
target_link_libraries(sparser_listener antlr4static)
