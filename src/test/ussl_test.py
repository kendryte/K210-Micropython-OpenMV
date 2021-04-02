import usocket as _socket
import ussl as ssl

website="www.baidu.com"
s = _socket.socket()
ai = _socket.getaddrinfo(website, 443)
print("Address infos:", ai)
addr = ai[0][-1]

print("Connect address:", addr)
s.connect(addr)

s = ssl.wrap_socket(s)
print(s)