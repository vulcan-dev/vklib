MONGODB_ENABLED = false

workspace "vklib"
    configurations { "Debug", "Release" }

project "vklib"
    kind "SharedLib"
    language "C"
    targetdir "build/bin"
    objdir "build/obj"
    architecture "x86_64"

    libdirs { "dep/lua/lib", "dep/mongodb/lib" }
    includedirs { "dep/lua/include", "dep/sqlite/include" }
    if MONGODB_ENABLED then
        includedirs { "dep/mongodb/include" }
    end

    links { "lua", "ws2_32" }
    if MONGODB_ENABLED then
        links { "mongoc-1.0", "bson-1.0" }
    end

    files {  "src/**.h", "src/**.c", }
    if not MONGODB_ENABLED then
        removefiles { "src/vklib_mongo/**.h", "src/vklib_mongo/**.c" }
    end

    defines { "VKLIB_FILESYSTEM", "VKLIB_REQUESTS", "VKLIB_SQLITE" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"

    filter {}