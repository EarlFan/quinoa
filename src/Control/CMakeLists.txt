cmake_minimum_required(VERSION 2.8.5)

project(Control CXX)

#### Quinoa control build configuration ########################################

include(charm)

#### Inciter control ###########################################################
add_library(InciterControl
            FileParser.cpp
            StringParser.cpp
            Inciter/InputDeck/Parser.cpp
            Inciter/CmdLine/Parser.cpp)

target_include_directories(InciterControl PUBLIC
                           ${QUINOA_SOURCE_DIR}
                           ${QUINOA_SOURCE_DIR}/Base
                           ${QUINOA_SOURCE_DIR}/Control
                           ${QUINOA_SOURCE_DIR}/Mesh
                           ${PEGTL_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS}
                           ${BRIGAND_INCLUDE_DIRS}
                           ${PROJECT_BINARY_DIR}/../Main)

set_target_properties(InciterControl PROPERTIES
                      LIBRARY_OUTPUT_NAME quinoa_incitercontrol)

INSTALL(TARGETS InciterControl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

#### RNGTest control ###########################################################
project(RNGTestControl CXX)

add_library(RNGTestControl
            FileParser.cpp
            StringParser.cpp
            RNGTest/InputDeck/Parser.cpp
            RNGTest/CmdLine/Parser.cpp)

target_include_directories(RNGTestControl PUBLIC
                           ${QUINOA_SOURCE_DIR}
                           ${QUINOA_SOURCE_DIR}/Base
                           ${QUINOA_SOURCE_DIR}/Control
                           ${PEGTL_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS}
                           ${BRIGAND_INCLUDE_DIRS}
                           ${MKL_INCLUDE_DIRS}
                           ${PROJECT_BINARY_DIR}/../Main)

set_target_properties(RNGTestControl PROPERTIES
                      LIBRARY_OUTPUT_NAME quinoa_rngtestcontrol)

INSTALL(TARGETS RNGTestControl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

#### UnitTest control ##########################################################
project(UnitTestControl CXX)

add_library(UnitTestControl
            StringParser.cpp
            FileParser.cpp
            UnitTest/CmdLine/Parser.cpp)

target_include_directories(UnitTestControl PUBLIC
                           ${QUINOA_SOURCE_DIR}
                           ${QUINOA_SOURCE_DIR}/Base
                           ${QUINOA_SOURCE_DIR}/Control
                           ${PROJECT_BINARY_DIR}/../Main
                           ${PEGTL_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS}
                           ${BRIGAND_INCLUDE_DIRS}
                           ${OMEGA_H_INCLUDE_DIRS})

set_target_properties(UnitTestControl PROPERTIES
                      LIBRARY_OUTPUT_NAME quinoa_unittestcontrol)

INSTALL(TARGETS UnitTestControl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

#### MeshConv control ##########################################################
project(MeshConvControl CXX)

add_library(MeshConvControl
            StringParser.cpp
            MeshConv/CmdLine/Parser.cpp)

target_include_directories(MeshConvControl PUBLIC
                           ${QUINOA_SOURCE_DIR}
                           ${QUINOA_SOURCE_DIR}/Base
                           ${QUINOA_SOURCE_DIR}/IO
                           ${QUINOA_SOURCE_DIR}/Control
                           ${PROJECT_BINARY_DIR}/../Main
                           ${PEGTL_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS}
                           ${BRIGAND_INCLUDE_DIRS})

set_target_properties(MeshConvControl PROPERTIES
                      LIBRARY_OUTPUT_NAME quinoa_meshconvcontrol)

INSTALL(TARGETS MeshConvControl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

#### FileConv control ##########################################################
if (ENABLE_FILECONV)
  project(FileConvControl CXX)

  add_library(FileConvControl
              StringParser.cpp
              FileConv/CmdLine/Parser.cpp)

  target_include_directories(FileConvControl PUBLIC
                             ${QUINOA_SOURCE_DIR}
                             ${QUINOA_SOURCE_DIR}/Base
                             ${QUINOA_SOURCE_DIR}/Control
                             ${PROJECT_BINARY_DIR}/../Main
                             ${PEGTL_INCLUDE_DIRS}
                             ${CHARM_INCLUDE_DIRS}
                             ${BRIGAND_INCLUDE_DIRS})

  set_target_properties(FileConvControl PROPERTIES
                        LIBRARY_OUTPUT_NAME quinoa_fileconvcontrol)

  INSTALL(TARGETS FileConvControl
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
endif()

#### Walker control ############################################################
add_library(WalkerControl
            FileParser.cpp
            StringParser.cpp
            Walker/InputDeck/Parser.cpp
            Walker/CmdLine/Parser.cpp)

target_include_directories(WalkerControl PUBLIC
                           ${QUINOA_SOURCE_DIR}
                           ${QUINOA_SOURCE_DIR}/Base
                           ${QUINOA_SOURCE_DIR}/Control
                           ${PROJECT_BINARY_DIR}/../Main
                           ${PEGTL_INCLUDE_DIRS}
                           ${MKL_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS}
                           ${BRIGAND_INCLUDE_DIRS})

set_target_properties(WalkerControl PROPERTIES
                      LIBRARY_OUTPUT_NAME quinoa_walkercontrol)

INSTALL(TARGETS WalkerControl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
