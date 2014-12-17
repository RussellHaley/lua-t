/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t_htpsrv.h
 * \brief     OOP wrapper for HTTP operation
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */


#include <string.h>               // memset

#include "t.h"
#include "t_htp.h"
#include "t_buf.h"


/** ---------------------------------------------------------------------------
 * Creates an T.Http.Server.
 * \param    luaVM    lua state.
 * \lparam   function WSAPI style request handler.
 * \return integer # of elements pushed to stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htpsrv_New( lua_State *luaVM )
{
	struct t_htpsrv  *s;
	struct t_elp     *l;

	if (lua_isfunction( luaVM, -1 ) && (l = t_elp_check_ud( luaVM, -2, 1 )))
	{
		s     = t_htpsrv_create_ud( luaVM );
		s->rR = luaL_ref( luaVM, LUA_REGISTRYINDEX );
		s->lR = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	}
	else
		return t_push_error( luaVM, "T.Http.Server( func ) requires a function as parameter" );
	return 1;
}


/**--------------------------------------------------------------------------
 * construct an HTTP Server
 * \param   luaVM    The lua state.
 * \lparam  CLASS    table Http.Server
 * \lparam  T.Socket sub protocol
 * \lreturn userdata struct t_htpsrv* ref.
 * \return  int    # of elements pushed to stack.
 * --------------------------------------------------------------------------*/
static int lt_htpsrv__Call( lua_State *luaVM )
{
	lua_remove( luaVM, 1 );
	return lt_htpsrv_New( luaVM );
}


/**--------------------------------------------------------------------------
 * create a t_htpsrv and push to LuaStack.
 * \param   luaVM  The lua state.
 *
 * \return  struct t_htpsrv*  pointer to the struct.
 * --------------------------------------------------------------------------*/
struct t_htpsrv
*t_htpsrv_create_ud( lua_State *luaVM )
{
	struct t_htpsrv  *s;
	s = (struct t_htpsrv *) lua_newuserdata( luaVM, sizeof( struct t_htpsrv ));

	luaL_getmetatable( luaVM, "T.Http.Server" );
	lua_setmetatable( luaVM, -2 );
	return s;
}


/**--------------------------------------------------------------------------
 * Check if the item on stack position pos is an t_htpsrv struct and return it
 * \param  luaVM    the Lua State
 * \param  pos      position on the stack
 *
 * \return  struct t_htpsrv*  pointer to the struct.
 * --------------------------------------------------------------------------*/
struct t_htpsrv
*t_htpsrv_check_ud( lua_State *luaVM, int pos, int check )
{
	void *ud = luaL_checkudata( luaVM, pos, "T.Http.Server" );
	luaL_argcheck( luaVM, (ud != NULL || !check), pos, "`T.Http.Server` expected" );
	return (struct t_htpsrv *) ud;
}


