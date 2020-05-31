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

    def __createuser(self, pid):
        login = hummingbird.make_uuid()
        password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
        jresult = hummingbird.post("http://localhost:5555/newuser/{0}".format(pid), login = login, password = password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)

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
        token0 = self.__createuser(Testing.pid)
        token1 = self.__createuser(Testing.pid)
        
        self.__join_user(token0)
        self.__join_user(token1)
        pass
        
    def __setusernickname(self, token, nickname):
        print("----setusernickname---- token: {0} {1}".format(token, nickname))
        jresult = hummingbird.post("http://localhost:5555/setusernickname/{0}".format(token), nickname = nickname)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def __setleaderscore(self, token, score):
        print("----setleaderscore---- token: {0} {1}".format(token, score))
        jresult = hummingbird.post("http://localhost:5555/setleaderscore/{0}".format(token), score = score)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def test_09_leaderboard(self):
        print("----leaderboard---- token:")
        
        token0 = self.__createuser(Testing.pid)
        token1 = self.__createuser(Testing.pid)
        token2 = self.__createuser(Testing.pid)
        token3 = self.__createuser(Testing.pid)
        token4 = self.__createuser(Testing.pid)
        
        self.__setusernickname(token0, "bob")
        self.__setusernickname(token1, "alice")
        self.__setusernickname(token2, "dave")
        self.__setusernickname(token3, "nick")
        self.__setusernickname(token4, "jennifer")
        
        self.__setleaderscore(token0, 100)
        self.__setleaderscore(token1, 200)
        self.__setleaderscore(token2, 50)
        self.__setleaderscore(token3, 250)
        self.__setleaderscore(token4, 150)
        pass
        
    def test_11_getleaderboard(self):
        begin = 0
        end = 10
        print("----getleaderboard---- token: {0} beign: {1} end: {2}".format(Testing.user_token, begin, end))
        jresult = hummingbird.post("http://localhost:5555/getleaderboard/{0}".format(Testing.user_token), begin = begin, end = end)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertEqual(jresult["leaderboard"][0]["nickname"], "nick")
        self.assertEqual(jresult["leaderboard"][0]["score"], 250)
        self.assertEqual(jresult["leaderboard"][1]["nickname"], "alice")
        self.assertEqual(jresult["leaderboard"][1]["score"], 200)
        self.assertEqual(jresult["leaderboard"][2]["nickname"], "jennifer")
        self.assertEqual(jresult["leaderboard"][2]["score"], 150)
        self.assertEqual(jresult["leaderboard"][3]["nickname"], "bob")
        self.assertEqual(jresult["leaderboard"][3]["score"], 100)
        self.assertEqual(jresult["leaderboard"][4]["nickname"], "dave")
        self.assertEqual(jresult["leaderboard"][4]["score"], 50)
        pass
        
    def test_11_getleaderrank(self):
        print("----getleaderrank----")
        
        token0 = self.__createuser(Testing.pid)
        
        self.__setusernickname(token0, "test")
        
        self.__setleaderscore(token0, 175)
        
        print("----getleaderrank---- token: {0}".format(token0))
        jresult = hummingbird.post("http://localhost:5555/getleaderrank/{0}".format(token0))
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertIn("rank", jresult)
        self.assertEqual(jresult["rank"], 2)
        pass
        
    def __postmessagechannel(self, token, channel, message, metainfo):
        print("----postmessageschannel---- token: {0} channel: {1}".format(token, channel))
        jresult = hummingbird.post("http://localhost:5555/postmessageschannel/{0}".format(token), uid=channel, message=message, metainfo=metainfo)
        print("response: ", jresult)
        
        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("postid", jresult)
        
        postid = jresult["postid"]
        
        return postid
        pass
        
    def test_12_messages(self):
        print("----messages----")
        
        print("----newmessageschannel---- token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jresult = hummingbird.post("http://localhost:5555/newmessageschannel/{0}/{1}".format(Testing.account_token, Testing.pid), maxpost=64)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("uid", jresult)
        
        channel = jresult["uid"]
        
        token0 = self.__createuser(Testing.pid)
        token1 = self.__createuser(Testing.pid)
        
        postid0 = self.__postmessagechannel(token0, channel, "Hello World!", "<hello>")
        postid1 = self.__postmessagechannel(token1, channel, "Happy World!", "<happy>")
        
        print("----getmessageschannel---- token: {0}".format(token0))
        jresult = hummingbird.post("http://localhost:5555/getmessageschannel/{0}".format(token0), uid=channel, postid=0)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def __geteventstopic(self, token, topic):
        print("----geteventstopic---- token: {0}".format(token))
        jresult = hummingbird.post("http://localhost:5555/geteventstopic/{0}".format(token), uid=topic, index=-1)
        print("response: ", jresult)
        
        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        
        index = jresult["index"]
        message = jresult["message"]
        
        print("index: ", index)
        print("message: ", message)
        
        return index
        pass
        
    def test_13_events(self):
        print("----events----")
        
        print("----neweventstopic---- token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jresult = hummingbird.post("http://localhost:5555/neweventstopic/{0}/{1}".format(Testing.account_token, Testing.pid), name="daily", delay=5)
        print("response: ", jresult)
        
        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("uid", jresult)
        
        topic = jresult["uid"]
        
        print("topic: ", topic)
        
        token0 = self.__createuser(Testing.pid)
        
        self.__geteventstopic(token0, topic)
        self.__geteventstopic(token0, topic)
        self.__geteventstopic(token0, topic)
        pass
    pass

unittest.main()