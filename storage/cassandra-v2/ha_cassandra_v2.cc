#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation        // gcc: Class implementation
#endif

#include <my_config.h>
#include <mysql/plugin.h>
#include "ha_cassandra_v2.h"
#include "sql_class.h"


static handler *cassandra_v2_create_handler(handlerton *hton,
                                       TABLE_SHARE *table, 
                                       MEM_ROOT *mem_root);

handlerton *cassandra_v2_hton;

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

/* Interface to mysqld, to check system tables supported by SE */
static const char* cassandra_v2_system_database();
static bool cassandra_v2_is_supported_system_table(const char *db,
                                      const char *table_name,
                                      bool is_sql_layer_system_table);
#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key ex_key_mutex_cassandra_v2_share_mutex;

static PSI_mutex_info all_cassandra_v2_mutexes[]=
{
  { &ex_key_mutex_cassandra_v2_share_mutex, "cassandra_v2_share::mutex", 0}
};

static void init_cassandra_v2_psi_keys()
{
  const char* category= "cassandra_v2";
  int count;

  count= array_elements(all_cassandra_v2_mutexes);
  mysql_mutex_register(category, all_cassandra_v2_mutexes, count);
}
#endif

cassandra_v2_share::cassandra_v2_share()
{
  thr_lock_init(&lock);
  mysql_mutex_init(ex_key_mutex_cassandra_v2_share_mutex,
                   &mutex, MY_MUTEX_INIT_FAST);
  CassFuture* connect_future = NULL;
  CassCluster* cluster = cass_cluster_new();
  CassSession* session = cass_session_new();
}


static int cassandra_v2_init_func(void *p)
{
  DBUG_ENTER("cassandra_v2_init_func");

#ifdef HAVE_PSI_INTERFACE
  init_cassandra_v2_psi_keys();
#endif

  cassandra_v2_hton= (handlerton *)p;
  cassandra_v2_hton->state=                     SHOW_OPTION_YES;
  cassandra_v2_hton->create=                    cassandra_v2_create_handler;
  cassandra_v2_hton->flags=                     HTON_CAN_RECREATE;
  /*cassandra_v2_hton->system_database=   cassandra_v2_system_database;
  cassandra_v2_hton->is_supported_system_table= cassandra_v2_is_supported_system_table;
*/
  DBUG_RETURN(0);
}


cassandra_v2_share *ha_cassandra_v2::get_share()
{
  cassandra_v2_share *tmp_share;

  DBUG_ENTER("ha_cassandra_v2::get_share()");

  lock_shared_ha_data();
  if (!(tmp_share= static_cast<cassandra_v2_share*>(get_ha_share_ptr())))
  {
    tmp_share= new cassandra_v2_share;
    if (!tmp_share)
      goto err;

    set_ha_share_ptr(static_cast<Handler_share*>(tmp_share));
  }
err:
  unlock_shared_ha_data();
  DBUG_RETURN(tmp_share);
}


static handler* cassandra_v2_create_handler(handlerton *hton,
                                       TABLE_SHARE *table, 
                                       MEM_ROOT *mem_root)
{
  return new (mem_root) ha_cassandra_v2(hton, table);
}

ha_cassandra_v2::ha_cassandra_v2(handlerton *hton, TABLE_SHARE *table_arg)
  :handler(hton, table_arg)
{}


static const char *ha_cassandra_v2_exts[] = {
  NullS
};

const char **ha_cassandra_v2::bas_ext() const
{
  return ha_cassandra_v2_exts;
}

const char* ha_cassandra_v2_system_database= NULL;
const char* cassandra_v2_system_database()
{
  return ha_cassandra_v2_system_database;
}


static st_system_tablename ha_cassandra_v2_system_tables[]= {
  {(const char*)NULL, (const char*)NULL}
};


static bool cassandra_v2_is_supported_system_table(const char *db,
                                              const char *table_name,
                                              bool is_sql_layer_system_table)
{
  st_system_tablename *systab;

  // Does this SE support "ALL" SQL layer system tables ?
  if (is_sql_layer_system_table)
    return false;

  // Check if this is SE layer system tables
  systab= ha_cassandra_v2_system_tables;
  while (systab && systab->db)
  {
    if (systab->db == db &&
        strcmp(systab->tablename, table_name) == 0)
      return true;
    systab++;
  }

  return false;
}

