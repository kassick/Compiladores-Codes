add_custom_target(naive_tokens_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/SExpr.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SExpr.html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/index.html PROPERTIES GENERATED TRUE)

SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SExprL.g4)
SET(GENERATED_PATH2 ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package sexpr_parser)

AddAntlrTarget(sparser_naive_tokens ${GRAMMAR_FILE} "" ${GENERATED_PATH2})
message("Naive tokens generated in ${GENERATED_PATH2}")
#message(FATAL "Dependencias !!!!! "${sparser_naive_tokens_GENERATED_SRC})

include_directories(${GENERATED_PATH2})

add_executable(sparser_naive_tokens
  sparser_naive_tokens.cpp
  ${sparser_naive_tokens_GENERATED_SRC})
add_dependencies(sparser_naive_tokens
  naive_tokens_html
  generate_sparser_naive_tokens)
target_link_libraries(sparser_naive_tokens antlr4static)
