local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()

local cwd, err = vklib.fs.get_cwd()
if not cwd then
    print(err)
    return
else
    print("current directory: " .. cwd)
end

local success, err = vklib.fs.create_dir("./tests/filesystem", true)
if not success then
    print(err)
    return
else
    print("created directory")
end

success, err = vklib.fs.create_file("./tests/filesystem/test.txt", "hello abc", "w")
if not success then
    print(err)
    return
else
    print("created file")
end

success, err = vklib.fs.remove_dir("tests/filesystem", true)
if not success then
    print(err)
    return
else
    print("removed directory")
end