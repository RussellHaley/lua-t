/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t_ort.c
 * \brief     data types for an ordered table
 *            Elements can be accessed by their name and their index. Basically
 *            an ordered hashmap.  It is implemented as intelligent mapper around
 *            a Lua table.
 *               ort[ i    ] = key
 *               ort[ key  ] = value
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>               // memset

#include "t.h"
#include "t_ort.h"


/**--------------------------------------------------------------------------
 * Create a new t.OrderedTable and return it.
 * \param   L  The lua state.
 * \lreturn struct t_ort userdata.
 * \return  int    # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int
lt_ort_New( lua_State *L )
{
	struct t_ort __attribute__ ((unused)) *ort = t_ort_create_ud( L );
	return 1;
}


/**--------------------------------------------------------------------------
 * Construct a t.OrderedTable and return it.
 * \param   L  The lua state.
 * \lparam  CLASS table t.OrderedTable.
 * \lreturn struct t_ort userdata.
 * \return  int    # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int lt_ort__Call( lua_State *L )
{
	lua_remove( L, 1 );
	return lt_ort_New( L );
}


/**--------------------------------------------------------------------------
 * Create a new t_ort userdata and push to LuaStack.
 * \param   L  The lua state.
 * \param   int start position on stack for elements.
 * \return  struct t_ort * pointer to new userdata on Lua Stack
 * --------------------------------------------------------------------------*/
struct t_ort
*t_ort_create_ud( lua_State *L )
{
	struct t_ort    *ort;

	ort = (struct t_ort *) lua_newuserdata( L, sizeof( struct t_ort ) );
	lua_newtable( L );

	ort->tR = luaL_ref( L, LUA_REGISTRYINDEX );
	luaL_getmetatable( L, "T.OrderedTable" );
	lua_setmetatable( L, -2 );
	return ort;
}


/**--------------------------------------------------------------------------
 * Check a value on the stack for being a struct t_ort
 * \param   L    The lua state.
 * \param   int      position on the stack
 * \param   int      check(boolean): if true error out on fail
 * \return  struct t_ort*  pointer to userdata on stack
 * --------------------------------------------------------------------------*/
struct t_ort
*t_ort_check_ud ( lua_State *L, int pos, int check )
{
	void *ud = luaL_checkudata( L, pos, "T.OrderedTable" );
	luaL_argcheck( L, (ud != NULL  || !check), pos, "`T.OrderedTable` expected" );
	return (NULL==ud) ? NULL : (struct t_ort *) ud;
}


/**--------------------------------------------------------------------------
 * Read an OrderedTable value.
 * \param   L    The lua state.
 * \lparam  userdata T.OrderedTable instance.
 * \lparam  key      string/integer.
 * \lreturn userdata T.OrderedTable instance.
 * \return  int    # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int
lt_ort__index( lua_State *L )
{
	struct t_ort *ort = t_ort_check_ud( L, -2, 1 );

	lua_rawgeti( L, LUA_REGISTRYINDEX, ort->tR );

	if (LUA_TNUMBER == lua_type( L, -2 ) )
	{
		lua_rawgeti( L, -1, luaL_checkinteger( L, -2) );
		lua_rawget( L, -2 );
		return 1;
	}
	else
	{
		luaL_checkstring( L, -2 );
		lua_pushvalue( L, -2 );
		lua_rawget( L, -2 );
		return 1;
	}
}


/**--------------------------------------------------------------------------
 * Write an OrderedTable value.
 * \param   L    The lua state.
 * \lparam  userdata T.OrderedTable instance.
 * \lparam  key      string/integer.
 * \lparam  value    Any value.
 * \lreturn userdata T.OrderedTable instance.
 * \return  int    # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int
lt_ort__newindex( lua_State *L )
{
	const char   *key;
	int           idx;
	size_t        len;
	struct t_ort *ort = t_ort_check_ud( L, -3, 1 );

	lua_rawgeti( L, LUA_REGISTRYINDEX, ort->tR ); // S: ort,key/id,val,rft
	len = lua_rawlen( L, -1 );

	if (LUA_TNUMBER == lua_type( L, -3 ) )
	{
		idx = luaL_checkinteger( L, -3 );
		luaL_argcheck( L, 1 <= idx && idx <= (int) len, 2,
			"Index must be greater than 0 and lesser than table length" );
		lua_rawgeti( L, -1, idx );     // S: ort,id,val,rft,key
		lua_pushvalue( L, -3 );        // S: ort,id,val,rft,key,val
		t_stackdump( L );
		lua_rawset( L, -3 );
		return 1;
	}
	else
	{
		// S: ort,key,val,rft
		key = luaL_checkstring( L, -3 );
		lua_pushvalue( L, -3 );
		lua_rawget( L, -2 );
		if (lua_isnoneornil( L, -1 ))    // insert new
		{
			lua_pop( L, 1 );
			lua_pushvalue( L, -3 );       // S:ort,key,val,rft,key
			lua_rawseti( L, -2, len+1 );  // S:ort,key,val,rft
			lua_pushvalue( L, -3 );
			lua_pushvalue( L, -3 );       // S:ort,key,val,rft,key,val
			lua_rawset( L, -3 );
		}
		else
		{
			lua_pushvalue( L, -3 ); // S: ort,key,val,rft,key,val
			t_stackdump( L );
			lua_rawset( L, -3 );
		}
		t_stackdump( L );
		lua_pop( L, 1 );
		return 0;
	}
}

///**--------------------------------------------------------------------------
// * the actual iterate(next) over the T.OrderedTable.
// * It will return key,value pairs in proper order as defined in the constructor.
// * \param   L lua Virtual Machine.
// * \lparam  cfunction.
// * \lparam  previous key.
// * \lparam  current key.
// * \lreturn current key, current value.
// * \return  int    # of values pushed onto the stack.
// *  -------------------------------------------------------------------------*/
//static int
//t_ort_iter( lua_State *L )
//{
//	struct t_ort *ort  = t_ort_check_ud( L, lua_upvalueindex( 1 ), 1 );
//
//	// get current index and increment
//	int crs = lua_tointeger( L, lua_upvalueindex( 2 ) ) + 1;
//
//	lua_rawgeti( L, LUA_REGISTRYINDEX, ort->tR );
//
//	if (crs > lua_rawlen( L, -1 ))
//		return 0;
//	else
//	{
//		lua_pushinteger( L, crs );
//		lua_replace( L, lua_upvalueindex( 2 ) );
//	}
//	lua_rawgeti( L, LUA_REGISTRYINDEX, pc->m );// Stack: func,nP,_idx
//	if (T_PCK_STR == pc->t)                        // Get the name for a Struct value
//		lua_rawgeti( L, -1 , crs + pc->s*2 );   // Stack: func,nP,_idx,nC
//	else
//		lua_pushinteger( L, crs );     // Stack: func,iP,_idx,iC
//	r = (struct t_pcr *) lua_newuserdata( L, sizeof( struct t_pcr ));
//	lua_rawgeti( L, -3 , crs+pc->s ); // Stack: func,xP,_idx,xC,Rd,ofs
//	lua_rawgeti( L, -4 , crs );       // Stack: func,xP,_idx,xC,Rd,ofs,pack
//	lua_remove( L, -5 );              // Stack: func,xP,xC,Rd,ofs,pack
//
//	r->r = luaL_ref( L, LUA_REGISTRYINDEX );   // Stack: func,xP,xC,Rd,ofs
//	r->o = lua_tointeger( L, lua_upvalueindex( 3 ) ) + luaL_checkinteger( L, -1 );
//	lua_pop( L, 1 );                  // remove ofs
//	luaL_getmetatable( L, "T.Pack.Reader" );
//	lua_setmetatable( L, -2 );
//
//	return 2;
//}
//
//
///**--------------------------------------------------------------------------
// * Pairs method to iterate over the T.Pack.Struct.
// * \param   L lua Virtual Machine.
// * \lparam  iterator T.Pack.Struct.
// * \lreturn pos    position in t_buf.
// * \return  int    # of values pushed onto the stack.
// *  -------------------------------------------------------------------------*/
//static int
//lt_pck__pairs( lua_State *L )
//{
//	struct t_pcr *pr = NULL;
//	t_pck_getpckreader( L, -1, &pr );
//
//	lua_pushnumber( L, 0 );
//	lua_pushinteger( L, (NULL == pr) ? 0 : pr->o );  // preserve offset for iteration
//	lua_pushcclosure( L, &t_pck_iter, 3 );
//	lua_pushvalue( L, -1 );
//	lua_pushnil( L );
//	return 3;
//}


