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
			buildoptions{ "-Bv" }
		filter {}

        location "blue"

        kind "SharedLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

		filter "system:linux"
			pchheader "blue/stdafx.hh"
		filter "system:windows"
			pchheader "stdafx.hh"
		
		filter {}

		pchsource "blue/stdafx.cc"

        files { "blue/*.hh", "blue/*.cc" }