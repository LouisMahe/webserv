#!/usr/bin/python
import cgi, cgitb
form = cgi.FieldStorage()

if form.getvalue('color'):
	color = form.getvalue('color')
else:
	color = 'red'
print("Set-Cookie: color=%s" % color, end='\r\n')
print("Location: http://localhost:8080/color.py\r\n", end='\r\n')

