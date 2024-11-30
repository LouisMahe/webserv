import cgi, cgitb

form = cgi.FieldStorage()

print("Location:http://localhost:8080/upload_file.html")
print("content-type: text/html\r\n")

print("<html>")
print("<head>")
print("<title> MY FIRST CGI FILE </title>")
print("</head>")
print("<body>")
print("<h3> This is HTML's Body Section </h3>")
print("</body>")
print("</html>")
