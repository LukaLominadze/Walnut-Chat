# Walnut Chat

Walnut Chat is a simple client/server text chat app built with [Walnut](https://github.com/StudioCherno/Walnut) and the [Walnut-Networking](https://github.com/StudioCherno/Walnut-Networking) module. The server currently runs on both Windows (GUI/headless) and Linux (headless only), and the client is Windows only at this stage (Linux support coming soon).

This was built as a demonstration of networking in C++ for a video on my YouTube channel. **There is no security** so be careful! Definitely don't run this as root on your server/computer and there certainly isn't any message encryption. [Watch the video here.](https://youtu.be/jS9rBienEFQ)


![WalnutExample](https://hazelengine.com/images/WalnutChat.jpg)
_<center>Walnut Chat Client</center>_

## Dependencies
- Api
(you will need dotnet to be able to generate and run the web api for the server, if you are using an IDE (like Visual Studio) you will need the tools to be able to work with ASP.NET Core Web Api)
- MySQL

## Building
### Windows
Running `scripts/Setup.bat` will generate `Walnut-Chat.sln` solution file for Visual Studio 2022.
Make sure to change the connection string in "Walnut-Chat-Api\Walnut.Api\appsettings.json" to your database's connection string.

### Linux (tested on Ubuntu 22)
Currenty developing support for Linux...
