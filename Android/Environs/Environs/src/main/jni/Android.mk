LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MAIN_PATH := $(LOCAL_PATH)

ASPRE  := ../../../../../../
ifeq ($(ECLIPSE),1)
    ASPRE := ../../../
endif

COMSRC := $(ASPRE)Common/
NATSRC := $(ASPRE)Native/
EXTROOT := $(ASPRE)3rd/
EXTSRC := $(EXTROOT)inc/
RELFLAG :=       
#RELFLAG := -DNDEBUG
FFM_LOAD := 0
LIBX264_LOAD := 0
ANDROID_SLIB := 3rd/lib/android/
ANDROID_SSRC := 3rd/inc/android/


##########################################################################################
# Environs base
##########################################################################################
#
LOCAL_MODULE    := Environs
LOCAL_SRC_FILES := $(NATSRC)Environs.Obj.cpp $(NATSRC)Core/Touch.Source.cpp $(COMSRC)Mediator.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Byte.Buffer.cpp $(NATSRC)Portal/Portal.Stream.cpp
LOCAL_SRC_FILES += $(NATSRC)Portal/Portal.Receiver.cpp $(NATSRC)Portal/Portal.Receiver.Android.cpp
LOCAL_SRC_FILES += $(NATSRC)Portal/Portal.Device.cpp $(NATSRC)Device/Device.Mobile.cpp
LOCAL_SRC_FILES += $(NATSRC)Device/Device.Base.cpp $(NATSRC)Device/Device.Android.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Notifications.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Core.cpp $(NATSRC)Core/Kernel.Mobile.cpp $(NATSRC)Core/Wifi.Observer.cpp
LOCAL_SRC_FILES += $(COMSRC)Queue.Vector.cpp $(NATSRC)Core/Queue.List.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Kernel.Mobile.Platform.cpp $(NATSRC)Core/Kernel.cpp
LOCAL_SRC_FILES += $(NATSRC)Device/Devices.cpp $(NATSRC)Core/Utils.cpp $(COMSRC)Interop/Threads.cpp
LOCAL_SRC_FILES += $(COMSRC)Interop/Export.cpp $(NATSRC)Core/Mediator.Client.cpp
LOCAL_SRC_FILES += $(NATSRC)Environs.Lib.cpp $(NATSRC)Environs.Lib.Mobile.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Callbacks.cpp $(NATSRC)Environs.Android.cpp
LOCAL_SRC_FILES += $(NATSRC)Core/Stunt.Request.cpp $(COMSRC)Environs.Types.cpp
LOCAL_SRC_FILES += $(NATSRC)Decoder/Decoder.Base.cpp $(COMSRC)Environs.Utils.cpp
LOCAL_SRC_FILES += $(COMSRC)Environs.Crypt.cpp $(COMSRC)Environs.Crypt.Lib.cpp $(NATSRC)Environs.Mobile.cpp
LOCAL_SRC_FILES += $(COMSRC)Environs.Crypt1.cpp $(NATSRC)Core/Async.Worker.cpp
LOCAL_SRC_FILES += $(NATSRC)Environs.Sensors.cpp
LOCAL_SRC_FILES += $(NATSRC)Encoder/Encoder.And.cpp $(COMSRC)Interfaces/IPortal.Encoder.cpp
LOCAL_SRC_FILES += $(COMSRC)Log.cpp $(COMSRC)Interfaces/IPortal.Renderer.cpp
LOCAL_SRC_FILES += $(COMSRC)Environs.Commit.cpp $(COMSRC)Tracer.cpp

##########################################################################################
# Build support for native openssl
##########################################################################################
OPENSSL_MODULE=$(EXTSRC)openssl/evp.h

ifneq ("$(wildcard $(OPENSSL_MODULE))","")
    LOCAL_SRC_FILES += $(EXTROOT)DynLib/Dyn.Lib.Crypto.cpp
endif



LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lz -landroid -ldl -latomic
LOCAL_C_INCLUDES += $(COMSRC) $(NATSRC) $(NATSRC)/Codec $(EXTSRC) $(EXTROOT)
LOCAL_CFLAGS += -g -DHAVE_SYS_UIO_H -DENVIRONS_CORE_LIB $(RELFLAG)

