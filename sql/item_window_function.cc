/* Copyright (C) 2013 Calpont Corp.
Copyright (c) 2016, MariaDB Corporation

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
*   $Id: item_window_function.cc 9560 2013-05-22 19:08:18Z xlou $
*
*
***********************************************************************/

#include "item_window_function.h"
#include "sql_parse.h"
/**
 * Boundary definition
 */

Boundary::Boundary() : item (NULL)
{}

Boundary::Boundary(BOUND b): item(NULL), bound(b)
{}

bool Boundary::fix_fields(THD* thd, Item** ref)
{
	if (item && item->fix_fields(thd, &item))
		return TRUE;
	return FALSE;
}

/**
 * Frame definition
 */

Frame::Frame() : start(NULL), end(NULL), isRange(true)
{}

Frame::~Frame()
{
	delete start;
	delete end;
}

bool Frame::fix_fields(THD* thd, Item** ref)
{
	if (start && start->fix_fields(thd, ref))
		return TRUE;
	if (end && end->fix_fields(thd, ref))
		return TRUE;
	return FALSE;
}

/**
 * Ordering definition
 */

Ordering::Ordering() : orders (NULL), frame(NULL)
{}

Ordering::~Ordering()
{
	if (orders)
	{
		orders->empty();
		delete orders;
	}
	delete frame;
}

bool Ordering::fix_fields(THD* thd, Item** ref)
{
	if (orders && orders->first)
	{
		ORDER* orderCol = reinterpret_cast<ORDER*>(orders->first);
		for (; orderCol; orderCol= orderCol->next)
		{
			Item* orderItem = *(orderCol->item);
			if (orderItem->fix_fields(thd, (orderCol->item)))
				return TRUE;
		}
	}
	if (frame && frame->fix_fields(thd, ref))
		return TRUE;
	return FALSE;
}

/**
 * Window_context definition
 */
Window_context::Window_context(): partitions(NULL), partition_count(0), ordering(NULL)
{}

Window_context::~Window_context()
{
	//printf("Window_context destructor\n");
	delete ordering;
}

bool Window_context::fix_fields(THD* thd, Item** ref)
{
	// check partition columns
	for (uint i = 0; i < partition_count; i++)
	{
		if (partitions[i]->fix_fields(thd, &partitions[i]))
			return TRUE;
	}

	// check order by
	if (ordering && ordering->fix_fields(thd, ref))
		return TRUE;

	return FALSE;
}

void Window_context::setPartitions(List<Item>* list)
{
	if (!list)
	{
		partition_count = 0;
		partitions = NULL;
		return;
	}
	partition_count = list->elements;
	if ((partitions = (Item**) sql_alloc(sizeof(Item*)*partition_count)))
	{
		List_iterator_fast<Item> li(*list);
		Item *item;
		Item **save_args= partitions;

		while ((item=li++))
		{
			*(save_args++)= item;
		}
	}
	list->empty();     // Fields are used
}

/**
 * Item_func_window definition
 */
Item_func_window::Item_func_window(THD* thd, LEX_STRING name,
                   Window_context* ctx,
                   bool dist):
                   Item_func(thd),
                   window_context(ctx),
                   distinct(dist),
                   funcname(name.str)
{}

Item_func_window::Item_func_window(THD* thd, LEX_STRING name,
                   Item *arg,
                   Window_context* ctx,
                   bool dist):
                   Item_func(thd, arg),
                   window_context(ctx),
                   distinct(dist),
                   funcname(name.str)
{}

Item_func_window::Item_func_window(THD* thd, LEX_STRING name,
                   Item *arg1,
                   Item *arg2,
                   Window_context* ctx,
                   bool dist):
                   Item_func(thd, arg1, arg2),
                   window_context(ctx),
                   distinct(dist),
                   funcname(name.str)
{}

Item_func_window::Item_func_window(THD* thd, LEX_STRING name,
                   Item *arg1,
                   Item *arg2,
                   Item *arg3,
                   Window_context* ctx,
                   bool dist):
                   Item_func(thd, arg1, arg2, arg3),
                   window_context(ctx),
                   distinct(dist),
                   funcname(name.str)
{}

