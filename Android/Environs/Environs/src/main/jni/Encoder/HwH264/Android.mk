include $(CLEAR_VARS)

##########################################################################################
# Build camera capture shared library 
##########################################################################################
#

LOCAL_MODULE    := Env-EncHwH264

ANDROID_SLVER := 4.4
ANDROID_LIBS := $(ASPRE)$(ANDROID_SLIB)$(ANDROID_SLVER)
ANDROID_SRC := $(ASPRE)$(ANDROID_SSRC)$(ANDROID_SLVER)
ANDROID_FW := av
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

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid -ljnigraphics
LOCAL_LDLIBS += -L$(ANDROID_LIBS) -lstagefright -lutils -lbinder

LOCAL_SHARED_LIBRARIES := libutils libEnvirons

LOCAL_SRC_FILES := $(NATSRC)Encoder/Encoder.Hw.H264.And.cpp $(NATSRC)Codec/Media.Buffer.Source.cpp
LOCAL_SRC_FILES += $(NATSRC)Encoder/Encoder.Base.cpp $(COMSRC)Interfaces/IPortal.Encoder.cpp

include $(BUILD_SHARED_LIBRARY)
