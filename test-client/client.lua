local socket = require("socket")

local c = socket.connect("localhost", 5002)

assert(type(c) ~= "nil", "socket is null")

c:send("{\"type\": 50}");

local l, e = c:receive("*a")
while not e do
	print(l)
	l, e = c:receive("*a")
end
print(e)