int ha_cassandra_v2::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_cassandra_v2::open");

  if (!(share = get_share()))
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock,&lock,NULL);

  DBUG_RETURN(0);
}



int ha_cassandra_v2::close(void)
{
  DBUG_ENTER("ha_cassandra_v2::close");
  DBUG_RETURN(0);
}




int ha_cassandra_v2::write_row(uchar *buf)
{
  DBUG_ENTER("ha_cassandra_v2::write_row");
  DBUG_RETURN(0);
}


int ha_cassandra_v2::update_row(const uchar *old_data, uchar *new_data)
{

  DBUG_ENTER("ha_cassandra_v2::update_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}




int ha_cassandra_v2::delete_row(const uchar *buf)
{
  DBUG_ENTER("ha_cassandra_v2::delete_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


/**
  @brief
  Positions an index cursor to the index specified in the handle. Fetches the
  row if available. If the key value is null, begin at the first key of the
  index.
*/

int ha_cassandra_v2::index_read_map(uchar *buf, const uchar *key,
                               key_part_map keypart_map __attribute__((unused)),
                               enum ha_rkey_function find_flag
                               __attribute__((unused)))
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::index_read");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


/**
  @brief
  Used to read forward through the index.
*/

int ha_cassandra_v2::index_next(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::index_next");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


/**
  @brief
  Used to read backwards through the index.
*/

int ha_cassandra_v2::index_prev(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::index_prev");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


/**
  @brief
  index_first() asks for the first key in the index.
  @details
  Called from opt_range.cc, opt_sum.cc, sql_handler.cc, and sql_select.cc.
  @see
  opt_range.cc, opt_sum.cc, sql_handler.cc and sql_select.cc
*/
int ha_cassandra_v2::index_first(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::index_first");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


/**
  @brief
  index_last() asks for the last key in the index.
  @details
  Called from opt_range.cc, opt_sum.cc, sql_handler.cc, and sql_select.cc.
  @see
  opt_range.cc, opt_sum.cc, sql_handler.cc and sql_select.cc
*/
int ha_cassandra_v2::index_last(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::index_last");
  MYSQL_INDEX_READ_ROW_START(table_share->db.str, table_share->table_name.str);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_INDEX_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_cassandra_v2::rnd_init(bool scan)
{
  DBUG_ENTER("ha_cassandra_v2::rnd_init");
  DBUG_RETURN(0);
}

int ha_cassandra_v2::rnd_end()
{
  DBUG_ENTER("ha_cassandra_v2::rnd_end");
  DBUG_RETURN(0);
}


int ha_cassandra_v2::rnd_next(uchar *buf)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::rnd_next");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str,
                       TRUE);
  rc= HA_ERR_END_OF_FILE;
  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


void ha_cassandra_v2::position(const uchar *record)
{
  DBUG_ENTER("ha_cassandra_v2::position");
  DBUG_VOID_RETURN;
}


int ha_cassandra_v2::rnd_pos(uchar *buf, uchar *pos)
{
  int rc;
  DBUG_ENTER("ha_cassandra_v2::rnd_pos");
  MYSQL_READ_ROW_START(table_share->db.str, table_share->table_name.str,
                       TRUE);
  rc= HA_ERR_WRONG_COMMAND;
  MYSQL_READ_ROW_DONE(rc);
  DBUG_RETURN(rc);
}


int ha_cassandra_v2::info(uint flag)
{
  DBUG_ENTER("ha_cassandra_v2::info");
  DBUG_RETURN(0);
}


/**
  @brief
  extra() is called whenever the server wishes to send a hint to
  the storage engine. The myisam engine implements the most hints.
  ha_innodb.cc has the most exhaustive list of these hints.
    @see
  ha_innodb.cc
*/
int ha_cassandra_v2::extra(enum ha_extra_function operation)
{
  DBUG_ENTER("ha_cassandra_v2::extra");
  DBUG_RETURN(0);
}


int ha_cassandra_v2::delete_all_rows()
{
  DBUG_ENTER("ha_cassandra_v2::delete_all_rows");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_cassandra_v2::truncate()
{
  DBUG_ENTER("ha_cassandra_v2::truncate");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_cassandra_v2::external_lock(THD *thd, int lock_type)
{
  DBUG_ENTER("ha_cassandra_v2::external_lock");
  DBUG_RETURN(0);
}


THR_LOCK_DATA **ha_cassandra_v2::store_lock(THD *thd,
                                       THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK)
    lock.type=lock_type;
  *to++= &lock;
  return to;
}


int ha_cassandra_v2::delete_table(const char *name)
{
  DBUG_ENTER("ha_cassandra_v2::delete_table");
  /* This is not implemented but we want someone to be able that it works. */
  DBUG_RETURN(0);
}


int ha_cassandra_v2::rename_table(const char * from, const char * to)
{
  DBUG_ENTER("ha_cassandra_v2::rename_table ");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


ha_rows ha_cassandra_v2::records_in_range(uint inx, key_range *min_key,
                                     key_range *max_key)
{
  DBUG_ENTER("ha_cassandra_v2::records_in_range");
  DBUG_RETURN(10);                         // low number to force index usage
}



int ha_cassandra_v2::create(const char *name, TABLE *table_arg,
                       HA_CREATE_INFO *create_info)
{
  DBUG_ENTER("ha_cassandra_v2::create");
  /*
    This is not implemented but we want someone to be able to see that it
    works.
  */
  DBUG_RETURN(0);
}


struct st_mysql_storage_engine cassandra_v2_storage_engine=
{ MYSQL_HANDLERTON_INTERFACE_VERSION };

static ulong srv_enum_var= 0;
static ulong srv_ulong_var= 0;

const char *enum_var_names[]=
{
  "e1", "e2", NullS
};

TYPELIB enum_var_typelib=
{
  array_elements(enum_var_names) - 1, "enum_var_typelib",
  enum_var_names, NULL
};

static MYSQL_SYSVAR_ENUM(
  enum_var,                       // name
  srv_enum_var,                   // varname
  PLUGIN_VAR_RQCMDARG,            // opt
  "Sample ENUM system variable.", // comment
  NULL,                           // check
  NULL,                           // update
  0,                              // def
  &enum_var_typelib);             // typelib

static MYSQL_SYSVAR_ULONG(
  ulong_var,
  srv_ulong_var,
  PLUGIN_VAR_RQCMDARG,
  "0..1000",
  NULL,
  NULL,
  8,
  0,
  1000,
  0);

static struct st_mysql_sys_var* cassandra_v2_system_variables[]= {
  MYSQL_SYSVAR(enum_var),
  MYSQL_SYSVAR(ulong_var),
  NULL
};

// this is an cassandra_v2 of SHOW_FUNC and of my_snprintf() service
static int show_func_cassandra_v2(MYSQL_THD thd, struct st_mysql_show_var *var,
                             char *buf)
{
  var->type= SHOW_CHAR;
  var->value= buf; // it's of SHOW_VAR_FUNC_BUFF_SIZE bytes
  my_snprintf(buf, SHOW_VAR_FUNC_BUFF_SIZE,
              "enum_var is %lu, ulong_var is %lu, %.6b", // %b is MySQL extension
              srv_enum_var, srv_ulong_var, "really");
  return 0;
}

static struct st_mysql_show_var func_status[]=
{
  {"cassandra_v2_func_cassandra_v2",  (char *)show_func_cassandra_v2, SHOW_FUNC},
  {0,0,SHOW_UNDEF}
};

mysql_declare_plugin(cassandra_v2)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &cassandra_v2_storage_engine,
  "YA_SKELETON",
  "Sho Nakatani",
  "Yet Another Skeleton Storage Engine",
  PLUGIN_LICENSE_GPL,
  cassandra_v2_init_func,                            /* Plugin Init */
  NULL,                                         /* Plugin Deinit */
  0x0001 /* 0.1 */,
  func_status,                                  /* status variables */
  cassandra_v2_system_variables,                     /* system variables */
  NULL,                                         /* config options */
  0,                                            /* flags */
}
mysql_declare_plugin_end;                    
