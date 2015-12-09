t = require( 't' )
o = t.OrderedTable( )
u = { }

o.one   = 'alpha'
o.two   = 'beta'
o.three = 'gamma'


u.one   = 'alpha'
u.two   = 'beta'
u.three = 'gamma'

print( "ordered:" )
for k,v in pairs( o ) do
	print( k,v )
end

print( "\nunordered:" )
for k,v in pairs( u ) do
	print( k,v )
end

o[2]     = 'Still in beta - modified by numeric index'
o.three  = 'More gamma here - modified by hashed index'
o.four   = 'Added some delta - auto indexed as four'

print( "\nordered(manipulated):" )
for k,v in pairs( o ) do
	print( k,v )
end
