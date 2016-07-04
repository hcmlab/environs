# Environs and App examples

---------------------------
*  Created on: 16.05.2013
*      Author: Chi-Tai Dang, dang@hcm-lab.de

# Table of Contents

-----------------------------------
  1. [Download and Compile](#compile)
  2. [Directory Structure and Contents](#dirstruct)
  3. [Windows Build Notices](#buildWindows)<br/>
  3.1 [Choose Platform before Build](#choosePlatform)<br/>
  3.2 [Collector Solutions](collector)<br/>
  3.3 [Example Solution](#winExamples)<br/>
  3.4 [Working Directory](#workDir)<br/>
  3.5 [Surface and PixelSense Tabletop](#tabletop)<br/>
  3.6 [Visual Studio 2010 Solution](#vs2010)<br/>
  3.7 [Why bin / bin64 / libs?](#binbin64)<br/>  
  4. [*NIX Build instructions](#build)<br/>
  4.1 [OSX](#buildOSX)<br/>
  4.2 [Linux](#buildLinux)<br/>
  4.3 [Raspberry PI](#buildRaspberry)<br/>
  

# 1. Download and Compile<a name="compile"></a>

-----------------------------------
1. **Requirements**
<br>
We strongly recommend a working git environment.
If you're on OSX or Linux, then you're safe. 
If you're on Windows, then make sure that you have a working Git Bash, e.g. [Git Extensions](http://sourceforge.net/projects/gitextensions/)


2. **Grab a copy of Environs**
<br>
Download an archive of Environs (e.g. [latest release](https://hcm-lab.de/git/environs/environs/repository/archive.zip)) and extract the archive.
You may also clone the git repository:
```bash
> git clone https://hcm-lab.de/environs.git
```
or
```bash
> git clone https://hcm-lab.de/git/environs/environs.git
```

3. **3rd party headers and libraries**
<br>
In order to compile Environs native layer, external 3rd party headers and libraries need to be downloaded.
We provide helper scripts in the [Tools](Tools) folder to do this automatically for you. 
The helper scripts are designed for a git bash (Windows) or the a *nix bash (OSX/Linux).
<br>
The scripts can be invoked all together by means of the main helper [download.sh](3rd/download.sh) 
which downloads Environs prebuilt binaries, required headers, sources, and libraries such as OpenCL, or openh264 from available sources.
<br>
In what follows, we assume that the root folder of your extracted archive or cloned repository is named "Environs".
```bash
> cd Environs
> 3rd/download.sh
```

4. **Copy Environs Android layer to examples**
<br>
If you want to run the Android examples, then the environs library needs to be copied into the libs folder of the examples.
We provide a script that automatically does this for you.
```bash
> cd Environs
> Tools/prepare.android.apps.sh
```

5. **Done**
<br>
That's it. Your local branch of Environs is ready. 
For next steps to work with Environs, please visit the home of Environs http://hcm-lab.de/environs
<br>
Depending on the platform that your project targets, things are different a little bit.
The provided tutorials will help you further.


# 2. Directory Structure and Contents<a name="dirstruct"></a>

-----------------------------------

#### [3rd](3rd)
3rd party folder


#### [Android](Android)
Android Studio (gradle based) projects and example applications.


#### [bin](bin) / [bin64](bin64)
Working directory for Windows 32/64 bit and OSX (64 bit)
<br>
Shows an example running environment with the required libraries.


#### [Common](Common)
Header files and tools that are commonly used by all platforms.


#### [iOSX](iOSX)
iOS and OSX projects. Environs library and applications.


#### [libs](libs)
iOS and Android libraries.


#### [Mediator](Mediator)
Mediator root directory for implementations.


#### [Native](Native)
Source code of native layer that is used by all platforms.


#### [Windows](Windows)
Windows environs and example applications.


-----------------------------------


# 3. Windows Build Notices<a name="buildWindows">

-----------------------------------

All Windows related projects and solutions are located in the subfolder [Windows](Windows) of the Environs repository. Within this directory, there are several solutions for building the library as well as for the examples.
The naming of the solutions (e.g. Environs.2015.sln) hints at the targetd Visual Studio version (e.g. VS 20150). If the name does not contain a number, then the solution targets the lowest supported Visual Studio which is (currently) Visual Studio 2010.

#### **Choose Platform before Build**<a name="choosePlatform">
**IMPORTANT:** Before you start any build, you should choose the desired platform and configuration, e.g. Release | x64. We configured only the platforms x86 (32 bit) and x64 (64 bit).<br/>

All other platforms and configurations are *NOT* guaranteed to be preconfigured and working. If you need for example "Any CPU", then you must configure that yourself.

#### **Collector Solutions** <a name="collector">
The solutions [Environs.sln](Windows/Environs.sln), [Environs.2013.sln](Windows/Environs.2013.sln), and [Environs.2015.sln](Windows/Environs.2015.sln) represent collector solutions which include the library project as well as all example projects. Hence, you can build all available projects by means of a collector solution.
However, there are a few exceptions and notices regarding the collector solutions as described in the following.>

#### **Example Solution**<a name="winExamples">
Each of the example solutions have the particular example projects included and requires that the Environs libraries are already built and located in the bin (32 bit) or bin64 (64 bit) folders. Hence, the Environs source projects are not included in the example solutions. <br/>

**IMPORTANT:** In order to successfuly build the example solutions, you have to make sure that the Environs libraries are available in the working directory. One way to do this is to download the prebuilt binaries (from the binaries repository) by means of a download helper script as shown in the following:
```bash
> cd Environs
> Tools/download.binaries.sh
```

You may also build the Environs libraries using the [Collector Solutions](collector). In this case, you have to make sure that the correct libraries are build, e.g. if you wanna run a Visual Studio 2015 example, then you have to build the Visual Studio 2015 libraries. The reason for this requirement is exlained [below](#binbin64).


##### **Working Directory**<a name="workDir">
Each of the example solutions are configured to run either in the [bin](bin) (32 bit) or [bin64](bin64) (64 bit) folder. You may, of course, choose another working directory. The only requirement is that the libs folder structure and libraries (as given in bin/bin64) are located within your desired working directory.


#### **Surface and PixelSense Tabletop**<a name="tabletop">
All tabletop related libraries and projects (PixelSense.dll, SurfaceExample, etc.) are excluded from all collector solutions by default as you may no necessarily have the required SDKs preinstalled. If you want to build tabletop applications, then just enable the approrpiate projects for the build.


#### **Visual Studio 2010 Solution**<a name="vs2010">
In the collector solution [Environs.sln](Windows/Environs.sln) for Visual Studio 2010, the Environs.NoUI project (for applications that do not assign a Window handle to Environs) requires also an installation of Visual Studio 2008 with x64 compilers installed. The reason behind this circumstance is that the Environs.NoUI.dll in this collector solution will be build with the platform toolkit v90 in order to prevent any dependency on .NET framework >= 4.0.<br/>

In particular, this project builds an Environs library that depends only on .NET framework <= 3.5 to allow, for example, inclusion into Unity games or legacy .NET applications.


#### **Why bin / bin64 / libs?**<a name="binbin64">
The reason why there are separate 32bit / 64bit folders are the native libraries and Environs is for over 90% a native framework. **You cannot load a 32bit library into a 64bit process and vice versa.** 
Therefore, if your application runs as a 64bit process, then you must make sure that all native libraries and modules are 64bit.<br/>

The reason for the libs folder is due to Windows runtime environments and Visual Studio versions. There is a bunch of different Visual Studio versions and supported platform toolkits. If you have one Visual Studio version installed, then it is not guaranteed that you also have a previous or newer platform toolkit and Visual Studio runtime installed. In this case you may not be able to load a native library that was linked against a different platform toolkit.
Of course, if you build all binaries yourself with the same Visual Studio version, then you're safe and do not need separate libs folders. In this case, you can place all libraries and modules into the working directory. The dynamic loader will look there at first. (However, you still need to consider 32/64 bit).<br/>

In order to reduce confusion on developer side (in particular for students with few programming experience), we provide prebuilt binaries for (currently) three supported platform toolkits distributed in separate folders, such as [libs/v100](libs/v100), [libs/v120](libs/v120), and [libs/v140](libs/v140) for both architectures, that is 32bit (bin) and 64bit (bin64).
Those libaries are loaded dynamically depending on which platform toolkit version is acutally available on the target machine.<br/>

**Hint:** If dynamic detection on the target machine fails, you may do it manually by means of a helper script as follows:
```bash
> cd bin
> ./0_prepare_windows_runtime.bat
> cd ../bin64
> ./0_prepare_windows_runtime.bat
```
You may, of course, skip either of the architecture if you focus on only one of the them.


# 4. *NIX Build Instructions<a name="build">

-----------------------------------

## 1. **OSX**<a name="buildOSX">

**Requirements**: 
All requirements for OSX builds are downloaded by means of the [download helper scripts](#compile). Afterwards libEnvirons.so can be build using the following commands:
<br>
```bash
> cd environs/Linux
> make new
> cd ../..
```

In order to build the Simple.Console or Echo.Bot app use these commands:
<br>
```bash
> cd environs/Linux/Simple.Console.CPP
> make new
> cd ../Echo.Bot.CPP
> make new
> cd ../../..
```

The Mediator server can be build using these commands:
<br>
```bash
> cd environs/Mediator
> make new
```


## 2. **Linux**<a name="buildLinux">

**Requirements**: 
In addition to the requirements downloaded by the [download helper scripts](#compile), you may need to install a few additional dependencies. Usually, those are already installed on state of the art *nix distributions:
- openssl
- uuid

The following commands check the dependencies and install from the Linux distribution repositories (if required).
```bash
> cd environs
> Tools/prepare.linux.sh
```

Afterwards libEnvirons.so can be build using the following commands:
```bash
> cd Linux
> make new
```

The commands for building the Simple.Console app and Mediator server are the same as for [OSX platforms](#buildOSX).


## 3. **Raspberry PI**<a name="buildRaspberry">

Environs also runs on the Raspberry PI at least with Raspbian Wheezy and Jessie.

**Requirements**: 
In addition to the requirements downloaded by the [download helper scripts](#compile), you need to install a few additional dependencies, which may or may not be already installed on your Raspbian image:
- openssl
- uuid
- g++-4.7 or g++-4.8

The following helper script checks those dependencies and installs them from the Raspbian distribution repositories (if not already installed).
<br>
```bash
> cd environs
> Tools/prepare.linux.sh

> cd Linux
> make new
```

Afterwards libEnvirons.so, the Simple.Console app, and the Mediator server can be build using the same commands as for [OSX/*nix platforms](#buildOSX).
