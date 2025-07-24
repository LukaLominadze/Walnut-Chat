project "App-Client"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "../App-Common/Source",

      "../Walnut/vendor/imgui",
      "../Walnut/vendor/glfw/include",
      "../Walnut/vendor/glm",
      "../Walnut/vendor/yaml-cpp/include",
      "../Walnut/vendor/nativefiledialog/src/include",
      "../Walnut/vendor/static-bin2header/src",

      "../Walnut/Walnut/Source",
      "../Walnut/Walnut/Platform/GUI",

      "%{IncludeDir.VulkanSDK}",
      "../Walnut/vendor/spdlog/include",
      "../Walnut/vendor/yaml-cpp/include",
      
      -- Walnut-Networking
      "../Walnut/Walnut-Modules/Walnut-Networking/Source",
      "../Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/include"
   }

   links
   {
       "App-Common",

       "yaml-cpp",
       "nfd",
       "Bin2Header",
   }

   	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/GameNetworkingSockets.dll" "%{cfg.targetdir}"',
	    '{COPY} "../%{WalnutNetworkingBinDir}/libcrypto-3-x64.dll" "%{cfg.targetdir}"',
	  }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      links { "../Walnut/vendor/static-bin2header/bin/" .. outputdir .. "/Bin2Header/Bin2Header_d.lib" }
      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/libprotobufd.dll" "%{cfg.targetdir}"'
	  }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      links { "../Walnut/vendor/static-bin2header/bin/" .. outputdir .. "/Bin2Header/Bin2Header.lib" }
      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/libprotobuf.dll" "%{cfg.targetdir}"'
	  }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "WL_DIST" }
      links { "../Walnut/vendor/static-bin2header/bin/" .. outputdir .. "/Bin2Header/Bin2Header.lib" }
      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/libprotobuf.dll" "%{cfg.targetdir}"'
	  }
      runtime "Release"
      optimize "On"
      symbols "Off"
   
   filter "action:vs*"
      buildoptions { "/utf-8" }

   filter { "toolset:gcc or toolset:clang" }
      buildoptions { "-finput-charset=UTF-8" }