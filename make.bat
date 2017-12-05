pushd %~dp0

premake5 vs2017
premake5 export-compile-commands

"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\msbuild" /property:GenerateFullPaths=true /t:build Blue.sln /p:Configuration=Debug /verbosity:minimal

popd

exit
