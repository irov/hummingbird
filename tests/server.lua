command.create = function(data)
    local entity = server.CreateProjectEntity("match", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    server.SetProjectPublicData(public_data)
end

event.onCreateUser = function(data)
    local entity = server.CreateUserEntity("test", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    server.SetCurrentUserPublicData(public_data)
    print(string.format("onCreateUser"))
end

event.onLoginUser = function(data)
    local a, b, e = server.GetCurrentUserPublicData({"a"})

    local update_data = {a = a + 3}
    server.UpdateCurrentUserPublicData(update_data)
    print(string.format("onLoginUser"))
end

event.onCreateWorld = function(name, pid, avatars, data)
    print(name)
    print(pid)
    print(avatars)
    print(data)
end

api.test = function(data)
    print(data.a, data.b, data.c)
    
    local a, b, e = server.GetCurrentUserPublicData({"a", "b", "e"})
    
    print(string.format("a = %s b = %s e = %s", a, b, e))
    
    local a, b, c = server.GetUserEntityPublicData(e, {"a", "b", "c"})

    print(string.format("a = %s b = %s c = %s", a, b, c))
    
    --[[
    local status, a, b, c = server.GetProjectEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(a)
    print(b)    
    print(c)
    ]]--
    
    return true, {a=a, b=b, c=c, e=e}
end