-- premake5.lua
workspace "Blue"
    configurations { "Debug", "Release" }
    platforms { "x32" }
	
	filter "system:windows"
		characterset "MBCS"

    filter "platforms:x32"
        architecture "x32"

    filter "configurations:Debug"
        cppdialect "C++17"
    
        defines { "DEBUG" }
		symbols "On"

    filter "configurations:Release"
        cppdialect "C++17"
    
        defines { "NDEBUG" }
        optimize "On"

    project "blue"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
		filter {}

        location "blue"

        kind "SharedLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        pchheader "blue/stdafx.h"
        pchsource "blue/stdafx.cpp"

        files { "blue/*.h", "blue/*.cpp" }