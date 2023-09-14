local ProjectName = _ARGS[1]

workspace(ProjectName)
	architecture "x64"
	startproject "Game"
	configurations { "Debug", "Release" }

project "Game"
	location "Game"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	defines
	{
		"GLM_FORCE_SWIZZLE",
		"GLM_FORCE_RADIANS",
		"GLM_ENABLE_EXPERIMENTAL",
	}

	dependson
	{
		"Engine",
		"glad",
		"glfw",
		"ImGui",
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
		"%{prj.name}/include"	
	}
	
	externalincludedirs
	{
		"%{prj.name}/../Engine/include",
		"vendor\\glad\\include",
		"vendor\\glfw\\include",
		"vendor\\imgui",
		"vendor\\imgui\\backends",
		"vendor\\glm\\glm",
	}

	libdirs
	{
		"Bin\\Engine\\%{cfg.buildcfg}\\%{cfg.platform}",
		"vendor\\glad\\lib\\%{cfg.buildcfg}\\%{cfg.platform}",
		"vendor\\glfw\\build\\src\\%{cfg.buildcfg}",
		"vendor\\imgui\\lib\\%{cfg.buildcfg}\\%{cfg.platform}",
	}

	links
	{
		"Engine.lib",
		"glad.lib",
		"glfw3.lib",
		"imgui.lib",
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"
		
		
project "Engine"
	location "Engine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	systemversion "latest"
	
	defines
	{
		"BUILD_DLL",
		"GLM_FORCE_SWIZZLE",
		"GLM_FORCE_RADIANS",
		"GLM_ENABLE_EXPERIMENTAL",
	}
	
	dependson
	{
		"glad",
		"glfw",
		"ImGui",
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
		"%{prj.name}/include"
	}

	externalincludedirs
	{
		"vendor\\glad\\include",
		"vendor\\glfw\\include",
		"vendor\\imgui",
		"vendor\\imgui\\backends",
		"vendor\\glm\\glm",
	}

	libdirs
	{
		"vendor\\glad\\lib\\%{cfg.buildcfg}\\%{cfg.platform}",
		"vendor\\glfw\\build\\src\\%{cfg.buildcfg}",
		"vendor\\imgui\\lib\\%{cfg.buildcfg}\\%{cfg.platform}",
	}
	
	links
	{
		"opengl32.lib",
		"glad.lib",
		"glfw3.lib",
		"imgui.lib",
	}

	postbuildcommands
	{
		"{MKDIR} \"%{wks.location}Bin/Game/%{cfg.buildcfg}/%{cfg.platform}\"",
		"{COPYFILE} \"%{wks.location}Bin/Engine/%{cfg.buildcfg}/%{cfg.platform}/%{cfg.buildtarget.basename}%{cfg.buildtarget.extension}\" \"%{wks.location}Bin/Game/%{cfg.buildcfg}/%{cfg.platform}\"",
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

project "ImGui"
	location "vendor/imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	targetdir "/%{prj.location}/lib/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "/%{prj.location}/lib/Intermediate/%{cfg.buildcfg}/%{cfg.platform}"

	includedirs
	{
		"%{prj.location}"
	}
	
	externalincludedirs
	{
		"vendor\\glfw\\include"
	}

	files
	{
		"%{prj.location}/imconfig.h",
		"%{prj.location}/imgui.h",
		"%{prj.location}/imgui.cpp",
		"%{prj.location}/imgui_demo.cpp",
		"%{prj.location}/imgui_draw.cpp",
		"%{prj.location}/imgui_internal.h",
		"%{prj.location}/imgui_tables.cpp",
		"%{prj.location}/imgui_widgets.cpp",
		"%{prj.location}/imstb_rectpack.h",
		"%{prj.location}/imstb_textedit.h",
		"%{prj.location}/imstb_truetype.h",
		"%{prj.location}/backends/imgui_impl_glfw.h",
		"%{prj.location}/backends/imgui_impl_glfw.cpp",
		"%{prj.location}/backends/imgui_impl_opengl3.h",
		"%{prj.location}/backends/imgui_impl_opengl3.cpp",		
	}
	
project "glad"
	location "vendor/glad"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	targetdir "/%{prj.location}/lib/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "/%{prj.location}/lib/Intermediate/%{cfg.buildcfg}/%{cfg.platform}"

	externalincludedirs
	{
		"%{prj.location}/include"
	}

	files
	{
		"%{prj.location}/include/glad/glad.h",
		"%{prj.location}/include/KHR/khrplatform.h",
		"%{prj.location}/src/glad.c",
	}
	
project "AllocatorTest"
	location "AllocatorTest"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	dependson
	{
		"Engine",
	}

	targetdir "Bin/Game/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/AllocatorTest/%{cfg.buildcfg}/%{cfg.platform}"

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/include"	
	}
	
	externalincludedirs
	{
		"%{prj.name}/../Engine/include",
	}

	libdirs
	{
		"Bin\\Engine\\%{cfg.buildcfg}\\%{cfg.platform}",
	}

	links
	{
		"Engine.lib",
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"

project "VoxelVolumeTest"
	location "VoxelVolumeTest"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	systemversion "latest"

	dependson
	{
		"Engine",
	}

	targetdir "Bin/Game/%{cfg.buildcfg}/%{cfg.platform}"
	objdir "Bin/Intermediate/AllocatorTest/%{cfg.buildcfg}/%{cfg.platform}"

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/include"	
	}
	
	externalincludedirs
	{
		"%{prj.name}/../Engine/include",
		"vendor\\glm\\glm",
	}

	libdirs
	{
		"Bin\\Engine\\%{cfg.buildcfg}\\%{cfg.platform}",
	}

	links
	{
		"Engine.lib",
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"


externalproject "glfw"
	location ("%{wks.location}\\vendor\\glfw\\build\\src")
	uuid "F70CFA3B-BFC4-3B3F-B758-519FB418430D"
	kind "StaticLib"
	language "C++"
	
externalproject "ZERO_CHECK"
	location ("%{wks.location}\\vendor\\glfw\\build")
	uuid "FC96CD67-3228-3471-843D-D7756AE336C1"
	kind "None"
	
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
            "vendor/glfw/build",
			"vendor/imgui/lib",
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