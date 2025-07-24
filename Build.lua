-- premake5.lua
workspace "Walnut-Chat"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App-Client"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

-- Directories
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
WalnutNetworkingBinDir = "Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/%{cfg.system}/%{cfg.buildcfg}/"

include "Walnut/Build-Walnut-Headless-External.lua"
include "Walnut/Build-Walnut-External.lua"

group "Core"
    include "vendor/curl/Build-Curl.lua"
group ""

group "App"
    include "App-Common/Build-App-Common.lua"
    include "App-Common/Build-App-Common-Headless.lua"
    include "App-Client/Build-App-Client.lua"
    include "App-Server/Build-App-Server-Headless.lua"
group ""