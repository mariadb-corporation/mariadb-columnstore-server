/* Copyright (C) 2013 Calpont Corp.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/***********************************************************************
*   $Id: item_window_function.h 9559 2013-05-22 17:58:13Z xlou $
*
*
***********************************************************************/

#ifndef ITEM_WINDOW_FUNCTION_H
#define ITEM_WINDOW_FUNCTION_H
#include "sql_class.h"
#include "item.h"
#include "errorids.h"                   /* InfiniDB Window Function errors */

enum BOUND
{
	PRECEDING = 0,
	FOLLOWING,
	UNBOUNDED_PRECEDING,
	UNBOUNDED_FOLLOWING,
	CURRENT_ROW
};

struct Boundary
{
	Boundary();
	Boundary(BOUND b);
	~Boundary() {}
	bool fix_fields(THD* thd, Item** ref);
	Item* item; // has to evaluate to unsigned value
	enum BOUND bound;
};

struct Frame
{
	Frame();
	~Frame();
	bool fix_fields(THD* thd, Item** ref);
	Boundary* start;
	Boundary* end;
	bool isRange; // RANGE or ROWS
};

struct Ordering
{
	Ordering();
	~Ordering();
	bool fix_fields(THD* thd, Item** ref);
	SQL_LIST* orders;
	Frame* frame;
};

struct Window_context
{
	Window_context();
	~Window_context();
	Item** partitions;
	uint partition_count;
	Ordering* ordering;
	void setPartitions(List<Item>* list);
	void setOrders(Ordering* order) { ordering = order; }
	bool fix_fields(THD* thd, Item** ref);

};

class Item_func_window : public Item_func
{
public:
	Item_func_window(THD* thd, LEX_STRING name,
	                 Window_context* ctx = NULL,
	                 bool dist = false);
	Item_func_window(THD* thd, LEX_STRING name,
	                 Item *arg,
	                 Window_context* ctx = NULL,
	                 bool dist = false);
	Item_func_window(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Window_context* ctx = NULL,
	                 bool dist = false);
	Item_func_window(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Item* arg3,
	                 Window_context* ctx = NULL,
	                 bool dist = false);
	Item_func_window(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Item* arg3,
	                 Item* arg4,
	                 Window_context* ctx = NULL,
	                 bool dist = false);
	virtual ~Item_func_window();
	const char *func_name() const { return funcname; }
	virtual void fix_length_and_dec();
	virtual enum Type type() const { return WINDOW_FUNC_ITEM; }

	// the following should probably error out because we'll never need mysql to evaluate
	// window functions.
	virtual double val_real();
	virtual longlong val_int();
	virtual String* val_str(String*);
	bool check_partition_func_processor(uchar *int_arg) {return FALSE;}
	void partition_by (List<Item>* list) { window_context->setPartitions(list); }
	Item** partition_by() {return window_context->partitions;}
	void order_by(Ordering* order) { window_context->setOrders(order); }
	Ordering* order_by() {return window_context->ordering;}
	virtual bool fix_fields(THD* thd, Item** ref);
	Window_context* window_ctx() { return window_context; }
	void window_ctx(Window_context* window_ctx) { window_context = window_ctx; }
	virtual enum Item_result result_type () const;
	void isDistinct(bool dist) { distinct = dist; }
	bool isDistinct() const { return distinct; }
	bool respectNulls; // for some window functions

	// @bug5777, to distinguish two types of real result type.
	virtual enum_field_types field_type() { return MYSQL_TYPE_DOUBLE; }

protected:
	Window_context* window_context;
	Item_result hybrid_type;
	bool distinct;
	char* funcname;
};

class Item_func_window_hybrid : public Item_func_window
{
public:
	Item_func_window_hybrid(THD* thd, LEX_STRING name,
	                 Window_context* ctx = NULL,
	                 bool dist = false) :
	                 Item_func_window(thd, name, ctx, dist)
	{}
	Item_func_window_hybrid(THD* thd, LEX_STRING name,
	                 Item *arg,
	                 Window_context* ctx = NULL,
	                 bool dist = false) :
	                 Item_func_window(thd, name, arg, ctx, dist)
	{}
	Item_func_window_hybrid(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Window_context* ctx = NULL,
	                 bool dist = false):
	                 Item_func_window(thd, name, arg1, arg2, ctx, dist)
	{}
	Item_func_window_hybrid(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Item* arg3,
	                 Window_context* ctx = NULL,
	                 bool dist = false):
	                 Item_func_window(thd, name, arg1, arg2, arg3, ctx, dist)
	{}
	Item_func_window_hybrid(THD* thd, LEX_STRING name,
	                 Item *arg1,
	                 Item *arg2,
	                 Item* arg3,
	                 Item* arg4,
	                 Window_context* ctx = NULL,
	                 bool dist = false) :
	                 Item_func_window(thd, name, arg1, arg2, arg3, arg4, ctx, dist)
	{}
	virtual ~Item_func_window_hybrid() {}
	virtual enum_field_types field_type();
};

