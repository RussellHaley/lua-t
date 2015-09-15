#!../out/bin/lua
local t=require('t')
ipAddr,port = '10.128.3.145', 8888 

local s,ip = t.Net.UDP.bind( ipAddr, port )
print( s,ip )

-- this select loop makes no sense, but prooves that select is in fact working
-- as expected
while true do
	res = t.Net.select({s},{})
	print(#res, s)
	msg, len, ip = res[1]:recvfrom()
	if len<1 then
		break
	else
		print( msg, len, ip )
		if 'exit' == msg:sub( 1,4 ) then
			break
		end
	end
end
s:close( )

