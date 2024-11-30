#!/usr/bin/python

import os
from http.cookies import SimpleCookie

def increment():
    """
    Retrieves cookie, either initializes counter,
    or increments the counter by one.
    """
    if 'HTTP_COOKIE' in os.environ:
        cnt = SimpleCookie(os.environ['HTTP_COOKIE'])
    else:
        cnt = SimpleCookie()
    if 'visits' not in cnt:
        cnt['visits'] = 0
    else:
        cnt['visits'] = str(1 + int(cnt['visits'].value))
    return cnt

def main():
    """
    Retrieves a cookie and writes
    the value of counter to the page,
    after incrementing the counter.
    """
    counter = increment()

    print(counter, end='\r\n')
    print("Content-Type: text/plain\r\n", end='\r\n')
    print("counter: %s\r\n" % counter['visits'].value)

main()