// avg
class Item_func_window_avg : public Item_func_window
{
public:
	Item_func_window_avg(THD* thd, LEX_STRING name,
	                     Item *arg,
	                     Window_context* ctx = NULL,
	                     bool dist = false) :
	                     Item_func_window(thd, name, arg, ctx, dist){}
	void fix_length_and_dec();
	enum Item_result result_type () const { return hybrid_type; }
	uint prec_increment;
};

// sum
class Item_func_window_sum : public Item_func_window
{
public:
	Item_func_window_sum(THD* thd, LEX_STRING name,
	                     Item *arg,
	                     Window_context* ctx = NULL,
	                     bool dist = false) :
	                     Item_func_window(thd, name, arg, ctx, dist){}
	void fix_length_and_dec();
	enum Item_result result_type () const { return hybrid_type; }
};

// rank
class Item_func_window_rank : public Item_func_window
{
public:
	Item_func_window_rank(THD* thd, LEX_STRING name,
		                  Window_context* ctx = NULL,
	                      bool dist = false) :
	                      Item_func_window(thd, name, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
	virtual enum Item_result result_type () const;
	virtual void fix_length_and_dec();
	uint prec_increment;
};

// nth_value
class Item_func_window_nth_value : public Item_func_window_hybrid
{
public:
	Item_func_window_nth_value(THD* thd, LEX_STRING name,
	                           Item *arg1,
	                           Item *arg2,
	                           Item *arg3,
	                           Item *arg4,
	                           Window_context* ctx = NULL,
	                           bool dist = false) :
	                           Item_func_window_hybrid(thd, name, arg1, arg2, arg3, arg4, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
};

// statistic function: stddev_pop/stddev_samp, var_pop/var_samp
class Item_func_window_stats : public Item_func_window
{
public:
	Item_func_window_stats(THD* thd, LEX_STRING name,
	                       Item *arg,
	                       Window_context* ctx = NULL,
	                       bool dist = false) :
	                       Item_func_window(thd, name, arg, ctx, dist){}
	virtual enum Item_result result_type () const;
	virtual void fix_length_and_dec();
	uint prec_increment;
};

// count
class Item_func_window_int : public Item_func_window
{
public:
	Item_func_window_int(THD* thd, LEX_STRING name,
		                 Window_context* ctx = NULL,
	                     bool dist = false) :
	                     Item_func_window(thd, name, ctx, dist){}
	Item_func_window_int(THD* thd, LEX_STRING name,
	                     Item *arg,
	                     Window_context* ctx = NULL,
	                     bool dist = false) :
	                     Item_func_window(thd, name, arg, ctx, dist){}
	virtual enum Item_result result_type () const;
};

// lead/lag
class Item_func_window_lead_lag : public Item_func_window_hybrid
{
public:
	Item_func_window_lead_lag(THD* thd, LEX_STRING name,
	                          Item *arg1,
	                          Item *arg2,
	                          Item *arg3,
	                          Item *arg4,
	                          Window_context* ctx = NULL,
	                          bool dist = false) :
	                          Item_func_window_hybrid(thd, name, arg1, arg2, arg3, arg4, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
};

// median
class Item_func_window_median : public Item_func_window
{
public:
	Item_func_window_median (THD* thd, LEX_STRING name,
	                         Item *arg1,
	                         Window_context* ctx = NULL,
	                         bool dist = false) :
	                         Item_func_window(thd, name, arg1, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
	virtual void fix_length_and_dec();
	virtual enum Item_result result_type () const;
	uint prec_increment;
};

// row_number
class Item_func_window_rownumber : public Item_func_window
{
public:
	Item_func_window_rownumber (THD* thd, LEX_STRING name,
	                            Window_context* ctx = NULL,
	                            bool dist = false) :
	                            Item_func_window(thd, name, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
};

// ntile
class Item_func_window_ntile : public Item_func_window
{
public:
	Item_func_window_ntile (THD* thd, LEX_STRING name,
	                        Item *arg1,
	                        Window_context* ctx = NULL,
	                        bool dist = false) :
	                        Item_func_window(thd, name, arg1, ctx, dist){}
	virtual bool fix_fields(THD* thd, Item** ref);
};

// percentile
class Item_func_window_percentile : public Item_func_window
{
public:
	Item_func_window_percentile (THD* thd, LEX_STRING name,
	                             Item *arg1,
	                             SQL_LIST* order,
	                             Window_context* ctx = NULL) :
	                               Item_func_window(thd, name, arg1, ctx, false),
	                               group_order (order){}
	virtual bool fix_fields(THD* thd, Item** ref);
	virtual void fix_length_and_dec();
	SQL_LIST* group_order;
	uint prec_increment;
	virtual enum Item_result result_type () const;
};

#endif
