add_custom_target(naive_html
  ALL
  COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/SExpr.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SExpr.html
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/index.html PROPERTIES GENERATED TRUE)

add_executable(sparser_naive sparser_naive.cpp)
add_dependencies(sparser_naive naive_html)
