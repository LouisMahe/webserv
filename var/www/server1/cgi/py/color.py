import os
from http.cookies import SimpleCookie



def choseColor(plate):
	print("Location: /chosecolor.html\r\n", end='\r\n')

def makePage(plate):
	style = plate['color'].value
	print(plate, end='\r\n')
	print("Content-Type: text/html\r\n", end='\r\n')
	print("<html><head><title> Colors </title></head><body><style>")
	if (style == 'red'):
		print("#one {color:darkred;text-align: center;font-size:150%;}")
	elif (style == 'blue'):
		print("#one {color:cornflowerblue;text-align: center;font-size:150%;}")
	else:
		print("#one {color:green;text-align: center;font-size:150%;}")
	print("</style><p id=\"one\">")
	print("I love sockets !")
	print("</p></body></html>")



def main():

	if 'HTTP_COOKIE' in os.environ:
		plate = SimpleCookie(os.environ['HTTP_COOKIE'])
	else:
		plate = SimpleCookie()
	if 'color' not in plate:
		plate = choseColor(plate)
	else:
		makePage(plate)


main()
