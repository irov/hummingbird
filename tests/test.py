from urllib import parse
from urllib import request
import mimetypes
import binascii
import io
import uuid
import json
import codecs

def post(url, **params):
    jd = json.dumps(params)
    data = jd.encode('utf-8')
    r = request.Request(url)
    r.add_header('Content-Type', 'application/json')
    r.add_header('Content-Length', len(data))
    response = request.urlopen(r, timeout=60, data=data)
    reader = codecs.getreader("utf-8")
    r = reader(response)
    j = json.load(r)
    return j
    pass
    
class MultiPartForm:
    """Accumulate the data to be used when posting a form."""

    def __init__(self):
        self.form_fields = []
        self.files = []
        # Use a large random byte string to separate
        # parts of the MIME data.
        self.boundary = uuid.uuid4().hex.encode('utf-8')
        return

    def get_content_type(self):
        return 'multipart/form-data; boundary={}'.format(
            self.boundary.decode('utf-8'))

    def add_field(self, name, value):
        """Add a simple field to the form data."""
        self.form_fields.append((name, value))

    def add_file(self, fieldname, filename, fileHandle,
                 mimetype=None):
        """Add a file to be uploaded."""
        body = fileHandle.read()
        if mimetype is None:
            mimetype = (
                mimetypes.guess_type(filename)[0] or
                'application/octet-stream'
            )
        self.files.append((fieldname, filename, mimetype, body))
        return

    @staticmethod
    def _form_data(name):
        return ('Content-Disposition: form-data; '
                'name="{}"\r\n').format(name).encode('utf-8')

    @staticmethod
    def _attached_file(name, filename):
        return ('Content-Disposition: form-data; '
                'name="{}"; filename="{}"\r\n').format(
                    name, filename).encode('utf-8')

    @staticmethod
    def _content_type(ct):
        return 'Content-Type: {}\r\n'.format(ct).encode('utf-8')

    def __bytes__(self):
        """Return a byte-string representing the form data,
        including attached files.
        """
        buffer = io.BytesIO()
        boundary = b'--' + self.boundary + b'\r\n'

        # Add the form fields
        for name, value in self.form_fields:
            buffer.write(boundary)
            buffer.write(self._form_data(name))
            buffer.write(b'\r\n')
            buffer.write(value.encode('utf-8'))
            buffer.write(b'\r\n')

        # Add the files to upload
        for f_name, filename, f_content_type, body in self.files:
            buffer.write(boundary)
            buffer.write(self._attached_file(f_name, filename))
            buffer.write(self._content_type(f_content_type))
            buffer.write(b'\r\n')
            buffer.write(body)
            buffer.write(b'\r\n')

        buffer.write(b'--' + self.boundary + b'--\r\n')
        return buffer.getvalue()
    
def upload(url, filename, **fields):
    form = MultiPartForm()
    for k, v in fields.items():
        form.add_field(k, v)

    # Add a fake file
    form.add_file('data', filename, fileHandle=open(filename, 'rb'))
    data=bytes(form)
    r = request.Request(url, data=data)
    r.add_header('Content-type', form.get_content_type())
    r.add_header('Content-length', len(data))
    response = request.urlopen(r, timeout=60)
    reader = codecs.getreader("utf-8")
    r = reader(response)
    j = json.load(r)
    return j
    pass
    
def api(url, token, method, **params):
    jd = json.dumps(params)
    data = jd.encode('utf-8')
    r = request.Request(url)
    r.add_header('X-Token', token)
    r.add_header('X-Method', method)
    r.add_header('Content-Type', 'application/json')
    r.add_header('Content-Length', len(data))
    response = request.urlopen(r, timeout=60, data=data)
    reader = codecs.getreader("utf-8")
    r = reader(response)
    j = json.load(r)
    return j
    pass
   
print("----newproject----")
jnewproject = post("http://localhost:5555/newproject")
print("response: ", jnewproject)
pid = jnewproject["pid"]

print("----upload---- pid: {0}".format(pid))
jupload = upload("http://localhost:5555/upload", "server.lua", pid = pid)
print("response: ", jupload)

login = uuid.uuid4().hex
password = "test"

print("----newuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
jnewuser = post("http://localhost:5555/newuser", pid = pid, login = login, password = password)
print("response: ", jnewuser)

print("----loginuser---- pid: {0} login: {1} password: {2}".format(pid, login, password))
jloginuser = post("http://localhost:5555/loginuser", pid = pid, login = login, password = password)
print("response: ", jloginuser)

token = jloginuser["token"]
method = "test"
data = dict(a=1, b=2, c="testc")
print("----api---- token: {0} method: {1} data: {2}".format(token, method, data))
japi = api("http://localhost:5555/api", token, method, **data)
print("response: ", japi)
