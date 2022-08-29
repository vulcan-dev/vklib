-- local vklib = assert(package.loadlib("cmake-build/Debug/vklib.dll", "luaopen_vklib"))()
local vklib = assert(require("vklib"))

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

print(vklib.fs.file_exists("./tests/filesystem/test.txt"))
print(vklib.fs.dir_exists("./tests/filesystem/"))

success, err = vklib.fs.remove_file("./tests/filesystem/test.txt")
if not success then
    print(err)
    return
else
    print("removed file")
end

success, err = vklib.fs.remove_dir("tests/filesystem", true)
if not success then
    print(err)
    return
else
    print("removed directory")
end