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
    
    try:
        j = json.load(r)
        
        return j
    except json.decoder.JSONDecodeError as ex:
        print("json.decoder.JSONDecodeError:", ex)
        print("response:", response.decode('utf-8'), r)
        pass
        
    return None
    pass

def post(url, **params):
    jd = json.dumps(params)
    data = jd.encode('utf-8')
    r = request.Request(url)
    r.add_header('Content-Type', 'application/json')
    r.add_header('Content-Length', len(data))
    try:
        response = request.urlopen(r, timeout=60, data=data)
    except HTTPError as e:
        print("HTTPError: ", e.code, e.reason)
        return None
        pass
    
    j = __response_json(response)
    
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
    f = open(filename, 'rb')
    form.add_file('data', filename, fileHandle=f)
    data=bytes(form)
    f.close()
    r = request.Request(url, data=data)
    r.add_header('Content-type', form.get_content_type())
    r.add_header('Content-length', len(data))
    
    try:
        response = request.urlopen(r, timeout=60)
    except HTTPError as e:
        print("HTTPError: ", e.code, e.reason)
        return None
        pass
    
    j = __response_json(response)
    
    return j
    pass
    
def api(url, **params):
    jd = json.dumps(params)
    data = jd.encode('utf-8')
    r = request.Request(url)
    r.add_header('Content-Type', 'application/json')
    r.add_header('Content-Length', len(data))
    
    try:
        response = request.urlopen(r, timeout=60, data=data)
    except HTTPError as e:
        print("HTTPError: ", e.code, e.reason)
        return None
        pass
        
    j = __response_json(response)
        
    return j
    pass
    
def make_uuid():
    return uuid.uuid4().hex
    pass