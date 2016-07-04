include $(CLEAR_VARS)

FFM_LOAD := 1

##########################################################################################
# Build camera capture shared library 
##########################################################################################
#

LOCAL_MODULE    := Env-DecAndroid

LOCAL_C_INCLUDES += $(COMSRC) $(NATSRC) $(NATSRC)/Codec $(EXTROOT) $(EXTSRC)

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_CFLAGS += -DLIBAV -DHAVE_SYS_UIO_H -DENVIRONS_MODULE -DENVIRONS_NATIVE_MODULE  $(RELFLAG)
LOCAL_SRC_FILES := $(NATSRC)Decoder/Decoder.Android.cpp
LOCAL_SRC_FILES += $(COMSRC)Interop/Threads.cpp

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid

include $(BUILD_SHARED_LIBRARY)


