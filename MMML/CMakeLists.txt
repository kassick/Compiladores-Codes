SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MMML.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package mmml)

AddAntlrTarget(mmml_parser ${GRAMMAR_FILE} "" ${GENERATED_PATH})
AddAntlrJavaTarget(mmml_java_parser mmml ${GRAMMAR_FILE} ${GENERATED_PATH} "")

add_custom_target(mmmlc_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/mmml.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/mmml.html
  DEPENDS generate_jar_mmml_java_parser
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${GENERATED_PATH})
include_directories(include/)

add_executable(mmmlc
  mmmlc.cpp
  Symbol.cpp
  TypeRegistry.cpp
  basic_types.cpp
  NestedSymbolTable.cpp
  CodeVisitor.cpp
  TypedArgListVisitor.cpp
  InstructionBlock.cpp
  mmml_error.cpp
  ${mmml_parser_GENERATED_SRC})
add_dependencies(mmmlc mmmlc_html generate_mmml_parser)
target_link_libraries(mmmlc antlr4static)

add_subdirectory(tests/)
