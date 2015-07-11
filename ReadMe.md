# Environs and App examples

---------------------------
*  Created on: 16.05.2013
*      Author: Chi-Tai Dang, dang@hcm-lab.de

# Table of Contents

-----------------------------------
  1. [Download and Compile](#compile)
  2. [Directory Structure and Contents](#dirstruct)


## 1. Download and Compile<a name="compile"></a>

-----------------------------------
1. **Requirements**
<br>
We strongly recommend a working git environment.
If you're on OSX or Linux, then you're safe. 
If you're on Windows, then make sure that you have a working Git Bash, e.g. [Git Extensions](http://sourceforge.net/projects/gitextensions/)


2. **Grab a copy of Environs**
<br>
Download an archive of Environs (e.g. [latest stable release](https://hcm-lab.de/git/environs/environs/repository/archive.zip?ref=0.6.0)) and extract the archive.
You may also clone the git repository:
```
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
```
> cd Environs
> 3rd/download.sh
```

4. **Copy Environs Android layer to examples**
<br>
If you want to run the Android examples, then the environs library needs to be copied into the libs folder of the examples.
We provide a script that automatically does this for you.
```
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


## 2. Directory Structure and Contents<a name="dirstruct"></a>

-----------------------------------

#### [3rd](3rd)
3rd party folder


#### [Androids](Androids)
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

