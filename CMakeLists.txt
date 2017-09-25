cmake_minimum_required(VERSION 3.3)
# find_package( Boost 1.53 COMPONENTS
    # system
    # program_options
    # system
    # filesystem)

include(UseJava)

set(CMAKE_CXX_FLAGS "-std=c++11")
if(${CMAKE_CXX_COMPILER} MATCHES "em\\+\\+")
  set(CMAKE_EXE_LINKER_FLAGS "-s EXPORTED_FUNCTIONS=\"['_parse_string_c']\"")
endif()

############################################################
# External Dependencies
# Compile them from their tarballs before compiling tinyFS
include(ExternalProject)

#############
# # Docopt C++
# ExternalProject_Add(ex_docopt
    # #GIT_REPOSITORY https://github.com/docopt/docopt.cpp.git
    # URL ${CMAKE_CURRENT_LIST_DIR}/deps/docopt.cpp-0.6.2.zip
    # PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/docopt
    # BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/docopt/build
    # INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/docopt/install
    # CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/docopt/install
# )
# ExternalProject_Get_Property(ex_docopt install_dir)
# message(STATUS Docopt compiled into ${install_dir})
# SET(docopt_install_dir ${install_dir})
# include_directories(${docopt_install_dir}/include/)

add_library(docopt UNKNOWN IMPORTED)
set_property(TARGET docopt PROPERTY IMPORTED_LOCATION
    ${docopt_install_dir}/lib/libdocopt.so)


###################################################
# Antlr4 cpp runtime
SET(ANTLR_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/antlrruntime)
SET(antlr_install_dir ${ANTLR_PREFIX}/install)
ExternalProject_Add(antlrruntime
    URL ${CMAKE_CURRENT_LIST_DIR}/deps/antlr4-cpp-runtime-4.7-source.zip
    PREFIX ${ANTLR_PREFIX}
    BINARY_DIR ${ANTLR_PREFIX}/build
    INSTALL_DIR ${ANTLR_PREFIX}/install
    CMAKE_ARGS -DCMAKE_CXX_FLAGS=-std=c++11 -DCMAKE_INSTALL_PREFIX=${ANTLR_PREFIX}/install -DWITH_DEMO:BOOL=FALSE
    PATCH_COMMAND cat ${CMAKE_CURRENT_LIST_DIR}/deps/antlr-emscripten.patch | patch -p1
)

message(STATUS Antlr-Runtime (Cpp) compiled into ${ANTLR_PREFIX}/install)
include_directories(
  ${antlr_install_dir}/include/antlr4-runtime
  )

add_library(antlr4static STATIC IMPORTED)
set_property(TARGET antlr4static PROPERTY IMPORTED_LOCATION
  ${antlr_install_dir}/lib/libantlr4-runtime.a)

SET(ANTLR_JAR_LOCATION ${CMAKE_CURRENT_LIST_DIR}/deps/antlr-4.7-complete.jar)

find_package(Java COMPONENTS Runtime REQUIRED)

