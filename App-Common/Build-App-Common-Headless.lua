project "App-Common-Headless"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "../Walnut/vendor/glm",

      "../Walnut/Walnut/Source",
      "../Walnut-Networking/Source",

      "../Walnut/vendor/spdlog/include",

      "../vendor/GameNetworkingSockets/include",
      "../vendor/curl/include"
   }

   links
   {
       "Walnut-Headless",
       "Walnut-Networking",
       "Curl"
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