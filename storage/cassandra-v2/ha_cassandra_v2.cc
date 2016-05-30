#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation        // gcc: Class implementation
#endif

#include <my_config.h>
#include <mysql/plugin.h>
#include "ha_cassandra_v2.h"
#include "sql_class.h"


static handler *cassandra_create_handler(handlerton *hton,
                                       TABLE_SHARE *table,
                                       MEM_ROOT *mem_root);

handlerton *cassandra_hton;


/**
  Structure for CREATE TABLE options (table options).
  It needs to be called ha_table_option_struct.

  The option values can be specified in the CREATE TABLE at the end:
  CREATE TABLE ( ... ) *here*
*/

struct ha_table_option_struct
{
  const char *cass_host;
  int         cass_port;
  const char *keyspace;
  const char *column_family;
};


ha_create_table_option cassandra_table_option_list[]=
{
  /*
    one option that takes an arbitrary string
  */
  HA_TOPTION_STRING("cass_host", cass_host),
  HA_TOPTION_NUMBER("cass_port", cass_port, 9042, 1, 65535, 0),
  HA_TOPTION_STRING("keyspace", keyspace),
  HA_TOPTION_STRING("column_family", column_family),
  HA_TOPTION_END
};
