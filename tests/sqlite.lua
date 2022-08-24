local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()

-- print(vklib.sqlite.connect("test"))