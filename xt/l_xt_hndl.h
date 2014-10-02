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

enum xt_hndl_t {
	UDP,
	TCP,
	TIME,
	FIL,
};

static const char *const xt_hndl_t_lst[] = {
	"UDP",
	"TCP",
	"TIME",
	"FIL",
	NULL
};

struct xt_hndl {
	enum xt_hndl_t      hd_t;
	int                 fd;    // socket, file handle, stream ...
	//t_timeout tm;
};

// Constructors
// l_xt_ipendpoint.c
struct sockaddr_in *check_ud_ipendpoint (lua_State *luaVM, int pos);
struct sockaddr_in *create_ud_ipendpoint (lua_State *luaVM);
int    set_ipendpoint_values (lua_State *luaVM, int pos, struct sockaddr_in *ip);

// l_xt_socket.c
struct xt_hndl *check_ud_socket (lua_State *luaVM, int pos);
struct xt_hndl *xt_socket_create_ud( lua_State *luaVM );
int             xt_socket_set_type( lua_State *luaVM, struct xt_hndl *hndl, enum xt_hndl_t );