Item_func_window::Item_func_window(THD* thd, LEX_STRING name,
                   Item *arg1,
                   Item *arg2,
                   Item *arg3,
                   Item *arg4,
                   Window_context* ctx,
                   bool dist):
                   Item_func(thd, arg1, arg2, arg3, arg4),
                   window_context(ctx),
                   distinct(dist),
                   funcname(name.str)
{}

Item_func_window::~Item_func_window()
{
	// The items are deleted by THD::cleanup_after_query
	//printf("destructor called\n");
	delete window_context;
}

bool Item_func_window::fix_fields(THD* thd, Item** ref)
{
	// check func arg
	if (Item_func::fix_fields(thd, ref))
		return TRUE;

	// check context
	if (window_context && window_context->fix_fields(thd, ref))
		return TRUE;

	const_item_cache = 0;

	return false;
}

enum Item_result Item_func_window::result_type () const
{
	if (args && arg_count >= 1)
		return args[0]->result_type();
	else
		return hybrid_type;
}

void Item_func_window::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window::fix_length_and_dec");
	maybe_null = null_value = 1;

	if (args && arg_count >= 1)
	{
		decimals = args[0]->decimals;
		max_length = args[0]->max_length;
		switch (args[0]->result_type())
		{
		case REAL_RESULT:
		case STRING_RESULT:
			hybrid_type = REAL_RESULT;
			break;
		case INT_RESULT:
			hybrid_type = INT_RESULT;
		case DECIMAL_RESULT:
		{
			hybrid_type = DECIMAL_RESULT;
			/* SUM result can't be longer than length(arg) + length(MAX_ROWS) */
			int precision= args[0]->decimal_precision() + DECIMAL_LONGLONG_DIGITS;
			max_length= my_decimal_precision_to_length_no_truncation(precision,
			                                                         decimals,
			                                                         unsigned_flag);
			//my_decimal_set_zero(dec_buffs);
			break;
		}
		case ROW_RESULT:
		default:
			DBUG_ASSERT(0);
		}
	}
	else // default for no arg
	{
		hybrid_type = INT_RESULT;

		int precision= MY_INT64_NUM_DECIMAL_DIGITS;
		decimals= 0;
		max_length= my_decimal_precision_to_length_no_truncation(precision,
		                                                         decimals,
		                                                         unsigned_flag);
	}
	DBUG_VOID_RETURN;
}

// No mysql evaluation. error out
double Item_func_window::val_real()
{
	if (!(current_thd->infinidb_vtable.vtable_state == THD::INFINIDB_CREATE_VTABLE))
		IDB_set_error(current_thd, logging::ERR_WF_IDB_ONLY, NULL, 0);
	return 0;
}

// No mysql evaluation. error out
longlong Item_func_window::val_int()
{
	if (!(current_thd->infinidb_vtable.vtable_state == THD::INFINIDB_CREATE_VTABLE))
		IDB_set_error(current_thd, logging::ERR_WF_IDB_ONLY, NULL, 0);
	return 0;
}

// No mysql evaluation. error out
String* Item_func_window::val_str(String*)
{
	if (!(current_thd->infinidb_vtable.vtable_state == THD::INFINIDB_CREATE_VTABLE))
		IDB_set_error(current_thd, logging::ERR_WF_IDB_ONLY, NULL, 0);
	return 0;
}

enum_field_types Item_func_window_hybrid::field_type() const
{
	if (args && arg_count < 1)
	{
		IDB_set_error(current_thd, logging::ERR_WF_WRONG_ARGS, funcname);
		return MYSQL_TYPE_DOUBLE;
	}
	return args[0]->field_type();
}

void Item_func_window_avg::fix_length_and_dec ()
{
	DBUG_ENTER("Item_func_window_avg::fix_length_and_dec");
	Item_func_window::fix_length_and_dec();
	maybe_null=null_value=1;
	prec_increment= current_thd->variables.div_precincrement;
	if (hybrid_type == DECIMAL_RESULT)
	{
		int precision= args[0]->decimal_precision() + prec_increment;
		decimals= std::min(args[0]->decimals + prec_increment, (uint)DECIMAL_MAX_SCALE);
		max_length= my_decimal_precision_to_length_no_truncation(precision,
		                                                         decimals,
		                                                         unsigned_flag);
	}
	else
	{
		decimals= std::min(args[0]->decimals + prec_increment, (uint)NOT_FIXED_DEC);
		max_length= args[0]->max_length + prec_increment;
	}
	DBUG_VOID_RETURN;
}

