 add_custom_target(StackVM_html
   ALL
   COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/StackVM.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
   COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/tests || true
   COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/tests/* ${CMAKE_CURRENT_BINARY_DIR}/tests
   BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/StackVM.html
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )

SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/StackVM.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package StackVM)

AddAntlrTarget(stackvm_antlr ${GRAMMAR_FILE} "" ${GENERATED_PATH})

include_directories(${GENERATED_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(stackvm
  stackvm.cpp
  ${stackvm_antlr_GENERATED_SRC}
  )
add_dependencies(stackvm
  StackVM_html
  generate_stackvm_antlr
  )
target_link_libraries(stackvm antlr4static docopt)

# ZIP the HTML and all the playground only if we are compiling in emscripten
if(${CMAKE_CXX_COMPILER} MATCHES "em\\+\\+")
add_custom_target(stackvm.zip
  ${CMAKE_COMMAND} -E tar "cfv" "stackvm.zip" --format=zip
  ${CMAKE_CURRENT_BINARY_DIR}/index.html
  ${CMAKE_CURRENT_BINARY_DIR}/stackvm.js
  ${CMAKE_CURRENT_BINARY_DIR}/tests/*
  ${CMAKE_SOURCE_DIR}/CMakeLists.txt
  ${CMAKE_SOURCE_DIR}/deps/
  ${CMAKE_SOURCE_DIR}/StackVM
  )

add_dependencies(stackvm.zip stackvm StackVM_html)

endif()
