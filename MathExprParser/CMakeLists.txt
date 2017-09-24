 # add_custom_target(MathExpr_html
 #   ALL
 #   COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/MathExpr.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
 #   BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
 #   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/MathExpr.html
 #   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 #   )

SET(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MathExpr.g4)
SET(GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated)
SET(ANTLR_EXTRA_OPTS  -package MathExpr)

AddAntlrTarget(mathexpr_antlr ${GRAMMAR_FILE} "" ${GENERATED_PATH})

include_directories(${GENERATED_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(mathexpr
  mathexpr.cpp
  ${mathexpr_antlr_GENERATED_SRC}
  )
add_dependencies(mathexpr
  #MathExpr_html
  generate_mathexpr_antlr
  )
target_link_libraries(mathexpr antlr4static)
