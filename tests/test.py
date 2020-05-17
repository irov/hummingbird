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
        jresult = hummingbird.post("http://localhost:5555/newaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
            
    def test_01_loginaccount(self):
        print("----loginaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jresult = hummingbird.post("http://localhost:5555/loginaccount", login = Testing.account_login, password = Testing.account_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)
        
        Testing.account_token = jresult["token"]
        pass
            
    #----------------------------------------------------------------------------------------------------------------
    # create project
    #----------------------------------------------------------------------------------------------------------------
            
    def test_02_newproject(self):
        print("----newproject---- token: {0}".format(Testing.account_token))
        jresult = hummingbird.post("http://localhost:5555/newproject/{0}".format(Testing.account_token))
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("pid", jresult)

        Testing.pid = jresult["pid"]
        pass

    def test_03_upload(self):
        global GLOBAL_upload_filepath
        filepath = "test.lua" if GLOBAL_upload_filepath is None else GLOBAL_upload_filepath
        print("----upload---- token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jresult = hummingbird.upload("http://localhost:5555/upload/{0}/{1}".format(Testing.account_token, Testing.pid), filepath)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
    
    def test_04_command(self):    
        method = "create"
        data = dict(a=5, b=6, c="testp")
        print("----command---- token: {0} method: {1} data: {2}".format(Testing.account_token, method, data))
        jresult = hummingbird.api("http://localhost:5555/command/{0}/{1}/{2}".format(Testing.account_token, Testing.pid, method), **data)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass

    #----------------------------------------------------------------------------------------------------------------
    # create user
    #----------------------------------------------------------------------------------------------------------------
    def test_05_newuser(self):
        Testing.user_login = hummingbird.make_uuid()
        Testing.user_password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jresult = hummingbird.post("http://localhost:5555/newuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass

    def test_06_loginuser(self):
        print("----loginuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jresult = hummingbird.post("http://localhost:5555/loginuser/{0}".format(Testing.pid), login = Testing.user_login, password = Testing.user_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)

        Testing.user_token = jresult["token"]
        pass
    
    def test_07_api(self):    
        method = "test"
        data = dict(a=1, b=2, c="testc")
        print("----api---- token: {0} method: {1} data: {2}".format(Testing.user_token, method, data))
        jresult = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(Testing.user_token, method), **data)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass

    def __create_user(self, pid):
        login = hummingbird.make_uuid()
        password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jresult = hummingbird.post("http://localhost:5555/newuser/{0}".format(pid), login = login, password = password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)

        print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jresult = hummingbird.post("http://localhost:5555/loginuser/{0}".format(pid), login = login, password = password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)
           
        token = jresult["token"]
        
        return token
        pass
        
    def __join_user(self, token):
        method = "join"
        data = dict()
        print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
        japi = hummingbird.api("http://localhost:5555/api/{0}/{1}".format(token, method), **data)
        print("response: ", japi)

        self.assertIsNotNone(japi)
        self.assertEqual(japi["code"], 0)        
        pass
            
    def test_08_matching(self):
        token0 = self.__create_user(Testing.pid)
        token1 = self.__create_user(Testing.pid)
        
        self.__join_user(token0)
        self.__join_user(token1)
        pass
        
    def __setusernickname_user(self, token, nickname):
        print("----setusernickname---- token: {0} {1}".format(token, nickname))
        jresult = hummingbird.post("http://localhost:5555/setusernickname/{0}".format(token), nickname = nickname)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def __setleaderboard_user(self, token, score):
        print("----setleaderboard---- token: {0} {1}".format(token, score))
        jresult = hummingbird.post("http://localhost:5555/setleaderboard/{0}".format(token), score = score)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def test_09_leaderboard(self):
        token0 = self.__create_user(Testing.pid)
        token1 = self.__create_user(Testing.pid)
        token2 = self.__create_user(Testing.pid)
        token3 = self.__create_user(Testing.pid)
        token4 = self.__create_user(Testing.pid)
        
        self.__setusernickname_user(token0, "bob")
        self.__setusernickname_user(token1, "alice")
        self.__setusernickname_user(token2, "dave")
        self.__setusernickname_user(token3, "nick")
        self.__setusernickname_user(token4, "jennifer")
        
        self.__setleaderboard_user(token0, 100)
        self.__setleaderboard_user(token1, 200)
        self.__setleaderboard_user(token2, 50)
        self.__setleaderboard_user(token3, 250)
        self.__setleaderboard_user(token4, 150)
        pass
        
    def test_11_getleaderboard(self):
        begin = 0
        end = 10
        print("----getleaderboard---- token: {0} {1} {2}".format(Testing.user_token, begin, end))
        jresult = hummingbird.post("http://localhost:5555/getleaderboard/{0}".format(Testing.user_token), begin = begin, end = end)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertEqual(jresult["leaderboard"][0]["nickname"], "nick")
        self.assertEqual(jresult["leaderboard"][0]["score"], 250)
        self.assertEqual(jresult["leaderboard"][1]["nickname"], "alice")
        self.assertEqual(jresult["leaderboard"][1]["score"], 200)
        pass        
    pass

unittest.main()