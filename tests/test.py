import hummingbird

import sys
import unittest
import base64

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
        jresult = hummingbird.post("http://localhost:5555/newaccount", account_login = Testing.account_login, account_password = Testing.account_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
            
    def test_01_loginaccount(self):
        print("----loginaccount---- login: {0} password: {1}".format(Testing.account_login, Testing.account_password))
        jresult = hummingbird.post("http://localhost:5555/loginaccount", account_login = Testing.account_login, account_password = Testing.account_password)
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
        jresult = hummingbird.post("http://localhost:5555/newproject", account_token = Testing.account_token)
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
        f = open(filepath, 'rb')
        d = f.read()
        f.close()
        d64 = base64.b64encode(d).decode('utf-8')
        jresult = hummingbird.post("http://localhost:5555/upload", account_token = Testing.account_token, project_uid = Testing.pid, code = d64)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
    
    def test_04_command(self):
        method = "create"
        args = dict(a=5, b=6, c="testp")
        print("----command---- token: {0} method: {1} data: {2}".format(Testing.account_token, method, args))
        jresult = hummingbird.post("http://localhost:5555/command", account_token = Testing.account_token, project_uid = Testing.pid, method = method, args = args)
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
        jresult = hummingbird.post("http://localhost:5555/newuser", project_uid = Testing.pid, user_login = Testing.user_login, user_password = Testing.user_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass

    def test_06_loginuser(self):
        print("----loginuser---- pid: {0} login: {1} password: {2}".format(Testing.pid, Testing.user_login, Testing.user_password))
        jresult = hummingbird.post("http://localhost:5555/loginuser", project_uid = Testing.pid, user_login = Testing.user_login, user_password = Testing.user_password, user_public_data_revision = 0, project_public_data_revision = 0)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)

        Testing.user_token = jresult["token"]
        pass
    
    def test_07_api(self):
        method = "test"
        args = dict(a=1, b=2, c="testc")
        print("----api---- token: {0} method: {1} data: {2}".format(Testing.user_token, method, args))
        jresult = hummingbird.post("http://localhost:5555/api", user_token = Testing.user_token, method = method, args = args)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass

    def __createuser(self, pid):
        user_login = hummingbird.make_uuid()
        user_password = "testuser"

        print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, user_login, user_password))
        jresult = hummingbird.post("http://localhost:5555/newuser", project_uid = pid, user_login = user_login, user_password = user_password)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)

        print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, user_login, user_password))
        jresult = hummingbird.post("http://localhost:5555/loginuser", project_uid = pid, user_login = user_login, user_password = user_password, user_public_data_revision = 0, project_public_data_revision = 0)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        self.assertIn("token", jresult)
           
        token = jresult["token"]
        
        return token
        pass
        
    def __join_user(self, token):
        method = "join"
        args = dict()
        print("----api---- token: {0} method: {1} data: {2}".format(token, method, args))
        japi = hummingbird.post("http://localhost:5555/api", user_token = token, method = method, args = args)
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
        
    def __setusernickname(self, user_token, nickname):
        print("----setusernickname---- token: {0} {1}".format(user_token, nickname))
        jresult = hummingbird.post("http://localhost:5555/setusernickname", user_token = user_token, nickname = nickname)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def __setleaderscore(self, user_token, score):
        print("----setleaderscore---- token: {0} {1}".format(user_token, score))
        jresult = hummingbird.post("http://localhost:5555/setleaderscore", user_token = user_token, score = score)
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
        leaderboard_begin = 0
        leaderboard_end = 10
        print("----getleaderboard---- token: {0} beign: {1} end: {2}".format(Testing.user_token, leaderboard_begin, leaderboard_end))
        jresult = hummingbird.post("http://localhost:5555/getleaderboard", user_token = Testing.user_token, leaderboard_begin = leaderboard_begin, leaderboard_end = leaderboard_end)
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
        
        user_token = self.__createuser(Testing.pid)
        
        self.__setusernickname(user_token, "test")
        
        self.__setleaderscore(user_token, 175)
        
        print("----getleaderrank---- token: {0}".format(user_token))
        jresult = hummingbird.post("http://localhost:5555/getleaderrank", user_token = user_token)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertIn("rank", jresult)
        self.assertEqual(jresult["rank"], 2)
        pass
        
    def __postmessagechannel(self, user_token, channel, message, metainfo):
        print("----postmessageschannel---- token: {0} channel: {1}".format(user_token, channel))
        jresult = hummingbird.post("http://localhost:5555/postmessageschannel", user_token = user_token, messageschannel_uid=channel, messageschannel_message=message, messageschannel_metainfo=metainfo)
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
        jresult = hummingbird.post("http://localhost:5555/newmessageschannel", account_token = Testing.account_token, project_uid = Testing.pid, messageschannel_maxpost = 64)
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
        jresult = hummingbird.post("http://localhost:5555/getmessageschannel", user_token = token0, messageschannel_uid = channel, messageschannel_postid = 0)
        print("response: ", jresult)

        self.assertIsNotNone(jresult)
        self.assertEqual(jresult["code"], 0)
        pass
        
    def __geteventstopic(self, user_token, topic):
        print("----geteventstopic---- user_token: {0}".format(user_token))
        jresult = hummingbird.post("http://localhost:5555/geteventstopic", user_token = user_token, eventstopic_uid = topic, eventstopic_index = -1)
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
        
        print("----neweventstopic---- account_token: {0} pid: {1}".format(Testing.account_token, Testing.pid))
        jresult = hummingbird.post("http://localhost:5555/neweventstopic", account_token = Testing.account_token, project_uid = Testing.pid, eventstopic_name = "daily", eventstopic_delay = 5)
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

unittest.main(failfast=True)