void Item_func_window_sum::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window_sum::fix_length_and_dec");
	maybe_null=null_value=1;
	decimals= args[0]->decimals;
	switch (args[0]->result_type()) {
	case REAL_RESULT:
	case STRING_RESULT:
		hybrid_type= REAL_RESULT;
		break;
	case INT_RESULT:
	case DECIMAL_RESULT:
	{
		/* SUM result can't be longer than length(arg) + length(MAX_ROWS) */
		int precision= args[0]->decimal_precision() + DECIMAL_LONGLONG_DIGITS;
		max_length= my_decimal_precision_to_length_no_truncation(precision,
		                                                         decimals,
		                                                         unsigned_flag);
		//curr_dec_buff= 0;
		hybrid_type= DECIMAL_RESULT;
		//my_decimal_set_zero(dec_buffs);
		break;
	}
	case ROW_RESULT:
	default:
		DBUG_ASSERT(0);
	}
	DBUG_VOID_RETURN;
}

bool Item_func_window_rank::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	// validate order by clause exists
	if (!window_context || !window_context->ordering)
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_MISSING, funcname);
		return TRUE;
	}

	// validate no window clause exists
	if (window_context && window_context->ordering->frame)
	{
		IDB_set_error(thd, logging::ERR_WF_WINDOW_CLAUSE, funcname);
		return TRUE;
	}

	// default window is unbounded preceding and unbounded following
	Frame *frame = new Frame();
	frame->start = new Boundary(UNBOUNDED_PRECEDING);
	frame->end = new Boundary(UNBOUNDED_FOLLOWING);
	window_context->ordering->frame = frame;

	return FALSE;
}

enum Item_result Item_func_window_rank::result_type () const
{
	if (strcasecmp(funcname, "CUME_DIST") == 0 ||
		  strcasecmp(funcname, "PERCENT_RANK") == 0)
		return REAL_RESULT;
	else
		return INT_RESULT;
}

void Item_func_window_rank::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window_rank::fix_length_and_dec");
	if (!(strcasecmp(funcname, "CUME_DIST") == 0 ||
		  strcasecmp(funcname, "PERCENT_RANK") == 0))
		DBUG_VOID_RETURN;
	maybe_null= null_value= 1;
	prec_increment= current_thd->variables.div_precincrement;
	hybrid_type= REAL_RESULT;

	int precision= 10 + prec_increment;
	decimals= 10;
	max_length= my_decimal_precision_to_length_no_truncation(precision,
	                                                         decimals,
	                                                       unsigned_flag);
	DBUG_VOID_RETURN;
}

bool Item_func_window_nth_value::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	if (!args || arg_count != 4)
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name);
		return TRUE;
	}

	// validate the 2nd arg has to be numeric type
	if (!args[0] ||
	    !args[1] ||
	    (args[1]->result_type() == STRING_RESULT &&
	     args[1]->type() != Item::NULL_ITEM))
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, funcname);
		return TRUE;
	}

	return FALSE;
}

enum Item_result Item_func_window_stats::result_type () const
{
	return REAL_RESULT;
}

void Item_func_window_stats::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window_stats::fix_length_and_dec");
	maybe_null= null_value= 1;
	prec_increment= current_thd->variables.div_precincrement;

	/*
	  According to the SQL2003 standard (Part 2, Foundations; sec 10.9,
	  aggregate function; paragraph 7h of Syntax Rules), "the declared
	  type of the result is an implementation-defined aproximate numeric
	  type.
	*/
	hybrid_type= REAL_RESULT;

	switch (args[0]->result_type()) {
	case REAL_RESULT:
	case STRING_RESULT:
		decimals= std::min(args[0]->decimals + 4, (uint)NOT_FIXED_DEC);
		break;
	case INT_RESULT:
	case DECIMAL_RESULT:
	{
		int precision= args[0]->decimal_precision()*2 + prec_increment;
		decimals= std::min(args[0]->decimals + prec_increment, (uint)DECIMAL_MAX_SCALE);
		max_length= my_decimal_precision_to_length_no_truncation(precision,
		                                                         decimals,
		                                                         unsigned_flag);

		break;
	}
	case ROW_RESULT:
	default:
		DBUG_ASSERT(0);
	}
	DBUG_VOID_RETURN;
}

