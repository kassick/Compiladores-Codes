add_custom_target(ll1_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/SExpr.html ${CMAKE_CURRENT_BINARY_DIR}/SExpr.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/SExpr.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SExpr.html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_executable(sexpr_ll1 sparser_naive.cpp)
add_dependencies(sexpr_ll1 ll1_html)
