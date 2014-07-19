solution "gs"
	configurations { "Debug", "Release" }

	language 	"C++"
	includedirs	{ "../include/" }

	if _ACTION == "gmake" and os.get() == "macosx" then
		location "osx/make/"
	elseif _ACTION == "gmake" and os.get() == "linux" then
		location "unix"
	elseif _ACTION == "gmake" and os.get() == "windows" then
		location "windows/mingw"
	elseif _ACTION == "vs2010" then
		location "windows/vs2010"
	elseif _ACTION == "vs2012" then
		location "windows/vs2012"
	elseif _ACTION == "vs2013" then
		location "windows/vs2013"
	elseif _ACTION == "vs2008" then
		location "windows/vs2008"
	elseif _ACTION == "xcode4" then
		location "osx/xcode"
	end

	if os.get() == "windows" then
		
		if _ACTION == "gmake" then
			buildoptions 	{ "-std=c++11" }
			defines		{ "DRGN_OS_WINDOWS_MINGW" }
		else
			defines		{ "DRGN_OS_WINDOWS_VS" }
		end

		configuration "Debug"
			targetdir	"../bin/windows/debug"
		
		configuration "Release"
			targetdir	"../bin/windows/release"

	elseif os.get() == "linux" then
		buildoptions 	{ "-std=c++11" }
		
		configuration "Debug"
			buildoptions	{ "-g" }
			targetdir	"../bin/unix/debug"
		
		configuration "Release"
			targetdir	"../bin/unix/release"
				
	else
		buildoptions 	{ "-stdlib=libc++", "-std=c++11" }
		
		if _ACTION == "gmake" then
			linkoptions	{ "-stdlib=libc++" }
		end
		
		configuration "Debug"
			targetdir	"../bin/osx/debug"
		
		configuration "Release"
			targetdir	"../bin/osx/release"
		
	end

	configuration "Debug"
		defines 	{ "GS_DEBUG" }
		flags		{ "Symbols" }

	configuration "Release"
		defines 	{ "GS_RELEASE" }
		flags		{ "Optimize" }

	project "test"
		kind		"ConsoleApp"
		includedirs	{ "../deps/Catch/include" }

		files 		{ 
			"../src/test/**.h",
			"../src/test/**.cpp"
		}

		if _ACTION == "gmake" and os.get() == "linux" then
			postbuildcommands {
				"cd ../../bin/unix/debug && ./test"
			}
		elseif _ACTION == "gmake" and os.get() == "macosx" then
			postbuildcommands {
				"cd ../../../bin/osx/debug && ./test"
			}
		end