version = 1

event.onCreateProject = function(data)
    local entity = server.CreateProjectEntity("match", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    local status = server.SetProjectPublicData(public_data)
end

event.onCreateUser = function(data)
    local entity = server.CreateUserEntity("test", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    local status = server.SetCurrentUserPublicData(public_data)
end

api.test = function(data)
    print(data.a, data.b, data.c)
    
    local status, a, b, e = server.GetCurrentUserPublicData({"a", "b", "e"})
    
    print(status)
    print(a)
    print(b)
    print(e)
    
    local status, a, b, c = server.GetUserEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(a)
    print(b)    
    print(c)    
    
    --[[
    local status, a, b, e = server.GetProjectPublicData({"a", "b", "e"})
    
    print(status)
    print(a)
    print(b)
    print(e)
    
    local status, a, b, c = server.GetProjectEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(a)
    print(b)    
    print(c)
    ]]--
    
    return status, {a=a, b=b, c=c, e=e}
end