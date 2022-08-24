local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()

-- local req, err = vklib.request.get("https://httpbin.org/ip")
local req, err = vklib.request.get("http://kissmp.online:3692/0.5.0")
if not req then
    print(err)
    return
end

print("content-type: " .. req.headers["content-type"])

-- for k, v in pairs(req.headers) do
--     print(k .. " = " .. v)
-- end

print('\n' .. req.body)