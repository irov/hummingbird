import hummingbird

import sys
import unittest

GLOBAL_upload_filepath = None

if len(sys.argv) == 2:
    GLOBAL_upload_filepath = sys.argv.pop()
    pass

class Testing(unittest.TestCase):
    #----------------------------------------------------------------------------------------------------------------
    # create account
    #----------------------------------------------------------------------------------------------------------------
    
    def test_00_newaccount(self):
        Testing.account_login = hummingbird.make_uuid()
        Testing.account_password = "testaccount"

        print("----newaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jnewaccount = hummingbird.post("http://localhost:5555/newaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jnewaccount)

        self.assertIsNotNone(jnewaccount)
        self.assertEqual(jnewaccount["code"], 0)
        pass
            
    def test_01_loginaccount(self):
        print("----loginaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jloginaccount = hummingbird.post("http://localhost:5555/loginaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jloginaccount)

        self.assertIsNotNone(jloginaccount)
        self.assertEqual(jloginaccount["code"], 0)
        self.assertIn("token", jloginaccount)
        
        Testing.account_token = jloginaccount["token"]
        pass
            
    #----------------------------------------------------------------------------------------------------------------
    # create project
    #----------------------------------------------------------------------------------------------------------------
            
    def test_02_newproject(self):
        print("----newproject---- token: {0}".format(Testing.account_token))
        jnewproject = hummingbird.post("http://localhost:5555/newproject/{0}".format(Testing.account_token))
        print("response: ", jnewproject)

        self.assertIsNotNone(jnewproject)
        self.assertEqual(jnewproject["code"], 0)
        self.assertIn("pid", jnewproject)

        Testing.pid = jnewproject["pid"]
        pass

    def test_03_upload(self):
        global GLOBAL_upload_filepath
        filepath = "test.lua" if GLOBAL_upload_filepath is None else GLOBAL_upload_filepath
        print("----upload---- token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jupload = hummingbird.upload("http://localhost:5555/upload/{0}/{1}".format(Testing.account_token, Testing.pid), filepath)
        print("response: ", jupload)

        self.assertIsNotNone(jupload)
        self.assertEqual(jupload["code"], 0)
        pass
    
    def test_04_command(self):    
        method = "create"
        data = dict(a=5, b=6, c="testp")
        print("----command---- token: {0} method: {1} data: {2}".format(Testing.account_token, method, data))
        jcommand = hummingbird.api("http://localhost:5555/command/{0}/{1}/{2}".format(Testing.account_token, Testing.pid, method), **data)
        print("response: ", jcommand)

        self.assertIsNotNone(jcommand)
        self.assertEqual(jcommand["code"], 0)
        pass

    #----------------------------------------------------------------------------------------------------------------
    # create user
    #----------------------------------------------------------------------------------------------------------------
    def test_05_newuser(self):
        Testing.user_login = hummingbird.make_uuid()
        Testing.user_password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jnewuser = hummingbird.post("http://localhost:5555/newuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jnewuser)

        self.assertIsNotNone(jnewuser)
        self.assertEqual(jnewuser["code"], 0)
        pass

    def test_06_loginuser(self):
        print("----loginuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jloginuser = hummingbird.post("http://localhost:5555/loginuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jloginuser)

        self.assertIsNotNone(jloginuser)
        self.assertEqual(jloginuser["code"], 0)
        self.assertIn("token", jloginuser)

        Testing.user_token = jloginuser["token"]
        pass
    
    def test_07_api(self):    
        method = "test"
        data = dict(a=1, b=2, c="testc")
        print("----api---- token: {0} method: {1} data: {2}".format(Testing.user_token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(Testing.user_token, method), **data)
        print("response: ", japi)

        self.assertIsNotNone(japi)
        self.assertEqual(japi["code"], 0)
        pass

    def __create_user(self, pid):
        login = hummingbird.make_uuid()
        password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jnewuser = hummingbird.post("http://localhost:5555/newuser/{0}".format(pid), login = login, password = password)
        print("response: ", jnewuser)

        self.assertIsNotNone(jnewuser)
        self.assertEqual(jnewuser["code"], 0)

        print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jloginuser = hummingbird.post("http://localhost:5555/loginuser/{0}".format(pid), login = login, password = password)
        print("response: ", jloginuser)

        self.assertIsNotNone(jloginuser)
        self.assertEqual(jloginuser["code"], 0)
        self.assertIn("token", jloginuser)
           
        token = jloginuser["token"]
        method = "join"
        data = dict()
        print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(token, method), **data)
        print("response: ", japi)

        self.assertIsNotNone(japi)
        self.assertEqual(japi["code"], 0)
        pass
            
    def test_08_matching(self):
        self.__create_user(Testing.pid)
        self.__create_user(Testing.pid)
        pass
        
    def test_09_setleaderboard(self):
        score = 7
        print("----setleaderboard---- token: {0} {1}".format(Testing.user_token, score))
        jsetleaderboard = hummingbird.post("http://localhost:5555/setleaderboard/{0}".format(Testing.user_token), score = score)
        print("response: ", jsetleaderboard)

        self.assertIsNotNone(jsetleaderboard)
        self.assertEqual(jsetleaderboard["code"], 0)
        pass
        
    def test_10_getleaderboard(self):
        begin = 0
        end = 10
        print("----getleaderboard---- token: {0} {1} {2}".format(Testing.user_token, begin, end))
        jgetleaderboard = hummingbird.post("http://localhost:5555/getleaderboard/{0}".format(Testing.user_token), begin = begin, end = end)
        print("response: ", jgetleaderboard)

        self.assertIsNotNone(jgetleaderboard)
        self.assertEqual(jgetleaderboard["code"], 0)
        pass        
    pass

unittest.main()