/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t_buf.c
 * \brief     OOP wrapper for a universal buffer implementation
 *            Allows for reading writing as mutable string
 *            can be used for socket communication
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>               // memset

#include "t.h"
#include "t_buf.h"


// --------------------------------- HELPERS Functions

/** -------------------------------------------------------------------------
 * Helper to check arguments for being a t_buf and a valid position.
 * \param     luaVM lua state.
 * \param     pB    position on stack which is buffer.
 * \param     pB    position on stack which is position (buffer index).
 *                  handled as pointer and decremented by one to deal with the
 *                  fact that C char buffers indexes are zero based.
 * \return   *t_buf pointer to validated buffer.
 *  -------------------------------------------------------------------------*/

struct t_buf * t_buf_getbuffer( lua_State *luaVM, int pB, int pP, int *pos )
{
	struct t_buf *buf = t_buf_check_ud( luaVM, pB, 1 );

	*pos = (lua_isnone( luaVM, pP ))
		? 1
		: luaL_checkinteger( luaVM, pP );   ///< starting byte  b->b[pos]

	luaL_argcheck( luaVM,  1 <= *pos && *pos <= (int) buf->len, pP,
		                 "T.Buffer position must be >= 1 or <= #buffer" );
	*pos = *pos-1;     /// account for char array access being 0 based
	return buf;
}



/////////////////////////////////////////////////////////////////////////////
//  _                        _    ____ ___
// | |   _   _  __ _        / \  |  _ \_ _|
// | |  | | | |/ _` |_____ / _ \ | |_) | |
// | |__| |_| | (_| |_____/ ___ \|  __/| |
// |_____\__,_|\__,_|    /_/   \_\_|  |___|
/////////////////////////////////////////////////////////////////////////////
/** -------------------------------------------------------------------------
 * creates the buffer from the function call.
 * \param     luaVM  lua state.
 * \lparam    length of buffer.
 *        ALTERNATIVE
 * \lparam    string buffer content initialized.
 * \return    integer   how many elements are placed on the Lua stack.
 *  -------------------------------------------------------------------------*/
static int lt_buf_New( lua_State *luaVM )
{
	size_t                                  sz;
	struct t_buf  __attribute__ ((unused)) *buf;

	if (lua_isnumber( luaVM, 1))
	{
		sz  = luaL_checkinteger( luaVM, 1 );
		buf = t_buf_create_ud( luaVM, sz );
	}
	else if (lua_isstring( luaVM, 1 ))
	{
		luaL_checklstring( luaVM, 1, &sz);
		buf = t_buf_create_ud( luaVM, sz );
		memcpy( (char*) &(buf->b[0]), luaL_checklstring( luaVM, 1, NULL ), sz );
	}
	else
	{
		t_push_error( luaVM, "can't create T.Buffer because of wrong argument type" );
	}
	return 1;
}


/** -------------------------------------------------------------------------
 * creates the buffer from the Constructor.
 * \param     luaVM  lua state.
 * \lparam    CLASS table Time.
 * \lparam    length of buffer.
 * \lparam    string buffer content initialized.            OPTIONAL
 * \return    integer   how many elements are placed on the Lua stack.
 *  -------------------------------------------------------------------------*/
static int lt_buf__Call (lua_State *luaVM)
{
	lua_remove( luaVM, 1 );    // remove the T.Buffer Class table
	return lt_buf_New( luaVM );
}


/**--------------------------------------------------------------------------
 * create a t_buf and push to LuaStack.
 * \param   luaVM  The lua state.
 *
 * \return  struct t_buf*  pointer to the  t_buf struct
 * --------------------------------------------------------------------------*/
struct t_buf *t_buf_create_ud( lua_State *luaVM, int size )
{
	struct t_buf  *b;
	size_t          sz;

	// size = sizof(...) -1 because the array has already one member
	sz = sizeof( struct t_buf ) + (size - 1) * sizeof( unsigned char );
	b  = (struct t_buf *) lua_newuserdata( luaVM, sz );
	memset( b->b, 0, size * sizeof( unsigned char ) );