function(AddAntlrTarget target parser_g4 lexer_g4 path)
 get_filename_component(g4base ${parser_g4} NAME_WE)
 message(INFO "g4 base is ${g4base}")
 set(${target}_GENERATED_SRC
   ${path}/${g4base}BaseListener.cpp
   ${path}/${g4base}Listener.cpp
   ${path}/${g4base}BaseVisitor.cpp
   ${path}/${g4base}Visitor.cpp
   ${path}/${g4base}Lexer.cpp
   ${path}/${g4base}Parser.cpp
   PARENT_SCOPE
   )
 set(the_sources
   ${path}/${g4base}BaseListener.cpp
   ${path}/${g4base}Listener.cpp
   ${path}/${g4base}BaseVisitor.cpp
   ${path}/${g4base}Visitor.cpp
   ${path}/${g4base}Lexer.cpp
   ${path}/${g4base}Parser.cpp
   )
 message("Fontes do Antlr ${the_sources}")
 add_custom_command(
   OUTPUT ${the_sources}
   COMMAND ${CMAKE_COMMAND} -E make_directory ${path}
   COMMAND "${Java_JAVA_EXECUTABLE}" -jar ${ANTLR_JAR_LOCATION} -Werror -Dlanguage=Cpp -listener -visitor ${ANTLR_EXTRA_OPTS} -o ${path} ${lexer_g4} ${parser_g4}
   COMMAND echo "COMANDO: ${Java_JAVA_EXECUTABLE}" -jar ${ANTLR_JAR_LOCATION} -Werror -Dlanguage=Cpp -listener -visitor ${ANTLR_EXTRA_OPTS} -o ${path} ${lexer_g4} ${parser_g4}
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   DEPENDS ${parser_g4} ${lexer_g4}
   )
 foreach( src_file ${the_sources} )
   set_source_files_properties(
     ${src_file}
     PROPERTIES GENERATED TRUE
     )
 endforeach()
 add_custom_target(generate_${target} DEPENDS ${the_sources})
endfunction()

function(AddAntlrJavaTarget target jar_name parser_g4 path extra_opts)
 get_filename_component(g4base ${parser_g4} NAME_WE)
 message(INFO "g4 base is ${g4base}")
 set(${target}_GENERATED_SRC
   ${path}/${g4base}BaseListener.java
   ${path}/${g4base}Listener.java
   ${path}/${g4base}BaseVisitor.java
   ${path}/${g4base}Visitor.java
   ${path}/${g4base}Lexer.java
   ${path}/${g4base}Parser.java
   PARENT_SCOPE
   )
 set(the_sources
   ${path}/${g4base}BaseListener.java
   ${path}/${g4base}Listener.java
   ${path}/${g4base}BaseVisitor.java
   ${path}/${g4base}Visitor.java
   ${path}/${g4base}Lexer.java
   ${path}/${g4base}Parser.java
   )
 message("Fontes do Antlr ${the_sources}")
 add_custom_command(
   OUTPUT ${the_sources}
   COMMAND ${CMAKE_COMMAND} -E make_directory ${path}
   COMMAND "${Java_JAVA_EXECUTABLE}" -jar ${ANTLR_JAR_LOCATION} -Werror -Dlanguage=Java -listener -visitor ${extra_opts} -o ${path} ${lexer_g4} ${parser_g4}
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   DEPENDS ${parser_g4} ${lexer_g4}
   )
 foreach( src_file ${the_sources} )
   message(INFO "Source ${src_file} is generated")
   set_source_files_properties(
     ${src_file}
     PROPERTIES GENERATED TRUE
     )
 endforeach()
 add_custom_target(generate_${target} DEPENDS ${the_sources})
 add_jar(generate_jar_${target}
        SOURCES ${the_sources}
        INCLUDE_JARS ${ANTLR_JAR_LOCATION}
        OUTPUT_NAME ${jar_name}
        )
endfunction()

function(AntlrGenerateParseTreePng grammar_jar grammar_name start_rule input output)
  add_custom_command(
    OUTPUT ${output}
    COMMAND "${Java_JAVA_EXECUTABLE}" -cp "${ANTLR_JAR_LOCATION}:${grammar_jar}" org.antlr.v4.gui.TestRig ${grammar_name} ${start_rule} -ps ${output}
    )
endfunction()

add_subdirectory(./SExpr/Antlr/)
add_subdirectory(./SExpr/AntlrListener/)
add_subdirectory(./SExpr/AntlrVisitor/)
add_subdirectory(./SExpr/LL1/)
add_subdirectory(./SExpr/Naive/)
add_subdirectory(./SExpr/Naive-Tokens/)
add_subdirectory(./MMML)
add_subdirectory(./StackVM)
add_subdirectory(./MathExprParser)
