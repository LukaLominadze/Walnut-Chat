@echo off

pushd ..
IF EXIST "vendor\vcpkg\" (
	echo vcpkg already exists
	Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build.lua vs2022
	Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build-Headless.lua vs2022
) ELSE (
	git clone https://github.com/microsoft/vcpkg.git vendor\vcpkg
	vendor\vcpkg\bootstrap-vcpkg.bat
	vendor\vcpkg\vcpkg integrate install
	vendor\vcpkg\vcpkg install curl --x-install-root="vendor\build"
	xcopy /E /I /Y "vendor\build\x64-windows" "vendor\curl"
    	rmdir /S /Q "vendor\build\x64-windows"
	Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build.lua vs2022
	Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build-Headless.lua vs2022
	pause
)
popd
pause