	b->len = size;
	luaL_getmetatable( luaVM, "T.Buffer" );
	lua_setmetatable( luaVM, -2 );
	return b;
}


/**--------------------------------------------------------------------------
 * Check if the item on stack position pos is an t_buf struct and return it
 * \param  luaVM    the Lua State
 * \param  pos      position on the stack
 *
 * \return struct t_buf* pointer to t_buf struct
 * --------------------------------------------------------------------------*/
struct t_buf *t_buf_check_ud( lua_State *luaVM, int pos, int check )
{
	void *ud = luaL_testudata( luaVM, pos, "T.Buffer" );
	luaL_argcheck( luaVM, (ud != NULL || !check), pos, "`T.Buffer` expected" );
	return (NULL==ud) ? NULL : (struct t_buf *) ud;
}


//////////////////////////////////////////////////////////////////////////////////////
//
// ================================= GENERIC LUA API========================

/**--------------------------------------------------------------------------
 * reads a value, unpacks it and pushes it onto the Lua stack.
 * By creating a reader instead of just passing the Packer an offset can be 
 * passed on to the __call method.
 * \param   luaVM  lua Virtual Machine.
 * \lparam  struct t_pack.
 * \lreturn value  unpacked value according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int lt_buf_unpack( lua_State *luaVM )
{
	int           pos;                               ///< starting byte  b->b[pos]
	struct t_pcr *pr;
	struct t_pck *pc;
	int           n,j;

	t_buf_getbuffer( luaVM, 1 , 3, &pos );
	lua_pushcfunction( luaVM, lt_pcr__call );
	pr = (struct t_pcr *) lua_newuserdata( luaVM, sizeof( struct t_pcr ));
	pc =  t_pck_getpck( luaVM, 2, &(pr->o) );
	pr->o = pos;
	lua_pushvalue( luaVM, 2 );             //Stack: buf,pack,ofs?,__call,rd,pack,buf
	pr->r = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	luaL_getmetatable( luaVM, "T.Pack.Reader" );
	lua_setmetatable( luaVM, -2 );
	lua_pushvalue( luaVM, 1 );             //Stack: buf,pack,ofs?,__call,rd,buf
	lua_call( luaVM, 2, 1 );

	if (T_PCK_SEQ == pc->t)
	{
		n = lua_rawlen( luaVM, -1 );
		for (j=1; j<=n; j++)
		{
			lua_rawgeti( luaVM, 0-j, j );
		}
		return n;
	}
	else
		return 1;
}



/**--------------------------------------------------------------------------
 * Read a set of chars to the buffer at position x.
 * \lparam  buf  userdata of type T.Buffer (struct t_buf).
 * \lparam  pos  position in bytes.
 * \lparam  sz   size in bytes(1-#buf).
 * \lreturn val  lua_String.
 * TODO: check buffer length vs requested size and offset
 *
 * \return integer 1 left on the stack.
 * --------------------------------------------------------------------------*/
static int lt_buf_read (lua_State *luaVM)
{
	struct t_buf *buf = t_buf_check_ud( luaVM, 1, 1 );
	int           pos;
	int           sz;

	pos = (lua_isnumber(luaVM, 2)) ? (size_t) luaL_checkinteger( luaVM, 2 ) : 0;
	sz  = (lua_isnumber(luaVM, 3)) ? (size_t) luaL_checkinteger( luaVM, 3 ) : buf->len - pos;

	lua_pushlstring(luaVM, (const char*) &(buf->b[ pos ]), sz );
	return 1;
}


/**--------------------------------------------------------------------------
 * Write an set of chars to the buffer at position x.
 * \lparam  buf  userdata of type T.Buffer (struct t_buf).
 * \lparam  val  lua_String.
 * \lparam  pos  position in bytes.
 * \lparam  sz   size in bits (1-64).
 * TODO: check string vs buffer length
 *
 * \return integer 1 left on the stack.
 * --------------------------------------------------------------------------*/
