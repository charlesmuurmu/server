# use the first path that has cassandra.h included, if found

FIND_PATH(Datastax_INCLUDE_DIRS cassandra.h PATHS
$ENV{DATASTAX_INCLUDE}      # environment variable to be used optionally
${Datastax_INCLUDE_DIR}     # this may be set
/usr/local/include/# list of additional directories to look from
/opt/local/include
/usr/include
/opt/include
)

# Verify that cassandra linking library is found
SET(Datastax_LIB_PATHS ${Datastax_LIB_PATHS} /usr/local/lib /opt/local/lib /opt/lib)
FIND_LIBRARY(Datastax_LIBS NAMES cassandra PATHS ${Datastax_LIB_PATHS} ${Datastax_LIB})

IF(EXISTS ${Datastax_LIBS})
  GET_FILENAME_COMPONENT(LINK_DIR ${Datastax_LIBS} PATH ABSOLUTE)
  INCLUDE_DIRECTORIES(AFTER ${Datastax_INCLUDE_DIRS}/..)
  SET(CMAKE_REQUIRED_INCLUDES ${Datastax_INCLUDE_DIRS})

  STRING(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  STRING(REPLACE "-fno-implicit-templates" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  SET(CMAKE_REQUIRED_INCLUDES "${Datastax_INCLUDE_DIRS}/..")
  CHECK_CXX_SOURCE_COMPILES(
"
#include <cassandra.h>
int main() {
  
  return 0;
}
" CASSANDRASE_OK)
ENDIF()

ENDIF()


SET(cassandra_sources
    ha_cassandra_v2.cc
    ha_cassandra_v2.h)

LINK_DIRECTORIES(${LINK_DIR})

IF(CASSANDRASE_OK)
  SET(CASSANDRA_DEB_FILES "usr/lib/mysql/plugin/ha_cassandra.so" PARENT_SCOPE)
  MYSQL_ADD_PLUGIN(cassandra ${cassandra_sources} STORAGE_ENGINE
    MODULE_ONLY LINK_LIBRARIES cassandra COMPONENT cassandra-engine)
ENDIF(CASSANDRASE_OK)


