/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t.h
 * \brief     general functions and globally known values
 *            data definitions
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */

#ifdef _WIN32
#include <Windows.h>
#else
#define _POSIX_SOURCE   1
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1
#endif

#include <sys/select.h>
#include <stdint.h>

/* includes the Lua headers */
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MAX_PKT_BYTES     1500

#define PRINT_DEBUGS      0

#define UNUSED(...)   (void)(__VA_ARGS__)


// http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
// http://esr.ibiblio.org/?p=5095
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)
#define IS_LITTLE_ENDIAN (1 == *(unsigned char *)&(const int){1})
//#define IS_LITTLE_ENDIAN (*(uint16_t*)"\0\1">>8)
//#define IS_BIG_ENDIAN (*(uint16_t*)"\1\0">>8)

#define TSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)

void t_stackDump ( lua_State *L );
void t_stackPrint( lua_State *L, int first, int last );
int  t_push_error( lua_State *L, const char *fmt, ... );


// global helper functions
uint16_t get_crc16( const unsigned char *data, size_t size );


// global sub classes registration
LUAMOD_API int luaopen_t_ael( lua_State *L );
LUAMOD_API int luaopen_t_tim( lua_State *L );
LUAMOD_API int luaopen_t_net( lua_State *L );
LUAMOD_API int luaopen_t_enc( lua_State *L );
LUAMOD_API int luaopen_t_buf( lua_State *L );
LUAMOD_API int luaopen_t_pck( lua_State *L );
LUAMOD_API int luaopen_t_tst( lua_State *L );
LUAMOD_API int luaopen_t_htp( lua_State *L );
LUAMOD_API int luaopen_t_wsk( lua_State *L );
LUAMOD_API int luaopen_t    ( lua_State *L );
#ifdef T_NRY
LUAMOD_API int luaopen_t_nry( lua_State *L );
#endif


/******************************************************************************
 * Copyright (C) 2012-2014, Tobias Kieslich (tkieslich). All rights reseved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
