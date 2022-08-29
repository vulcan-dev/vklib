-- local vklib = assert(package.loadlib("cmake-build/Debug/vklib.dll", "luaopen_vklib"))()
local vklib = assert(require("vklib"))

print("getting cwd")

local cwd, err = vklib.fs.get_cwd()
if not cwd then
    print(err)
    return
else
    print("current directory: " .. cwd)
end

print("creating dir")

local success, err = vklib.fs.create_dir("./tests/filesystem", true)
if not success then
    print(err)
    return
else
    print("created directory")
end

print("creating file")

success, err = vklib.fs.create_file("./tests/filesystem/test.txt", "hello abc", "w")
if not success then
    print("create_file failed: " .. tostring(err))
    return
else
    print("created file")
end

print("file_exists: " .. tostring(vklib.fs.file_exists("./tests/filesystem/test.txt")))
print("dir_exists: " .. tostirng(vklib.fs.dir_exists("./tests/filesystem/")))

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