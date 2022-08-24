local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()

local client = vklib.mongodb.new_client()
client:connect("mongodb://localhost:27017/")
client:set_database(client:get_database("vklib_mongo"))

local collection = client:get_collection("test", "_id_")
local query = client:query(collection, 0, 0, 0, {}, {});
local result = query:next()
while result do
    print(result)
    result = query:next()
end

client:destroy_collection(collection)
client:destroy_database()
client:destroy()