#LOCAL_C_INCLUDES += ${NDKROOT}/sources/cxx-stl/stlport/stlport 
#LOCAL_C_INCLUDES += ${NDKROOT}/sources/cxx-stl/llvm-libc++/libcxx/include
#LOCAL_SHARED_LIBRARIES := libstlport 
#LOCAL_SHARED_LIBRARIES += -Llibs/armabi libutils 

##########################################################################################
# Camera capture: Required for a PortalGenerator
##########################################################################################
# 
LOCAL_SRC_FILES += $(NATSRC)Portal/Portal.Generator.cpp $(NATSRC)Portal/Portal.Generator.Mobile.cpp
LOCAL_SRC_FILES += $(NATSRC)Portal/Portal.Generator.Android.cpp $(COMSRC)Interfaces/IEnvirons.Base.cpp
LOCAL_SRC_FILES += $(NATSRC)Recognizer/Recognizers.cpp
#LOCAL_SRC_FILES += $(NATSRC)Recognizer/Gesture.Bezel.Touch.cpp $(NATSRC)Recognizer/Gesture.Three.Touch.cpp
#$(NATSRC)Capture/AndroidScreen.cpp


include $(BUILD_SHARED_LIBRARY)



##########################################################################################
# Build dummy touch recognizer module
##########################################################################################
#include $(LOCAL_PATH)/Recognizer/Dummy/Android.mk

##########################################################################################
# Build touch recognizer modules
##########################################################################################
include $(LOCAL_PATH)/Recognizer/Gesture.Three.Touch/Android.mk
include $(LOCAL_PATH)/Recognizer/Gesture.Bezel.Touch/Android.mk


##########################################################################################
# Build Camera capture module
##########################################################################################
CAM_MODULE=$(NATSRC)Capture/Cam.Android1.cpp

ifneq ("$(wildcard $(CAM_MODULE))","")
    include $(LOCAL_PATH)/Capture/AndroidCam/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCam4.4/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCam4.1.1/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCam4.3/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCam3/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCam2.2/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCamInfo4.4/Android.mk
    include $(LOCAL_PATH)/Capture/AndroidCamInfo4.2/Android.mk
endif

##########################################################################################
# Build OpenHh264 encoder module
##########################################################################################
include $(LOCAL_PATH)/Encoder/LibOpenH264/Android.mk


##########################################################################################
# Build OpenHh264 decoder module
##########################################################################################
include $(LOCAL_PATH)/Decoder/LibOpenH264/Android.mk


##########################################################################################
# Build Android native decoder module
##########################################################################################
include $(LOCAL_PATH)/Decoder/Android/Android.mk


HW264_MODULE=$(LOCAL_PATH)/$(EXTROOT)lib/android/4.3/libgui1.so

ifneq ("$(wildcard $(HW264_MODULE))","")
    ##########################################################################################
    # Build hardware h264 encoder module
    ##########################################################################################
    include $(LOCAL_PATH)/Encoder/HwH264/Android.mk
    include $(LOCAL_PATH)/Encoder/HwH264.4.2/Android.mk
    include $(LOCAL_PATH)/Encoder/HwH264.2.3/Android.mk


    ##########################################################################################
    # Build hardware h264 decoder module
    ##########################################################################################
    # Fails to build
    #include $(LOCAL_PATH)/Decoder/HwH264/Android.mk
endif


##########################################################################################
# Include 3rd party libraries
##########################################################################################

#$(warning $(TARGET_ARCH))

#OH264_MODULE=$(LOCAL_PATH)/$(EXTROOT)lib/android/arm/libopenh264.so

#ifneq ("$(wildcard $(OH264_MODULE))","")
#    LOCAL_PATH := $(MAIN_PATH)

#    include $(CLEAR_VARS)

#    MOD_PATH := $(LOCAL_PATH)/$(EXTROOT)lib/android/arm/libopenh264.mk
#    ifeq ($(LIBX264_LOAD),1)
#        include $(MOD_PATH)
#    endif
#endif
