require("premake_modules/export-compile-commands")

workspace "Blue"
    configurations { "Debug", "Release" }
    platforms { "x32" }

    location "premake"
    
    filter "system:windows"
        characterset "MBCS"

    filter {}

    filter "platforms:x32"
        architecture "x32"

    filter "configurations:Debug"
        cppdialect "C++17"
    
        defines { "DEBUG", "_DEBUG" }
        optimize "Off"
        symbols "Full"
        runtime "Debug"

    filter "configurations:Release"
        cppdialect "C++17"
    
        defines { "NDEBUG" }
        optimize "Full"
        symbols "Off"
        flags {"LinkTimeOptimization"}

    filter {}

    project "blue"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
            buildoptions{ "--driver-mode=cl" }
        filter {}

        kind "SharedLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        filter "system:linux"
            pchheader "blue/stdafx.hh"
        filter "system:windows"
            pchheader "stdafx.hh"
        
        filter {}

        pchsource "blue/stdafx.cc"

		includedirs { "blue" }
        files { "blue/*.hh", "blue/*.cc",
                "blue/modules/*.hh", "blue/modules/*.cc" }

    
    project "boverlay"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
        filter {}

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

        includedirs {"boverlay/include/ftgl", "boverlay/include"}

        filter {"configurations:Debug"}
            libdirs {"boverlay/lib", "boverlay/lib/debug"}
        filter {"configurations:Release"}
            libdirs {"boverlay/lib", "boverlay/lib/release"}
        filter {}

		links {"ftgl_static", "freetype281", "glew32s"}
        
        files { "boverlay/*.hh", "boverlay/*.cc" }
        filter {}

    project "testbed"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
        filter {}

        dependson {"boverlay"}

        libdirs{"lib/%{cfg.buildcfg}"}

        kind "ConsoleApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"
            
        files { "testbed/*.hh", "testbed/*.cc" }    
        filter {}

