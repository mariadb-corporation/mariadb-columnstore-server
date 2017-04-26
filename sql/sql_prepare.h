#ifndef SQL_PREPARE_H
#define SQL_PREPARE_H
/* Copyright (c) 1995-2008 MySQL AB, 2009 Sun Microsystems, Inc.
   Use is subject to license terms.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql_error.h"
#include "sql_class.h"   // @InfiniDB move here for Prepared_statement


#define LAST_STMT_ID 0xFFFFFFFF
#define STMT_ID_MASK 0x7FFFFFFF

class THD;
struct LEX;

/**
  An interface that is used to take an action when
  the locking module notices that a table version has changed
  since the last execution. "Table" here may refer to any kind of
  table -- a base table, a temporary table, a view or an
  information schema table.

  When we open and lock tables for execution of a prepared
  statement, we must verify that they did not change
  since statement prepare. If some table did change, the statement
  parse tree *may* be no longer valid, e.g. in case it contains
  optimizations that depend on table metadata.

  This class provides an interface (a method) that is
  invoked when such a situation takes place.
  The implementation of the method simply reports an error, but
  the exact details depend on the nature of the SQL statement.

  At most 1 instance of this class is active at a time, in which
  case THD::m_reprepare_observer is not NULL.

  @sa check_and_update_table_version() for details of the
  version tracking algorithm 

  @sa Open_tables_state::m_reprepare_observer for the life cycle
  of metadata observers.
*/

class Reprepare_observer
{
public:
  /**
    Check if a change of metadata is OK. In future
    the signature of this method may be extended to accept the old
    and the new versions, but since currently the check is very
    simple, we only need the THD to report an error.
  */
  bool report_error(THD *thd);
  bool is_invalidated() const { return m_invalidated; }
  void reset_reprepare_observer() { m_invalidated= FALSE; }
private:
  bool m_invalidated;
};


void mysqld_stmt_prepare(THD *thd, const char *packet, uint packet_length);
void mysqld_stmt_execute(THD *thd, char *packet, uint packet_length);
void mysqld_stmt_bulk_execute(THD *thd, char *packet, uint packet_length);
void mysqld_stmt_close(THD *thd, char *packet);
void mysql_sql_stmt_prepare(THD *thd);
void mysql_sql_stmt_execute(THD *thd);
void mysql_sql_stmt_execute_immediate(THD *thd);
void mysql_sql_stmt_close(THD *thd);
void mysqld_stmt_fetch(THD *thd, char *packet, uint packet_length);
void mysqld_stmt_reset(THD *thd, char *packet);
void mysql_stmt_get_longdata(THD *thd, char *pos, ulong packet_length);
void reinit_stmt_before_use(THD *thd, LEX *lex);

ulong bulk_parameters_iterations(THD *thd);
my_bool bulk_parameters_set(THD *thd);
/**
  Execute a fragment of server code in an isolated context, so that
  it doesn't leave any effect on THD. THD must have no open tables.
  The code must not leave any open tables around.
  The result of execution (if any) is stored in Ed_result.
*/

class Server_runnable
{
public:
  virtual bool execute_server_code(THD *thd)= 0;
  virtual ~Server_runnable();
};


/**
  Execute direct interface.

  @todo Implement support for prelocked mode.
*/

class Ed_row;

/**
  Ed_result_set -- a container with result set rows.
  @todo Implement support for result set metadata and
  automatic type conversion.
*/

class Ed_result_set: public Sql_alloc
{
public:
  operator List<Ed_row>&() { return *m_rows; }
  unsigned int size() const { return m_rows->elements; }

  Ed_result_set(List<Ed_row> *rows_arg, size_t column_count,
                MEM_ROOT *mem_root_arg);

  /** We don't call member destructors, they all are POD types. */
  ~Ed_result_set() {}

  size_t get_field_count() const { return m_column_count; }

  static void operator delete(void *ptr, size_t size) throw ();
private:
  Ed_result_set(const Ed_result_set &);        /* not implemented */
  Ed_result_set &operator=(Ed_result_set &);   /* not implemented */
private:
  MEM_ROOT m_mem_root;
  size_t m_column_count;
  List<Ed_row> *m_rows;
  Ed_result_set *m_next_rset;
  friend class Ed_connection;
};


class Ed_connection
{
public:
  /**
    Construct a new "execute direct" connection.

    The connection can be used to execute SQL statements.
    If the connection failed to initialize, the error
    will be returned on the attempt to execute a statement.

    @pre thd  must have no open tables
              while the connection is used. However,
              Ed_connection works okay in LOCK TABLES mode.
              Other properties of THD, such as the current warning
              information, errors, etc. do not matter and are
              preserved by Ed_connection. One thread may have many
              Ed_connections created for it.
  */
  Ed_connection(THD *thd);

