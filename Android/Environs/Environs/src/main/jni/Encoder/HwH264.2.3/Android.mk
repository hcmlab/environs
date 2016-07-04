include $(CLEAR_VARS)

##########################################################################################
# Build Encoder Creator shared library 
##########################################################################################
#

LOCAL_MODULE    := Env-EncHwH264.2.3

ANDROID_SLVER := 2.3.7
ANDROID_LIBS := $(ASPRE)$(ANDROID_SLIB)$(ANDROID_SLVER)
ANDROID_SRC := $(ASPRE)$(ANDROID_SSRC)$(ANDROID_SLVER)
ANDROID_FW := base
ANDROID_FWBASE := $(ANDROID_SRC)/frameworks/$(ANDROID_FW)
ANDROID_FWINC := $(ANDROID_SRC)/frameworks/$(ANDROID_FW)/include


LOCAL_C_INCLUDES := $(COMSRC) $(NATSRC) $(NATSRC)/Codec $(EXTSRC)

LOCAL_C_INCLUDES += $(ANDROID_SRC)/system/core/include
LOCAL_C_INCLUDES += $(ANDROID_SRC)/frameworks/native/include
LOCAL_C_INCLUDES += $(ANDROID_SRC)/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(ANDROID_SRC)/hardware/libhardware/include
LOCAL_C_INCLUDES += $(ANDROID_FWBASE)/media/libstagefright
LOCAL_C_INCLUDES += $(ANDROID_FWINC)
LOCAL_C_INCLUDES += $(ANDROID_FWINC)/media/stagefright
LOCAL_C_INCLUDES += $(ANDROID_FWINC)/media/stagefright/openmax
LOCAL_CFLAGS += -Wno-multichar -DHAVE_SYS_UIO_H -DENVIRONS_MODULE -DENVIRONS_NATIVE_MODULE $(RELFLAG)

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
LOCAL_LDLIBS += -L$(ANDROID_LIBS) -lstagefright -lstagefright_omx -lutils

LOCAL_SRC_FILES := $(NATSRC)Encoder/Encoder.Hw.H264.And.2.3.cpp


include $(BUILD_SHARED_LIBRARY)
