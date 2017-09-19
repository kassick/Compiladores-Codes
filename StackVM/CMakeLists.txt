# add_custom_target(StackVM_html
#   ALL
#   COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/StackVM_html
#   BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
#   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/StackVM_html
#   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#   )

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
  #antlr_html
  generate_stackvm_antlr
  )
target_link_libraries(stackvm antlr4static)
