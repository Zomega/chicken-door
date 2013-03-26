from datetime import *

now = datetime.now()

def padInt( i ):
	if i < 10:
		return "0" + str( i )
	return str( i )

# Time info...
hour = now.hour
minute = now.minute
second = now.second

# Date info...
year = now.year % 100
month = now.month
dayOfMonth = now.day
dayOfWeek = now.weekday()+2
if dayOfWeek == 8:
	dayOfWeek == 1

print "t" + padInt( second ) + padInt( minute ) + padInt( hour ) + str( dayOfWeek ) + padInt( dayOfMonth ) + padInt( month ) + padInt( year )