/**--------------------------------------------------------------------------
 * Accept a connection from a Http.Server listener.
 * Called anytime a new connection gets established.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htpsrv.
 * \param   pointer to the buffer to read from(already positioned).
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htpsrv_accept( lua_State *luaVM )
{
	struct t_htpsrv    *s     = lua_touserdata( luaVM, 1 );
	struct sockaddr_in *si_cli;
	struct t_sck       *c_sck;
	struct t_sck       *s_sck;
	struct t_htpcon    *c;      // new connection userdata

	lua_remove( luaVM, 1 );                 // remove http server instance
	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, s->sR );
	s_sck = t_sck_check_ud( luaVM, -1, 1 );

	lt_sck_accept( luaVM );  //S: ssck,csck,cip
	c_sck  = t_sck_check_ud( luaVM, -2, 1 );
	si_cli = t_ipx_check_ud( luaVM, -1, 1 );
	c      = (struct t_htpcon *) lua_newuserdata( luaVM, sizeof( struct t_htpcon ) );
	c->aR  = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	c->sR  = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	c->rR  = s->rR;                // copy function reference
	c->fd  = c_sck->fd;

	luaL_getmetatable( luaVM, "T.Http.Connection" );
	lua_setmetatable( luaVM, -2 );
	return 1;
}


/**--------------------------------------------------------------------------
 * Reads a chunk from socket.  Called anytime socket returns from read.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htpsrv.
 * \param   pointer to the buffer to read from(already positioned).
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
t_htpcon_read( lua_State *luaVM )
{
	struct t_htpcon *c = (struct t_htpcon *) luaL_checkudata( luaVM, 1, "T.http.Connection" );
	char             buffer[ BUFSIZ ];
	int              rcvd;
	struct t_sck    *c_sck;

	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, c->sR );
	c_sck = t_sck_check_ud( luaVM, -1, 1 );

	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, c->rR );
	// TODO: Idea
	// WS is in a state -> empty, receiving, sending
	// negotiate to read into the buffer initially or into the luaL_Buffer
	rcvd = t_sck_recv_tdp( luaVM, c_sck, &(buffer[ 0 ]), BUFSIZ );
	return rcvd;
}


/**--------------------------------------------------------------------------
 * Puts the http server on a T.Loop to listen to incoming requests.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htpsrv.
 * \param   pointer to the buffer to read from(already positioned).
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htpsrv_listen( lua_State *luaVM )
{
	struct t_htpsrv    *s   = t_htpsrv_check_ud( luaVM, 1, 1 );
	struct t_elp       *elp;
	struct t_sck       *sc  = NULL;
	struct sockaddr_in *ip  = NULL;

	// reuse socket:bind()
	lua_remove( luaVM, 1 );    // remove http server instance from stack
	lt_sck_listen( luaVM );

	sc = t_sck_check_ud( luaVM, -2, 1 );
	ip = t_ipx_check_ud( luaVM, -1, 1 );

	lua_pushcfunction( luaVM, lt_elp_addhandle ); //S: sc,ip,addhandle
	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, s->lR );
	elp= t_elp_check_ud( luaVM, -1, 1 );
	lua_pushvalue( luaVM, -4 );                  /// push socket
	lua_pushboolean( luaVM, 1 );                 /// yepp, that's for reading
	lua_pushcfunction( luaVM, lt_htpsrv_accept );

	lua_call( luaVM, 4, 0 );
	return  0;
}


/**--------------------------------------------------------------------------
 * __tostring (print) representation of a packer instance.
 * \param   luaVM      The lua state.
 * \lparam  xt_pack    the packer instance user_data.
 * \lreturn string     formatted string representing packer.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int lt_htpsrv__tostring( lua_State *luaVM )
{
	struct t_htpsrv *s = t_htpsrv_check_ud( luaVM, 1, 1 );

	lua_pushfstring( luaVM, "T.Http.Server: %p", s );
	return 1;
}


/**--------------------------------------------------------------------------
 * __len (#) representation of an instance.
 * \param   luaVM      The lua state.
 * \lparam  userdata   the instance user_data.
 * \lreturn string     formatted string representing the instance.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int lt_htpsrv__len( lua_State *luaVM )
{
	//struct t_wsk *wsk = t_wsk_check_ud( luaVM, 1, 1 );
	//TODO: something meaningful here?
	lua_pushinteger( luaVM, 4 );
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_htpsrv_fm [] = {
	{"__call",        lt_htpsrv__Call},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_htpsrv_cf [] = {
	{"new",           lt_htpsrv_New},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief      the buffer library definition
 *             assigns Lua available names to C-functions
 * --------------------------------------------------------------------------*/
static const luaL_Reg t_htpsrv_m [] = {
	{NULL,    NULL}
};


/**--------------------------------------------------------------------------
 * \brief   pushes this library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   luaVM     The lua state.
 * \lreturn string    the library
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
LUAMOD_API int luaopen_t_htpsrv (lua_State *luaVM)
{
	// T.Http.Server instance metatable
	luaL_newmetatable( luaVM, "T.Http.Server" );
	luaL_newlib( luaVM, t_htpsrv_m );
	lua_setfield( luaVM, -2, "__index" );
	lua_pushcfunction( luaVM, lt_htpsrv__len );
	lua_setfield( luaVM, -2, "__len");
	lua_pushcfunction( luaVM, lt_htpsrv__tostring );
	lua_setfield( luaVM, -2, "__tostring");
	lua_pop( luaVM, 1 );        // remove metatable from stack
	// T.Websocket class
	luaL_newlib( luaVM, t_htpsrv_cf );
	luaL_newlib( luaVM, t_htpsrv_fm );
	lua_setmetatable( luaVM, -2 );
	return 1;
}

