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

    
	project "boverlay"
	    filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
            --buildoptions{ "-Bv" }
            
        location "boverlay"

        kind "Sharedlib"
        language "C++"
		targetdir "bin/%{cfg.buildcfg}"
		implibdir "lib/%{cfg.buildcfg}"

        filter "system:linux"
			pchheader "boverlay/stdafx.hh"
		filter "system:windows"
            pchheader "stdafx.hh"
            
        filter {}
			
        pchsource "boverlay/stdafx.cc"
		
        files { "boverlay/*.hh", "boverlay/*.cc" }	
		filter {}

	project "testbed"
	    filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
            --buildoptions{ "-Bv" }

        filter {}

        location "testbed"
		libdirs{"lib/%{cfg.buildcfg}"}

        kind "ConsoleApp"
        language "C++"
		targetdir "bin/%{cfg.buildcfg}"
			
        files { "testbed/*.hh", "testbed/*.cc" }	
		filter {}

