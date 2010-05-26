cmake_minimum_required(VERSION 2.8)

IF(CMAKE_BUILD_TYPE MATCHES "Release")
SET(PROGRAM_NAME mhmake)
ELSE(CMAKE_BUILD_TYPE MATCHES "Release")
SET(PROGRAM_NAME mhmake_dbg)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")

PROJECT(${PROGRAM_NAME})

SET( CMAKE_CXX_FLAGS_DEBUG "-D _DEBUG")

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mhmakelexer.cpp ${CMAKE_CURRENT_BINARY_DIR}/mhmakelexer.h
   COMMAND flex++ -8 -S${CMAKE_CURRENT_SOURCE_DIR}/src/flexskel.cc -H${CMAKE_CURRENT_SOURCE_DIR}/src/flexskel.h -h${CMAKE_CURRENT_BINARY_DIR}/mhmakelexer.h -otemp1234.456 ${CMAKE_CURRENT_SOURCE_DIR}/src/mhmakelexer.l
   COMMAND echo '\#include \"stdafx.h\"' > ${CMAKE_CURRENT_BINARY_DIR}/mhmakelexer.cpp
   COMMAND cat temp1234.456 >> ${CMAKE_CURRENT_BINARY_DIR}/mhmakelexer.cpp
   COMMAND rm temp1234.456

   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/mhmakelexer.l ${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.h
)
ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.cpp ${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.h
   COMMAND bison++ -d -S${CMAKE_CURRENT_SOURCE_DIR}/src/bison.cc -H${CMAKE_CURRENT_SOURCE_DIR}/src/bison.h -h${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.h -otemp1234.456 ${CMAKE_CURRENT_SOURCE_DIR}/src/mhmakeparser.y
   COMMAND echo '\#include \"stdafx.h\"' > ${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.cpp
   COMMAND cat temp1234.456 >> ${CMAKE_CURRENT_BINARY_DIR}/mhmakeparser.cpp
   COMMAND rm temp1234.456

   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/mhmakeparser.y
)
INCLUDE_DIRECTORIES(src ${CMAKE_CURRENT_BINARY_DIR})
LINK_LIBRARIES(/usr/lib/libpopt.a)

ADD_EXECUTABLE(${PROGRAM_NAME}
               mhmakeparser.cpp
               mhmakelexer.cpp
               src/mhmake.cpp
               src/mhmakefileparser.cpp
               src/util.cpp
               src/functions.cpp
               src/fileinfo.cpp
               src/rule.cpp
               src/md5.cpp
               src/build.cpp
               src/curdir.cpp
               src/commandqueue.cpp
              )

INSTALL_TARGETS( /bin ${PROGRAM_NAME} )
