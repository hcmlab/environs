include $(CLEAR_VARS)

FFM_LOAD := 1

##########################################################################################
# Build camera capture shared library 
##########################################################################################
#

LOCAL_MODULE    := Env-DecOpenH264

LOCAL_SHARED_LIBRARIES := libutils
LOCAL_C_INCLUDES += $(COMSRC) $(NATSRC) $(NATSRC)/Codec $(EXTROOT) $(EXTSRC)


LOCAL_CFLAGS += -DLIBAV -DHAVE_SYS_UIO_H -DENVIRONS_MODULE -DENVIRONS_NATIVE_MODULE $(RELFLAG)
LOCAL_SRC_FILES := $(EXTROOT)Decoder/Decoder.OpenH264.cpp $(EXTROOT)DynLib/Dyn.Lib.OpenH264.cpp
LOCAL_SRC_FILES += $(NATSRC)Decoder/Decoder.Base.cpp $(COMSRC)Interop/Threads.cpp

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid

include $(BUILD_SHARED_LIBRARY)


