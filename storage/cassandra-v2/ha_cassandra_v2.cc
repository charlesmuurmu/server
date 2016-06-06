#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation        // gcc: Class implementation
#endif

#include <my_config.h>
#include <mysql/plugin.h>
#include <cassandra.h>
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


int ha_cassandra_v2::close(void)
{
 /* Still be be implemented*/
  return HA_ERR_INTERNAL_ERROR; 
}

int ha_cassandra_v2::open(const char *name, int mode, uint test_if_locked)
{
  /* Still be be implemented*/
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::create(const char *name, TABLE *table_arg,
                         HA_CREATE_INFO *create_info)
{
  /* Still be be implemented*/
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::write_row(uchar *buf){
  /* Still be be implemented*/
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::update_row(const uchar *old_data, uchar *new_data){
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::delete_row(const uchar *buf){
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::rnd_init(bool scan){
  return HA_ERR_INTERNAL_ERROR;
} 

int ha_cassandra_v2::rnd_end(){
  return HA_ERR_INTERNAL_ERROR;
}

int ha_cassandra_v2::rnd_next(uchar *buf){
  return HA_ERR_INTERNAL_ERROR;
} 

int ha_cassandra_v2::rnd_pos(uchar *buf, uchar *pos){
  return HA_ERR_INTERNAL_ERROR;
}

void ha_cassandra_v2::position(const uchar *record){
  
} 

int ha_cassandra_v2::info(uint){
  return HA_ERR_INTERNAL_ERROR;
} 

int ha_cassandra_v2::delete_all_rows(void){
  return HA_ERR_INTERNAL_ERROR;
}

THR_LOCK_DATA **ha_cassandra_v2::store_lock(THD *thd, THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type)
  {
    return NULL;
  } 

handler *cassandra_create_handler(handlerton *hton, TABLE_SHARE *table, MEM_ROOT *mem_root)
  {
    return NULL;
  }                     
