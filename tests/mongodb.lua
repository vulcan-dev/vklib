local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()
local mongo = vklib.mongo

-- I've never used mongo before, I'm confused.
-- API: https://github.com/neoxic/lua-mongo