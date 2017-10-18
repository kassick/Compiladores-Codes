SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MMML.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package mmml)

AddAntlrTarget(mmml_parser ${GRAMMAR_FILE} "" ${GENERATED_PATH})
AddAntlrJavaTarget(mmml_java_parser mmml ${GRAMMAR_FILE} ${GENERATED_PATH} "")

add_custom_target(mmmlc_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/mmml.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/inputs || true
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/inputs/* ${CMAKE_CURRENT_BINARY_DIR}/inputs/
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/examples || true
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/examples/* ${CMAKE_CURRENT_BINARY_DIR}/examples/
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/errors || true
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/errors/* ${CMAKE_CURRENT_BINARY_DIR}/errors/
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
  NestedSymbolTable.cpp
  TypeRegistry.cpp
  basic_types.cpp
  InstructionBlock.cpp
  ToplevelVisitor.cpp
  TypedArgListVisitor.cpp
  FunctionCallArgListVisitor.cpp
  FuncbodyVisitor.cpp
  MetaExprVisitor.cpp
  mmml_error.cpp
  casts.cpp
  ${mmml_parser_GENERATED_SRC})
add_dependencies(mmmlc mmmlc_html generate_mmml_parser)
target_link_libraries(mmmlc antlr4static docopt)

add_subdirectory(tests/)

# ZIP the HTML and all the playground only if we are compiling in emscripten
if(${CMAKE_CXX_COMPILER} MATCHES "em\\+\\+")
add_custom_target(mmml.zip
  ${CMAKE_COMMAND} -E tar "cfv" "mmml.zip" --format=zip
  ${CMAKE_CURRENT_BINARY_DIR}/index.html
  ${CMAKE_CURRENT_BINARY_DIR}/mmmlc.js
  ${CMAKE_CURRENT_BINARY_DIR}/examples/*.mmml
  ${CMAKE_CURRENT_BINARY_DIR}/inputs/*
  ${CMAKE_CURRENT_BINARY_DIR}/errors/*.mmml
  ${CMAKE_CURRENT_SOURCE_DIR}/MMML.g4
  ${CMAKE_CURRENT_SOURCE_DIR}/NestedSymbolTable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/mmml/NestedSymbolTable.H
  ${CMAKE_CURRENT_SOURCE_DIR}/old/NestedSymbolTable.cs
  )

add_dependencies(mmml.zip mmmlc mmmlc_html)
endif()
