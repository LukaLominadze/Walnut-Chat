project "Curl"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "include/**.h", "include/**.cpp", "src/**.h", "src/**.cpp" }

   includedirs
   {
      "include"
   }

   targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }
      links
      {
         "lib/libcurl.lib",
         "lib/zlib.lib"
      }

   filter "configurations:Debug"
      defines { "WL_DEBUG", "_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE", "NDEBUG" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "WL_DIST", "NDEBUG" }
      runtime "Release"
      optimize "On"
      symbols "Off"