  /**
    Execute one SQL statement.

    Until this method is executed, no other methods of
    Ed_connection can be used. Life cycle of Ed_connection is:

    Initialized -> a statement has been executed ->
    look at result, move to next result ->
    look at result, move to next result ->
    ...
    moved beyond the last result == Initialized.

    This method can be called repeatedly. Once it's invoked,
    results of the previous execution are lost.

    A result of execute_direct() can be either:

    - success, no result set rows. In this case get_field_count()
    returns 0. This happens after execution of INSERT, UPDATE,
    DELETE, DROP and similar statements. Some other methods, such
    as get_affected_rows() can be used to retrieve additional
    result information.

    - success, there are some result set rows (maybe 0). E.g.
    happens after SELECT. In this case get_field_count() returns
    the number of columns in a result set and store_result()
    can be used to retrieve a result set..

    - an error, methods to retrieve error information can
    be used.

    @return execution status
    @retval FALSE  success, use get_field_count()
                   to determine what to do next.
    @retval TRUE   error, use get_last_error()
                   to see the error number.
  */
  bool execute_direct(LEX_STRING sql_text);

  /**
    Same as the previous, but takes an instance of Server_runnable
    instead of SQL statement text.

    @return execution status
      
    @retval  FALSE  success, use get_field_count() 
                    if your code fragment is supposed to
                    return a result set
    @retval  TRUE   failure
  */
  bool execute_direct(Server_runnable *server_runnable);

  /**
    Get the number of result set fields.

    This method is valid only if we have a result:
    execute_direct() has been called. Otherwise
    the returned value is undefined.

    @sa Documentation for C API function
    mysql_field_count()
  */
  ulong get_field_count() const
  {
    return m_current_rset ? m_current_rset->get_field_count() : 0;
  }

  /**
    Get the number of affected (deleted, updated)
    rows for the current statement. Can be
    used for statements with get_field_count() == 0.

    @sa Documentation for C API function
    mysql_affected_rows().
  */
  ulonglong get_affected_rows() const
  {
    return m_diagnostics_area.affected_rows();
  }

  /**
    Get the last insert id, if any.

    @sa Documentation for mysql_insert_id().
  */
  ulonglong get_last_insert_id() const
  {
    return m_diagnostics_area.last_insert_id();
  }

  /**
    Get the total number of warnings for the last executed
    statement. Note, that there is only one warning list even
    if a statement returns multiple results.

    @sa Documentation for C API function
    mysql_num_warnings().
  */
  ulong get_warn_count() const
  {
    return m_diagnostics_area.warn_count();
  }

  /**
    The following members are only valid if execute_direct()
    or move_to_next_result() returned an error.
    They never fail, but if they are called when there is no
    result, or no error, the result is not defined.
  */
  const char *get_last_error() const { return m_diagnostics_area.message(); }
  unsigned int get_last_errno() const { return m_diagnostics_area.sql_errno(); }
  const char *get_last_sqlstate() const { return m_diagnostics_area.get_sqlstate(); }

  /**
    Provided get_field_count() is not 0, this never fails. You don't
    need to free the result set, this is done automatically when
    you advance to the next result set or destroy the connection.
    Not returning const because of List iterator not accepting
    Should be used when you would like Ed_connection to manage
    result set memory for you.
  */
  Ed_result_set *use_result_set() { return m_current_rset; }
  /**
    Provided get_field_count() is not 0, this never fails. You
    must free the returned result set. This can be called only
    once after execute_direct().
    Should be used when you would like to get the results
    and destroy the connection.
  */
  Ed_result_set *store_result_set();

  /**
    If the query returns multiple results, this method
    can be checked if there is another result beyond the next
    one.
    Never fails.
  */
  bool has_next_result() const { return MY_TEST(m_current_rset->m_next_rset); }
  /**
    Only valid to call if has_next_result() returned true.
    Otherwise the result is undefined.
  */
  bool move_to_next_result()
  {
    m_current_rset= m_current_rset->m_next_rset;
    return MY_TEST(m_current_rset);
  }

  ~Ed_connection() { free_old_result(); }
private:
  Diagnostics_area m_diagnostics_area;
  /**
    Execute direct interface does not support multi-statements, only
    multi-results. So we never have a situation when we have
    a mix of result sets and OK or error packets. We either
    have a single result set, a single error, or a single OK,
    or we have a series of result sets, followed by an OK or error.
  */
  THD *m_thd;
  Ed_result_set *m_rsets;
  Ed_result_set *m_current_rset;
  friend class Protocol_local;
private:
  void free_old_result();
  void add_result_set(Ed_result_set *ed_result_set);
private:
  Ed_connection(const Ed_connection &);        /* not implemented */
  Ed_connection &operator=(Ed_connection &);   /* not implemented */
};