/**--------------------------------------------------------------------------
 * Returns len of the ordrred table
 * \param   L    The Lua state
 * \return  int  # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int
lt_ort__len( lua_State *L )
{
	struct t_ort *ort = t_ort_check_ud( L, 1, 1 );

	lua_rawgeti( L, LUA_REGISTRYINDEX, ort->tR );
	lua_pushinteger( L, lua_rawlen( L, -1 ) );
	lua_remove( L, -2 );
	return 1;
}


/**--------------------------------------------------------------------------
 * Return Tostring representation of a ordered table stream.
 * \param   L     The lua state.
 * \lreturn string    formatted string representing ordered table.
 * \return  int  # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
static int
lt_ort__tostring( lua_State *L )
{
	struct t_ort *ort = t_ort_check_ud( L, 1, 1 );

	lua_rawgeti( L, LUA_REGISTRYINDEX, ort->tR );
	lua_pushfstring( L, "T.OrderedTable[%d]: %p", lua_rawlen( L, -1 ), ort );
	lua_remove( L, -2 );
	return 1;
}


/**--------------------------------------------------------------------------
 * Class metamethods library definition
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_ort_fm [] = {
	{"__call",        lt_ort__Call},
	{NULL,            NULL}
};

/**--------------------------------------------------------------------------
 * Class functions library definition
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_ort_cf [] = {
	{"new",           lt_ort_New},
	{NULL,            NULL}
};

/**--------------------------------------------------------------------------
 * Objects metamethods library definition
 * --------------------------------------------------------------------------*/
static const luaL_Reg t_ort_m [] = {
	{ "__tostring", lt_ort__tostring },
	{ "__len",      lt_ort__len },
	{ "__index",    lt_ort__index },
	{ "__newindex", lt_ort__newindex },
	{NULL,    NULL}
};


/**--------------------------------------------------------------------------
 * Pushes the T.OrderedTable library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   L      The lua state.
 * \lreturn table  the library
 * \return  int    # of values pushed onto the stack.
 * --------------------------------------------------------------------------*/
LUAMOD_API int
luaopen_t_ort( lua_State *L )
{
	// T.Pack.Struct instance metatable
	luaL_newmetatable( L, "T.OrderedTable" );   // stack: functions meta
	luaL_setfuncs( L, t_ort_m, 0 );
	lua_pop( L, 1 );        // remove metatable from stack

	// Push the class onto the stack
	// this is avalable as T.Pack.<member>
	luaL_newlib( L, t_ort_cf );
	luaL_newlib( L, t_ort_fm );
	lua_setmetatable( L, -2 );
	return 1;
}
