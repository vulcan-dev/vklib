-- local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()
local vklib = assert(require("vklib"))
local mongo = vklib.mongo

-- I've never used mongo before, I'm confused.
-- API: https://github.com/neoxic/lua-mongo
-- Note: I have lowercased the main names, check "vklib.c"

local client = mongo.client("mongodb://localhost:27017")
local collection = client:getCollection("test", "_id_")
collection:drop()

local id = mongo.objectid()
print(id)