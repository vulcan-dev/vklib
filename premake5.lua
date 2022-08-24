workspace "vklib"
    configurations { "Debug", "Release" }

project "vklib"
    kind "SharedLib"
    language "C"
    targetdir "build/bin"
    objdir "build/obj"
    architecture "x86_64"

    libdirs "dep/lua/lib"
    includedirs { "dep/lua/include", "dep/sqlite/include" }

    links { "lua", "ws2_32" }

    files { "src/**.h", "src/**.c" }

    defines { "VKLIB_FILESYSTEM", "VKLIB_REQUESTS", "VKLIB_SQLITE" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"

    filter {}