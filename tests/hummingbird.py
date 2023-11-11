from urllib import request
from urllib.error import HTTPError
import mimetypes
import binascii
import sys
import io
import uuid
import json
import codecs

def __response_json(response):
    reader = codecs.getreader("utf-8")
    r = reader(response)
    
    d = r.read()
    
    try:
        j = json.loads(d)
        
        return j
    except json.decoder.JSONDecodeError as e:
        print("except json.decoder.JSONDecodeError:", e)
        print("data:", d)
        pass
        
    return None
    pass
    
def __post_json(params):
    try:
        jd = json.dumps(params)
        data = jd.encode('utf-8')
        
        return data
    except json.decoder.JSONDecodeError as e:
        print("except json.dumps.JSONDecodeError:", e)
        print("params", params)
        pass
    
    return None
    pass

def post(server, cmd, **params):
    url = "{0}/{1}".format(server, cmd)
    data = __post_json(params)
    
    if data is None:
        print("post url '{0}' invalid json".format(url))
        return None
        pass
        
    print("post url '{0}' data: {1}".format(url, data))
    
    r = request.Request(url)
    r.add_header('Content-Type', 'application/json')
    r.add_header('Content-Length', len(data))
    try:
        response = request.urlopen(r, timeout=5, data=data)
    except HTTPError as e:
        print("post url '{0}' HTTPError: {1} [{2}]".format(url, e.reason, e.code))
        return None
        pass
    
    j = __response_json(response)
    
    return j
    pass
    
def make_uuid():
    return uuid.uuid4().hex
    pass