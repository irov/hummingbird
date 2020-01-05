import hummingbird
    
login = hummingbird.make_uuid()
password = "testaccount"
    
print("----newaccount---- login: {0} password: {1}".format(login, password))
jnewaccount = hummingbird.post("http://localhost:5555/newaccount", login = login, password = password)
print("response: ", jnewaccount)

if jnewaccount["code"] != 0:
    sys.exit(0)
    
print("----loginaccount---- login: {0} password: {1}".format(login, password))
jloginaccount = hummingbird.post("http://localhost:5555/loginaccount", login = login, password = password)
print("response: ", jloginaccount)

if jloginaccount["code"] != 0:
    sys.exit(0)    
    
token = jloginaccount["token"]
   
print("----newproject---- token: {0}".format(token))
jnewproject = hummingbird.post("http://localhost:5555/newproject/{0}".format(token))
print("response: ", jnewproject)

if jnewproject["code"] != 0:
    sys.exit(0)

pid = jnewproject["pid"]

print("----upload---- token: {0} pid: {1}".format(token, pid))
jupload = hummingbird.upload("http://localhost:5555/upload/{0}/{1}".format(token, pid), "server.lua")
print("response: ", jupload)

if jupload["code"] != 0:
    sys.exit(0)
    
token = jloginaccount["token"]
method = "create"
data = dict(a=5, b=6, c="testp")
print("----command---- token: {0} method: {1} data: {2}".format(token, method, data))
japi = hummingbird.api("http://localhost:5555/command/{0}/{1}/{2}".format(token, pid, method), **data)
print("response: ", japi)

if japi["code"] != 0:
    sys.exit(0)    

login = hummingbird.make_uuid()
password = "testuser"

print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
jnewuser = hummingbird.post("http://localhost:5555/newuser/{0}".format(pid), login = login, password = password)
print("response: ", jnewuser)

if jnewuser["code"] != 0:
    sys.exit(0)

print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
jloginuser = hummingbird.post("http://localhost:5555/loginuser/{0}".format(pid), login = login, password = password)
print("response: ", jloginuser)

if jloginuser["code"] != 0:
    sys.exit(0)

token = jloginuser["token"]
method = "test"
data = dict(a=1, b=2, c="testc")
print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(token, method), **data)
print("response: ", japi)

if japi["code"] != 0:
    sys.exit(0)
    
print("done")
