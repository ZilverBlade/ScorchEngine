workspace "ScorchEngine"
   architecture "x86_64"
   preferredtoolarchitecture "x86_64"
   configurations { "Debug", "Release" }

local outputSubfolder = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local VULKAN_SDK = os.getenv("VULKAN_SDK")
   
project "ScorchEngineDev"
   location "./"
   debugdir "./"
   local bin = "./bin/"..outputSubfolder

   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir (bin)

   flags { "MultiProcessorCompile" }
   disablewarnings { "26812;4244;4996;4005" }
   
   files {
	  "src/scorch/**.cpp",
	  "src/scorch/**.h",
	  
      "src/scorch/vkapi/**.*",
      "src/scorch/rendering/**.*",
      "src/scorch/systems/**.*",
      "src/scorch/graphics/**.*",
      "src/scorch/ecs/**.*",
      "src/scorch/utils/**.*",
      "src/scorch/apps/**.*"
   }

   includedirs {
	  "./src",
	  VULKAN_SDK.."/Include",
	  "vendor/glfw-3.3.7/include",
      "vendor/imgui/include",
	  "vendor/stbimage/include",
	  "vendor/simpleini/include",
	  "vendor/simdjson/include",
	  "vendor/assimp/include",
	  "vendor/entt/include",
	  "vendor/GammaHUD/GHUDCore/include",
	  "vendor/GammaHUD/GHUDVulkan/include",
   }
   
   libdirs {
      VULKAN_SDK.."/Lib",
	  "vendor/glfw-3.3.7/lib-vc2019", 
	  "vendor/imgui/lib",
	  "vendor/assimp/lib",
	  "vendor/simdjson/lib",
	  "vendor/GammaHUD/GHUDCore/lib",
	  "vendor/GammaHUD/GHUDVulkan/lib"
   }
   links {
      "vulkan-1",
	  "glfw3",
   }

   filter { "system:windows" }
      links {
	   "shlwapi"
      }
	
   filter "configurations:Debug"
      defines { "_DEBUG" }
      runtime "Debug"
      symbols "On"
	  links {
	  "ImGui_d",
	  "GHUDCore_d",
	  "GHUDVulkan_d",
	  "assimp-vc143-mtd",
	  "simdjson_d",
	  }

   filter "configurations:Release"
	  defines { "NDEBUG" }
      runtime "Release"
      optimize "Speed"
	  links {
	  "ImGui",
	  "GHUDCore",
	  "GHUDVulkan",
	  "assimp-vc143-mt",
	  "simdjson",
	  }