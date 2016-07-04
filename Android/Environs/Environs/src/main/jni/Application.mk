APP_STL := gnustl_shared
#APP_STL := c++_shared
#APP_STL := stlport_shared

APP_CPPFLAGS := -fno-rtti -fexceptions -Wl,--threads -Wno-deprecated-register -std=c++11
#-fno-exceptions
#-frtti
NDK_TOOLCHAIN_VERSION=4.8
APP_PLATFORM := android-19
#STLPORT_FORCE_REBUILD := true
APP_ABI := armeabi armeabi-v7a x86