#include <jni.h>
#include <android/log.h>

jboolean Java_com_ubercaster_receiver_NativeReceiver_init(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_init()");
    return JNI_TRUE;
}

void Java_com_ubercaster_receiver_NativeReceiver_start(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_start()");
}

void Java_com_ubercaster_receiver_NativeReceiver_stop(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_stop()");
}

void Java_com_ubercaster_receiver_NativeReceiver_terminate(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_terminate()");
}
