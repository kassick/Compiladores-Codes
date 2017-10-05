add_executable(test_nested_table
  test_nested_table.cpp
  ../NestedSymbolTable.cpp
  ../Symbol.cpp
  )

add_executable(test_types
  test_types.cpp
  ../TypeRegistry.cpp
  )

add_executable(test_types_with_registry
  test_types_with_registry.cpp
  ../TypeRegistry.cpp
  )

add_executable(test_sequence_types
  test_sequence_types.cpp
  ../TypeRegistry.cpp
  )

add_executable(test_antlr_any
  test_antlr_any.cpp)
