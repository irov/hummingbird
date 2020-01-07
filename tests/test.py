import hummingbird
import unittest



class Testing(unittest.TestCase):
    #----------------------------------------------------------------------------------------------------------------
    # create account
    #----------------------------------------------------------------------------------------------------------------
    
    def test_0_newaccount(self):
        Testing.account_login = hummingbird.make_uuid()
        Testing.account_password = "testaccount"

        print("----newaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jnewaccount = hummingbird.post("http://localhost:5555/newaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jnewaccount)

        self.assertEqual(jnewaccount["code"], 0)
        pass
            
    def test_1_loginaccount(self):
        print("----loginaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jloginaccount = hummingbird.post("http://localhost:5555/loginaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jloginaccount)

        self.assertEqual(jloginaccount["code"], 0)
        self.assertIn("token", jloginaccount)
        
        Testing.account_token = jloginaccount["token"]
        pass
            
    #----------------------------------------------------------------------------------------------------------------
    # create project
    #----------------------------------------------------------------------------------------------------------------
            
    def test_2_newproject(self):
        print("----newproject---- token: {0}".format(Testing.account_token))
        jnewproject = hummingbird.post("http://localhost:5555/newproject/{0}".format(Testing.account_token))
        print("response: ", jnewproject)

        self.assertEqual(jnewproject["code"], 0)
        self.assertIn("pid", jnewproject)

        Testing.pid = jnewproject["pid"]
        pass

    def test_3_upload(self):
        print("----upload---- token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jupload = hummingbird.upload("http://localhost:5555/upload/{0}/{1}".format(Testing.account_token, Testing.pid), "server.lua")
        print("response: ", jupload)

        self.assertEqual(jupload["code"], 0)
        pass
    
    def test_4_command(self):    
        method = "create"
        data = dict(a=5, b=6, c="testp")
        print("----command---- token: {0} method: {1} data: {2}".format(Testing.account_token, method, data))
        jcommand = hummingbird.api("http://localhost:5555/command/{0}/{1}/{2}".format(Testing.account_token, Testing.pid, method), **data)
        print("response: ", jcommand)

        self.assertEqual(jcommand["code"], 0)
        pass

    #----------------------------------------------------------------------------------------------------------------
    # create user
    #----------------------------------------------------------------------------------------------------------------
    def test_5_newuser(self):
        Testing.user_login = hummingbird.make_uuid()
        Testing.user_password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jnewuser = hummingbird.post("http://localhost:5555/newuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jnewuser)

        self.assertEqual(jnewuser["code"], 0)
        pass

    def test_6_loginuser(self):
        print("----loginuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jloginuser = hummingbird.post("http://localhost:5555/loginuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jloginuser)

        self.assertEqual(jloginuser["code"], 0)
        self.assertIn("token", jloginuser)

        Testing.user_token = jloginuser["token"]
        pass
    
    def test_7_api(self):    
        method = "test"
        data = dict(a=1, b=2, c="testc")
        print("----api---- token: {0} method: {1} data: {2}".format(Testing.user_token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(Testing.user_token, method), **data)
        print("response: ", japi)

        self.assertEqual(japi["code"], 0)
        pass

    def __create_user(self, pid):
        login = hummingbird.make_uuid()
        password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jnewuser = hummingbird.post("http://localhost:5555/newuser/{0}".format(pid), login = login, password = password)
        print("response: ", jnewuser)

        self.assertEqual(jnewuser["code"], 0)

        print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jloginuser = hummingbird.post("http://localhost:5555/loginuser/{0}".format(pid), login = login, password = password)
        print("response: ", jloginuser)

        self.assertEqual(jloginuser["code"], 0)

        token = jloginuser["token"]
        method = "test"
        data = dict(a=1, b=2, c="testc")
        print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(token, method), **data)
        print("response: ", japi)

        self.assertEqual(japi["code"], 0)
            
        token = jloginuser["token"]
        method = "join"
        data = dict()
        print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(token, method), **data)
        print("response: ", japi)

        self.assertEqual(japi["code"], 0)
        pass
            
    def test_8_matching(self):
        self.__create_user(Testing.pid)
        self.__create_user(Testing.pid)
        pass
    pass

unittest.main()