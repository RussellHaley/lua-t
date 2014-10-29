/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * l_xt_hndl.h
 * socket functions wrapped for lua
 *
 * data definitions
 */
#ifdef _WIN32
#include <WinSock2.h>
#include <winsock.h>
#include <time.h>
#include <stdint.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif
#define MAX_BUF_SIZE      4096

enum xt_sck_t {
	XT_SCK_UDP,
	XT_SCK_TCP,
};

static const char *const xt_sck_t_lst[] = {
	"UDP",
	"TCP",
	NULL
};

struct xt_sck {
	enum xt_sck_t    t;
	int              fd;    ///< socket handle
};

// Constructors
// l_xt_ipendpoint.c
struct sockaddr_in *check_ud_ipendpoint (lua_State *luaVM, int pos);
struct sockaddr_in *create_ud_ipendpoint (lua_State *luaVM);
int    set_ipendpoint_values (lua_State *luaVM, int pos, struct sockaddr_in *ip);

// l_xt_socket.c
int            luaopen_xt_sck    ( lua_State *luaVM );
int            lxt_sck_New       ( lua_State *luaVM );
struct xt_sck *xt_sck_check_ud   ( lua_State *luaVM, int pos );
struct xt_sck *xt_sck_create_ud  ( lua_State *luaVM, enum xt_sck_t type);