static int lt_buf_write (lua_State *luaVM)
{
	struct t_buf *buf = t_buf_check_ud( luaVM, 1, 1 );
	int           pos = (lua_isnumber(luaVM, 3)) ? luaL_checkinteger(luaVM, 3) : 0;
	size_t        sz;

	// if a third parameter is given write only x bytes of the input string to the buffer
	if (lua_isnumber(luaVM, 4))
	{
		sz  = luaL_checkinteger(luaVM, 4);
		memcpy  ( (char*) &(buf->b[ pos ]), luaL_checklstring( luaVM, 2, NULL ), sz );
	}
	// otherwise write the whole thing
	else
	{
		memcpy  ( (char*) &(buf->b[ pos ]), luaL_checklstring( luaVM, 2, &sz ), sz );
	}
	return 0;
}


/**--------------------------------------------------------------------------
 * \brief    gets the content of the Stream in Hex
 * \lreturn  string buffer representation in Hexadecimal
 * \TODO     use luaL_Buffer?
 *
 * \return integer 0 left on the stack
 * --------------------------------------------------------------------------*/
static int lt_buf_tohexstring (lua_State *luaVM)
{
	int           l,c;
	char         *sbuf;
	struct t_buf *buf = t_buf_check_ud( luaVM, 1, 1 );

	sbuf = malloc( 3 * buf->len * sizeof( char ) );
	memset( sbuf, 0, 3 * buf->len * sizeof( char ) );

	c = 0;
	for (l=0; l < (int) buf->len; l++) {
		c += snprintf( sbuf+c, 4, "%02X ", buf->b[l] );
	}
	lua_pushstring( luaVM, sbuf );
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief     returns len of the buffer
 * \param     lua state
 * \return    integer   how many elements are placed on the Lua stack
 * --------------------------------------------------------------------------*/
static int lt_buf__len (lua_State *luaVM)
{
	struct t_buf *buf = t_buf_check_ud( luaVM, 1, 1 );

	lua_pushinteger( luaVM, (int) buf->len );
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   tostring representation of a buffer stream.
 * \param   luaVM     The lua state.
 * \lparam  sockaddr  the buffer-Stream in user_data.
 * \lreturn string    formatted string representing buffer.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int lt_buf__tostring (lua_State *luaVM)
{
	struct t_buf *buf = t_buf_check_ud( luaVM, 1, 1 );
	lua_pushfstring( luaVM, "T.Buffer{%d}: %p", buf->len, buf );
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_buf_fm [] = {
	{"__call",        lt_buf__Call},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_buf_cf [] = {
	{"new",           lt_buf_New},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief      the buffer library definition
 *             assigns Lua available names to C-functions
 * --------------------------------------------------------------------------*/
static const luaL_Reg t_buf_m [] = {
	{"unpack",   lt_buf_unpack},
	{"read",     lt_buf_read},
	{"write",    lt_buf_write},
	// univeral stuff
	{"toHex",    lt_buf_tohexstring},
	{"length",   lt_buf__len},
	{"toString", lt_buf__tostring},
	{NULL,    NULL}
};


/**--------------------------------------------------------------------------
 * \brief   pushes the BufferBuffer library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   luaVM     The lua state.
 * \lreturn string    the library
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
LUAMOD_API int luaopen_t_buf (lua_State *luaVM)
{
	// T.Buffer instance metatable
	luaL_newmetatable( luaVM, "T.Buffer" );
	luaL_newlib( luaVM, t_buf_m );
	lua_setfield( luaVM, -2, "__index" );
	lua_pushcfunction( luaVM, lt_buf__len );
	lua_setfield( luaVM, -2, "__len");
	lua_pushcfunction( luaVM, lt_buf__tostring );
	lua_setfield( luaVM, -2, "__tostring");
	lua_pop( luaVM, 1 );        // remove metatable from stack
	// T.Buffer class
	luaL_newlib( luaVM, t_buf_cf );
	luaL_newlib( luaVM, t_buf_fm );
	lua_setmetatable( luaVM, -2 );
	return 1;
}


