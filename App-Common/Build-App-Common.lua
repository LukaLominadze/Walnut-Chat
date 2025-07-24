project "App-Common"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "../Walnut/vendor/imgui",
      "../Walnut/vendor/glfw/include",
      "../Walnut/vendor/glm",
      "../Walnut/vendor/yaml-cpp/include",
      "../Walnut/vendor/nativefiledialog/src/include",
      "../Walnut/vendor/static-bin2header/src",

      "../Walnut/Walnut/Source",
      "../Walnut-Networking/Source",

      "%{IncludeDir.VulkanSDK}",
      "../Walnut/vendor/spdlog/include",

      "../Walnut-Networking/vendor/GameNetworkingSockets/include",
      "../vendor/curl/include"
   }

   links
   {
       "Walnut",
       "Walnut-Networking",
       "Curl",
       "Bin2Header",
       "nfd",
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"
   
   filter "action:vs*"
      buildoptions { "/utf-8" }

   filter { "toolset:gcc or toolset:clang" }
      buildoptions { "-finput-charset=UTF-8" }