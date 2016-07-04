include $(CLEAR_VARS)

# Note: You must adapt this path to match with your IDE. The path must point to the common source/include folder
#COMINC := ../../Common
#COMSRC := ../$(COMINC)/

# Note: Native source is optional. This example only uses the log macros CLog, etc.. from the native source
#NATINC := ../../Native

LOCAL_MODULE    := Env-RecGestureThreeTouch

LOCAL_C_INCLUDES := $(COMSRC) $(NATSRC)
LOCAL_CFLAGS += -g -DHAVE_SYS_UIO_H -DENVIRONS_MODULE -DENVIRONS_NATIVE_MODULE $(RELFLAG)
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

# Source files to compile into the module
LOCAL_SRC_FILES := $(NATSRC)Recognizer/Gesture.Three.Touch.cpp

include $(BUILD_SHARED_LIBRARY)

