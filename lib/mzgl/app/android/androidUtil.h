#pragma once

#include <string>
#include <vector>
#include <functional>
#include <android/log.h>
#include "App.h"
#include "Midi.h"

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "native-activity", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))


#include "android_native_app_glue.h"

bool loadAndroidAsset(const std::string &path, std::vector<unsigned char> &outData);
void listAndroidAssetDir(const std::string &path, std::vector<std::string> &outPaths);

std::string getAndroidInternalDataPath();
std::string getAndroidExternalDataPath();
void androidAlertDialog(const std::string &title, const std::string &msg);
bool androidHasMicPermission();
void androidConfirmDialog(std::string title, std::string msg,
                   std::function<void()> okPressed,
                   std::function<void()> cancelPressed);

void androidTextboxDialog(std::string title,
        std::string msg,
        std::string text,
        std::function<void(std::string, bool)> completionCallback);

void androidTwoOptionCancelDialog(std::string title, std::string msg,
        std::string buttonOneText, std::function<void()> buttonOnePressed,
        std::string buttonTwoText, std::function<void()> buttonTwoPressed,
        std::function<void()> cancelPressed);

void androidThreeOptionCancelDialog(std::string title, std::string msg,
                                    std::string buttonOneText, std::function<void()> buttonOnePressed,
                                    std::string buttonTwoText, std::function<void()> buttonTwoPressed,
                                    std::string buttonThreeText, std::function<void()> buttonThreePressed,
                                    std::function<void()> cancelPressed);

std::string androidGetAppVersionString();
void androidShareDialog(std::string message,
        std::string path,
        std::function<void(bool)> completionCallback);
void androidImageDialog(std::string copyToPath, std::function<void(bool success, std::string imgPath)> completionCallback);
void androidLaunchUrl(const std::string &url);
std::string getAndroidTempDir();
std::vector<std::string> androidGetMidiDeviceNames();

bool isUsingHeadphones();
bool isUsingUSBInterface();
bool isUsingAirplay();
bool isUsingBluetoothHeadphones();

void androidSetupAllMidiIns();
void androidAddMidiListener(MidiListener *listener);

android_app *getAndroidAppPtr();
class EventDispatcher;
EventDispatcher *getAndroidEventDispatcher();
void callJNI(std::string methodName, std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5);
void callJNI(std::string methodName, std::string arg1, std::string arg2, std::string arg3, std::string arg4);
void callJNI(std::string methodName, std::string arg1, std::string arg2, std::string arg3);
void callJNI(std::string methodName, std::string arg1, std::string arg2);
void callJNI(std::string methodName, std::string arg1);
void callJNI(std::string methodName);
void callJNI(std::string methodName, int32_t arg1);
bool callJNIForBoolean(std::string methodName);
bool callJNIForBoolean(std::string methodName, int arg1);
bool callJNIForBoolean(std::string methodName, std::string arg1);
bool callJNIForBoolean(std::string methodName, const std::string &arg1, int arg2);

float callJNIForFloat(std::string methodName);

int64_t callJNIForLong(std::string methodName);
int32_t callJNIForInt(std::string methodName);

std::string callJNIForString(std::string methodName, int32_t arg1);
int64_t callJNIForLong(std::string methodName, int32_t arg1);

void callJNI(std::string methodName, const std::string &arg1, int32_t arg2);

std::string callJNIForString(std::string methodName);
std::string callJNIForString(std::string methodName, std::string arg1);
std::string jstringToString(JNIEnv *jni, jstring text);

// these can be nested - if you have a nested scoped jni, only the outer one
// does anything, and the inner ones are ignored - handy if you need to make
// a block of Jni calls, so you don't keep having to Attach and DetatchCurrentThread
class ScopedJni {
public:
    ScopedJni() {
        auto *appPtr = getAndroidAppPtr();
        if(appPtr==nullptr) {
            success = appPtr->activity->vm->AttachCurrentThread(&jni, nullptr)==JNI_OK;
        }
    }
    JNIEnv *j() {
        return jni;
    }
    jmethodID getMethodID(const std::string &methodName, const std::string &signature) {
        auto *cl = getClass();
        if(cl==nullptr) return nullptr;
        return jni->GetMethodID(cl, methodName.c_str(), signature.c_str());
    }

    jclass getClass() {
        if(!success) return nullptr;
        auto *appPtr = getAndroidAppPtr();
        if(appPtr!=nullptr) {
            jclass clazz = jni->GetObjectClass(appPtr->activity->clazz);

            return clazz;
        }
        return nullptr;
    }
    ~ScopedJni() {
        if(success) {
            getAndroidAppPtr()->activity->vm->DetachCurrentThread();
            jni = nullptr;
        }
    }
private:
    bool success = false;
    JNIEnv *jni;
};


