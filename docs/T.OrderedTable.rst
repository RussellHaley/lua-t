lua-t T.OrderedTable - The Lua table with keys that are ordered
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Overview
========

T.OrderedTable is a simple wrapper around table via a metatable.  It provides a
guarantee that the table won't be sparse, that the table length is accurate and
that any operations which would destroy that are intercepted.


API
===

Class Members
-------------

none

Class Metamembers
-----------------

t.OrderedTable( [int *x*] )   [__call]
  instantiate a new t.OrderedTable object with an optional value *x* length


Instance Members
----------------

time:sleep( )
  makes process sleep for however long the instance time period is set

int *x* = time:get( )
  returns the timer instance duration in milliseconds

void time:set( int *x* )
  set the timers instance duration to *x* milliseconds

void time:since( )
  set the timers instance duration to the difference between the previous value
  and now in reference to epoch.  It interprets the current value on time as
  duration passed since epoch.  The new value of time will be the difference of
  the time since epoch to now and the original value in the timer.

void time:now( )
  set the timers instance duration to the difference between now and epoch.


Instance Metamembers
--------------------

boolean *x* = (instance1 == instance2)  [__eq]
  Compares two time duration objects for equality.  If instance1 has the same
  duration as instance2 it returns *x* as true, otherwise as false.

t.time *t* = t.time *t1* + t.time *t2*  [__add]
  Adds the duration of operand *t1* to operand *t2*.  Returns a new instance of
  *t*.

t.time *t* = t.time *t1* - t.time *t2*  [__sub]
  Substracts duration of operand *t2* from operand *t1*.  Returns a new
  instance *t*.

string *s* = tostring( timeInstance )  [__toString]
  Returns a string representing the time instance.  The String contains type,
  length and memory address information such as "T.Time[0:200000]: 0x1193d18",
  meaning it has a duration of 0 seconds and 200000 microseconds.