enum Item_result Item_func_window_int::result_type () const
{
	return INT_RESULT;
}

// LEAD, LAG
bool Item_func_window_lead_lag::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	// validate order by clause exists
	if (!window_context ||
		  !window_context->ordering)
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_MISSING, funcname);
		return TRUE;
	}

	// validate no window clause exists
	if (window_context &&
		  window_context->ordering->frame)
	{
		IDB_set_error(thd, logging::ERR_WF_WINDOW_CLAUSE, funcname);
		return TRUE;
	}

	// default window is unbounded preceding and unbounded following
	if (window_context && window_context->ordering)
	{
		Frame *frame = new Frame();
		frame->start = new Boundary(UNBOUNDED_PRECEDING);
		frame->end = new Boundary(UNBOUNDED_FOLLOWING);
		window_context->ordering->frame = frame;
	}

	return FALSE;
}

// Median -- equivelent to percentile_cont
bool Item_func_window_median::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE; // no order by is allowed

	if (arg_count != 1 || !args)
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name);
		return TRUE;
	}

	if (window_context && window_context->ordering)
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_BY, funcname);
		return TRUE;
	}

	// add arg to order by list. share the pointer here because it's read only
	SQL_LIST* orders = new SQL_LIST();
	orders->elements= 0;
	orders->first= 0;
	orders->next= &(orders->first);
	if (add_to_list(thd, *orders, args[0], true/*, 0*/)) // decending, null last
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_BY, funcname);
		return TRUE;
	}

	args[0] = new (thd->mem_root) Item_decimal(thd, "0.5", 3, thd->charset());

	window_context->ordering = new Ordering();
	window_context->ordering->orders = orders;
	window_context->ordering->frame = new Frame();
	window_context->ordering->frame->start = new Boundary(UNBOUNDED_PRECEDING);
	window_context->ordering->frame->end = new Boundary(UNBOUNDED_FOLLOWING);

	return FALSE;
}

enum Item_result Item_func_window_median::result_type () const
{
	return REAL_RESULT;
}

void Item_func_window_median::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window_median::fix_length_and_dec");
	maybe_null= null_value= 1;
	prec_increment= current_thd->variables.div_precincrement;
	hybrid_type= REAL_RESULT;

	int precision= 10 + prec_increment;
	decimals= 10;
	max_length= my_decimal_precision_to_length_no_truncation(precision,
	                                                         decimals,
	                                                       unsigned_flag);
	DBUG_VOID_RETURN;
}

bool Item_func_window_rownumber::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	// validate no window clause exists
	if (window_context &&
		  window_context->ordering &&
		  window_context->ordering->frame)
	{
		IDB_set_error(thd, logging::ERR_WF_WINDOW_CLAUSE, funcname);
		return TRUE;
	}

	// default window is unbounded preceding and unbounded following
	if (window_context && window_context->ordering)
	{
		Frame *frame = new Frame();
		frame->start = new Boundary(UNBOUNDED_PRECEDING);
		frame->end = new Boundary(UNBOUNDED_FOLLOWING);
		window_context->ordering->frame = frame;
	}

	return FALSE;
}

