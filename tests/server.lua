event.onCreateUser = function(data)
    local public_data = {a = 1, b = "test"}
    local status = server.SetCurrentUserPublicData(public_data)
end

api.test = function(data)
    print(data.a, data.b, data.c)
    
    local status, a, b = server.GetCurrentUserPublicData({"a","b"})
    
    print(status)
    print(a)
    print(b)
    
    return status, {a=a, b=b}
end