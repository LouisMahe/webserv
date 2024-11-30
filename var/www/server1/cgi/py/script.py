# Import modules for CGI handling
import cgi, cgitb
import cgitb
cgitb.enable()

# Create instance of FieldStorage
form = cgi.FieldStorage()

print("Content-type:text/html\r\n\r\n")
# Get data from fieldse
for key in form.keys():
	print(form[key])

print("<html>")
print("<head>")
print("<title>Checkbox - Third CGI Program</title>")
print("</head>")
print("<body>")
print("<h2> CheckBox Maths is :</h2>")
print("<h2> CheckBox Physics is :</h2>")
print("</body>")
print("</html>")
