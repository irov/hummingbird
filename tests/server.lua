event.onCreateProject = function(data)
    local entity = server.CreateProjectEntity("match", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    local status = server.SetProjectPublicData(public_data)
end

event.onCreateUser = function(data)
    local entity = server.CreateUserEntity("test", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    local status = server.SetCurrentUserPublicData(public_data)
    print(string.format("onCreateUser: %s", status))
end

event.onLoginUser = function(data)
    local status, a, b, e = server.GetCurrentUserPublicData({"a"})

    local update_data = {a = a + 3}
    local status = server.UpdateCurrentUserPublicData(update_data)
    print(string.format("onLoginUser: %s", status))
end

api.test = function(data)
    print(data.a, data.b, data.c)
    
    local status, a, b, e = server.GetCurrentUserPublicData({"a", "b", "e"})
    
    print(status)
    print(string.format("a = %s b = %s e = %s", a, b, e))
    
    local status, a, b, c = server.GetUserEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(string.format("a = %s b = %s c = %s", a, b, c))
    
    --[[
    local status, a, b, c = server.GetProjectEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(a)
    print(b)    
    print(c)
    ]]--
    
    return status, {a=a, b=b, c=c, e=e}
end