/** One result set column. */

struct Ed_column: public LEX_STRING
{
  /** Implementation note: destructor for this class is never called. */
};


/** One result set record. */

class Ed_row: public Sql_alloc
{
public:
  const Ed_column &operator[](const unsigned int column_index) const
  {
    return *get_column(column_index);
  }
  const Ed_column *get_column(const unsigned int column_index) const
  {
    DBUG_ASSERT(column_index < size());
    return m_column_array + column_index;
  }
  size_t size() const { return m_column_count; }

  Ed_row(Ed_column *column_array_arg, size_t column_count_arg)
    :m_column_array(column_array_arg),
    m_column_count(column_count_arg)
  {}
private:
  Ed_column *m_column_array;
  size_t m_column_count; /* TODO: change to point to metadata */
};

// InfiniDB: Move class Select_fetch_protocol_binary and Prepared_statement's definition from sql_parse.cc here
class Select_fetch_protocol_binary: public select_send
{
  Protocol_binary protocol;
public:
  Select_fetch_protocol_binary(THD *thd);
  virtual bool send_result_set_metadata(List<Item> &list, uint flags);
  virtual int send_data(List<Item> &items);
  virtual bool send_eof();
#ifdef EMBEDDED_LIBRARY
  void begin_dataset()
  {
    protocol.begin_dataset();
  }
#endif
};

/****************************************************************************/

/**
  Prepared_statement: a statement that can contain placeholders.
*/

class Prepared_statement: public Statement
{
public:
  enum flag_values
  {
    IS_IN_USE= 1,
    IS_SQL_PREPARE= 2
  };

  THD *thd;
  Select_fetch_protocol_binary result;
  Item_param **param_array;
  Server_side_cursor *cursor;
  uchar *packet;
  uchar *packet_end;
  ulong iterations;
  uint param_count;
  uint last_errno;
  uint flags;
  /*
    The value of thd->select_number at the end of the PREPARE phase.

    The issue is: each statement execution opens VIEWs, which may cause 
    select_lex objects to be created, and select_number values to be assigned.

    On the other hand, PREPARE assigns select_number values for triggers and
    subqueries.

    In order for select_number values from EXECUTE not to conflict with
    select_number values from PREPARE, we keep the number and set it at each
    execution.
  */
  uint select_number_after_prepare;
  char last_error[MYSQL_ERRMSG_SIZE];
  my_bool start_param;
#ifndef EMBEDDED_LIBRARY
  bool (*set_params)(Prepared_statement *st, uchar *data, uchar *data_end,
                     uchar *read_pos, String *expanded_query);
  bool (*set_bulk_params)(Prepared_statement *st,
                          uchar **read_pos, uchar *data_end, bool reset);
#else
  bool (*set_params_data)(Prepared_statement *st, String *expanded_query);
  /*TODO: add bulk support for builtin server */
#endif
  bool (*set_params_from_actual_params)(Prepared_statement *stmt,
                                        List<Item> &list,
                                        String *expanded_query);
public:
  Prepared_statement(THD *thd_arg);
  virtual ~Prepared_statement();
  void setup_set_params();
  virtual Query_arena::Type type() const;
  virtual void cleanup_stmt();
  bool set_name(LEX_STRING *name);
  inline void close_cursor();
  inline bool is_in_use() { return flags & (uint) IS_IN_USE; }
  inline bool is_sql_prepare() const { return flags & (uint) IS_SQL_PREPARE; }
  void set_sql_prepare() { flags|= (uint) IS_SQL_PREPARE; }
  bool prepare(const char *packet, uint packet_length);
  bool execute_loop(String *expanded_query,
                    bool open_cursor,
                    uchar *packet_arg, uchar *packet_end_arg);
  bool execute_bulk_loop(String *expanded_query,
                         bool open_cursor,
                         uchar *packet_arg, uchar *packet_end_arg,
                         ulong iterations);
  bool execute_server_runnable(Server_runnable *server_runnable);
  my_bool set_bulk_parameters(bool reset);
  ulong bulk_iterations();
  /* Destroy this statement */
  void deallocate();
  bool execute_immediate(const char *query, uint query_length);
  bool set_parameters(String *expanded_query,
                      uchar *packet, uchar *packet_end);
private:
  /**
    The memory root to allocate parsed tree elements (instances of Item,
    SELECT_LEX and other classes).
  */
  MEM_ROOT main_mem_root;
  sql_mode_t m_sql_mode;

  bool set_db(const char *db, uint db_length);
  bool execute(String *expanded_query, bool open_cursor);
  void deallocate_immediate();
  bool reprepare();
  bool validate_metadata(Prepared_statement  *copy);
  void swap_prepared_statement(Prepared_statement *copy);
};


#endif // SQL_PREPARE_H
