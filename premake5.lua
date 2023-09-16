local ProjectName = _ARGS[1]

workspace(ProjectName)
	architecture "x64"
	startproject "ForthRepl"
	configurations { "Debug", "Release" }

project "ForthRepl"
	location "ForthRepl"
	kind "ConsoleApp"
	language "C"
	cdialect "C11"
	staticruntime "on"
	systemversion "latest"

	defines
	{

	}

	dependson
	{
		"Forth"
	}

	targetdir "Bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.c"
	}

	includedirs
	{
		"%{prj.name}/include"	
	}
	
	externalincludedirs
	{
		"%{prj.name}/../Forth/include",
	}

	libdirs
	{
		"Bin\\Forth\\%{cfg.buildcfg}\\%{cfg.platform}",
	}

	links
	{
		"Forth"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"
		
		
project "Forth"
	location "Engine"
	kind "StaticLib"
	language "C"
	cdialect "C11"
	staticruntime "off"
	systemversion "latest"
	
	defines
	{
	}
	
	dependson
	{
	}
	
	targetdir "Bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	
	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.c"
	}

	includedirs
	{
		"%{prj.name}/include"
	}

	externalincludedirs
	{
	}

	libdirs
	{
	}
	
	links
	{
	}

	postbuildcommands
	{
		"{MKDIR} \"%{wks.location}Bin/ForthRepl/%{cfg.buildcfg}/%{cfg.platform}\"",
		"{COPYFILE} \"%{wks.location}Bin/Forth/%{cfg.buildcfg}/%{cfg.platform}/%{cfg.buildtarget.basename}%{cfg.buildtarget.extension}\" \"%{wks.location}Bin/ForthRepl/%{cfg.buildcfg}/%{cfg.platform}\"",
		"{MKDIR} \"%{wks.location}Bin/ForthTest/%{cfg.buildcfg}/%{cfg.platform}\"",
		"{COPYFILE} \"%{wks.location}Bin/Forth/%{cfg.buildcfg}/%{cfg.platform}/%{cfg.buildtarget.basename}%{cfg.buildtarget.extension}\" \"%{wks.location}Bin/ForthRepl/%{cfg.buildcfg}/%{cfg.platform}\"",

		"rd /s /q $(SolutionDir)x64"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"

project "ForthTest"
	location "ForthTest"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	defines
	{

	}

	dependson
	{
		"Forth",
		"GoogleTest"
	}

	targetdir "Bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/include",
		"vendor/googletest/googletest/include" 
	}
	
	externalincludedirs
	{
		"%{prj.name}/../Forth/include",
	}

	libdirs
	{
		"Bin\\GoogleTest\\%{cfg.buildcfg}\\%{cfg.platform}",
	}

	links
	{
		"Forth",
		"GoogleTest"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"
		staticruntime "on"

project "GoogleTest"
    kind "StaticLib"
    files { "vendor/googletest/googletest/src/gtest-all.cc" }
    includedirs { "vendor/googletest/googletest/include", "vendor/googletest/googletest" }
    targetdir "Bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"
		staticruntime "on"

	
	postbuildcommands
	{
		"{MKDIR} \"%{wks.location}Bin/ForthTest/%{cfg.buildcfg}/%{cfg.platform}\"",
		"{COPYFILE} \"%{wks.location}Bin/GoogleTest/%{cfg.buildcfg}/%{cfg.platform}/%{cfg.buildtarget.basename}%{cfg.buildtarget.extension}\" \"%{wks.location}Bin/ForthTest/%{cfg.buildcfg}/%{cfg.platform}\"",

	}
newaction {
	trigger = "clean",
	description = "clean the software",
	execute = function ()
		print("Cleaning solution...")
		
		-- Solution files
		for _, slnFile in ipairs(os.matchfiles("*.sln")) do
            os.remove(slnFile)
        end
		
		-- Project files
		for _, slnFile in ipairs(os.matchfiles("*.vcxproj")) do
            os.remove(slnFile)
        end
		
		-- Project filter files
		for _, slnFile in ipairs(os.matchfiles("*.vcxproj.filters")) do
            os.remove(slnFile)
        end
		
		-- Specific files with extensions
        local filesToDelete = {
            -- Add more file paths here
        }
		for _, filePath in ipairs(filesToDelete) do
            os.remove(filePath)
        end
		
		-- Specific directories
        local directoriesToDelete = {
			"Bin",
            "obj",
            -- Add more file paths here
        }
		for _, directoryPath in ipairs(directoriesToDelete) do
            if os.isdir(directoryPath) then
                os.rmdir(directoryPath)
            end
        end
		
		print("Done cleaning.")
	end
}