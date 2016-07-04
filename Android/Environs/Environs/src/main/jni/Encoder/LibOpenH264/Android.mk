include $(CLEAR_VARS)

LIBX264_LOAD := 1

##########################################################################################
# Build camera capture shared library
##########################################################################################
#

LOCAL_MODULE    := Env-EncOpenH264

LOCAL_C_INCLUDES += $(COMSRC) $(NATSRC) $(NATSRC)/Codec $(EXTROOT) $(EXTSRC)

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid
LOCAL_LDLIBS += -L$(ANDROID_LIBS)
LOCAL_CFLAGS += -DHAVE_SYS_UIO_H -DENVIRONS_MODULE -DENVIRONS_NATIVE_MODULE $(RELFLAG)

LOCAL_SRC_FILES := $(EXTROOT)Encoder/Encoder.OpenH264.cpp $(EXTROOT)DynLib/Dyn.Lib.OpenH264.cpp
LOCAL_SRC_FILES += $(COMSRC)Interfaces/IPortal.Encoder.cpp $(COMSRC)Interop/Threads.cpp
LOCAL_SRC_FILES += $(NATSRC)Encoder/Encoder.Base.cpp

include $(BUILD_SHARED_LIBRARY)