bool Item_func_window_ntile::fix_fields(THD* thd, Item** ref)
{
	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	// validate order by clause exists
	if (!window_context ||
		  !window_context->ordering)
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_MISSING, funcname);
		return TRUE;
	}

	// validate no window clause exists
	if (window_context && window_context->ordering->frame)
	{
		IDB_set_error(thd, logging::ERR_WF_WINDOW_CLAUSE, funcname);
		return TRUE;
	}

	// validate the arg has to be numeric type
	if (!args[0] ||
	    (args[0]->result_type() == STRING_RESULT &&
	     args[0]->type() != Item::NULL_ITEM))
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, funcname);
		return TRUE;
	}

	// default window is unbounded preceding and unbounded following
	if (window_context && window_context->ordering)
	{
		Frame *frame = new Frame();
		frame->start = new Boundary(UNBOUNDED_PRECEDING);
		frame->end = new Boundary(UNBOUNDED_FOLLOWING);
		window_context->ordering->frame = frame;
	}

	return FALSE;
}

bool Item_func_window_percentile::fix_fields(THD* thd, Item** ref)
{
	// add group ordering to window_context order by clause
	if (group_order && group_order->first)
	{
		ORDER* orderCol = reinterpret_cast<ORDER*>(group_order->first);
		for (; orderCol; orderCol= orderCol->next)
		{
			Item* orderItem = *(orderCol->item);
			if (orderItem->fix_fields(thd, (orderCol->item)))
				return TRUE;
		}
	}

	if (Item_func_window::fix_fields(thd, ref))
		return TRUE;

	// validate order by clause not exists
	if (window_context && window_context->ordering)
	{
		IDB_set_error(thd, logging::ERR_WF_ORDER_BY, funcname);
		return TRUE;
	}

	if (group_order && group_order->elements >1 )
	{
		IDB_set_error(thd, logging::ERR_WF_INVALID_ORDER_KEY_WITHIN, funcname);
		return TRUE;
	}

	if (!window_context)
		window_context = new Window_context();

	if (group_order)
	{
		window_context->ordering = new Ordering();
		window_context->ordering->orders = group_order;
	}

	// default window is unbounded preceding and unbounded following
	if (window_context && window_context->ordering)
	{
		Frame *frame = new Frame();
		frame->start = new Boundary(UNBOUNDED_PRECEDING);
		frame->end = new Boundary(UNBOUNDED_FOLLOWING);
		window_context->ordering->frame = frame;
	}

	return FALSE;
}

void Item_func_window_percentile::fix_length_and_dec()
{
	DBUG_ENTER("Item_func_window_percentile::fix_length_and_dec");
	maybe_null= null_value= 1;
	prec_increment= current_thd->variables.div_precincrement;
	hybrid_type= REAL_RESULT;

	if (strcasecmp(funcname, "PERCENTILE_CONT") == 0)
	{
		int precision= 10 + prec_increment;
		decimals= 10;
		max_length= my_decimal_precision_to_length_no_truncation(precision,
		                                                         decimals,
		                                                         unsigned_flag);
	}
	else
	{
		if (group_order && group_order->first)
		{
			ORDER* orderCol = reinterpret_cast<ORDER*>(group_order->first);
			Item* orderItem = *(orderCol->item);
			switch (orderItem->result_type())
			{
				case REAL_RESULT:
				case STRING_RESULT:
					hybrid_type= REAL_RESULT;
					break;
				case INT_RESULT:
				case DECIMAL_RESULT:
				{
					/* SUM result can't be longer than length(arg) + length(MAX_ROWS) */
					decimals = orderItem->decimals;
					int precision= args[0]->decimal_precision() + DECIMAL_LONGLONG_DIGITS;
					max_length= my_decimal_precision_to_length_no_truncation(precision,
					                                                         decimals,
					                                                         unsigned_flag);
					//curr_dec_buff= 0;
					hybrid_type= DECIMAL_RESULT;
					//my_decimal_set_zero(dec_buffs);
					break;
				}
				case ROW_RESULT:
				default:
					DBUG_ASSERT(0);
			}
		}
	}
	DBUG_VOID_RETURN;
}

enum Item_result Item_func_window_percentile::result_type () const
{
	if (strcasecmp(funcname, "PERCENTILE_DISC") == 0)
	{
		// percentile_disc return order by column type
		ORDER* col = reinterpret_cast<ORDER*>(group_order->first);
		if (col)
		{
			Item* item = *(col->item);
			if (item)
				return item->result_type();
		}
	}
	return hybrid_type;
}




