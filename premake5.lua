
require("premake_modules/export-compile-commands")

-- premake5.lua
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
    
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        cppdialect "C++17"
    
        defines { "NDEBUG" }
        optimize "On"

    filter {}

    project "blue"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
            buildoptions{ "--driver-mode=cl -Bv" }
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

        files { "blue/*.hh", "blue/*.cc" }

    
    project "boverlay"
        filter "system:linux"
            toolset "clang"
        filter "system:windows"
            toolset "msc-v141"
            --buildoptions{ "-Bv" }

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
            --buildoptions{ "-Bv" }
        filter {}

        dependson {"boverlay"}

        libdirs{"lib/%{cfg.buildcfg}"}

        kind "ConsoleApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"
            
        files { "testbed/*.hh", "testbed/*.cc" }    
        filter {}

