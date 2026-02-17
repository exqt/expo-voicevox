#include <jni.h>
#include <string>
#include <cstdio>
#include <android/log.h>
#include "voicevox_core.h"

#define LOG_TAG "VoicevoxJNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const VoicevoxOnnxruntime *g_onnxruntime = nullptr;
static OpenJtalkRc *g_open_jtalk = nullptr;
static VoicevoxSynthesizer *g_synthesizer = nullptr;

static void throwVoicevoxException(JNIEnv *env, VoicevoxResultCode code) {
    const char *message = voicevox_error_result_to_message(code);
    jclass exClass = env->FindClass("java/lang/RuntimeException");
    env->ThrowNew(exClass, message);
}

extern "C" {

JNIEXPORT void JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeInitialize(
    JNIEnv *env, jobject /* this */,
    jint accelerationMode, jint cpuNumThreads,
    jstring openJtalkDictDir) {

    // Load ONNX Runtime
    if (g_onnxruntime == nullptr) {
        VoicevoxLoadOnnxruntimeOptions ortOptions = voicevox_make_default_load_onnxruntime_options();
        VoicevoxResultCode result = voicevox_onnxruntime_load_once(ortOptions, &g_onnxruntime);
        if (result != VOICEVOX_RESULT_OK) {
            throwVoicevoxException(env, result);
            return;
        }
    }

    // Create OpenJtalk
    const char *dictDir = env->GetStringUTFChars(openJtalkDictDir, nullptr);
    if (g_open_jtalk != nullptr) {
        voicevox_open_jtalk_rc_delete(g_open_jtalk);
        g_open_jtalk = nullptr;
    }
    VoicevoxResultCode result = voicevox_open_jtalk_rc_new(dictDir, &g_open_jtalk);
    env->ReleaseStringUTFChars(openJtalkDictDir, dictDir);
    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
        return;
    }

    // Create Synthesizer
    if (g_synthesizer != nullptr) {
        voicevox_synthesizer_delete(g_synthesizer);
        g_synthesizer = nullptr;
    }
    VoicevoxInitializeOptions options = voicevox_make_default_initialize_options();
    options.acceleration_mode = static_cast<VoicevoxAccelerationMode>(accelerationMode);
    options.cpu_num_threads = static_cast<uint16_t>(cpuNumThreads);

    result = voicevox_synthesizer_new(g_onnxruntime, g_open_jtalk, options, &g_synthesizer);
    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
    }
}

JNIEXPORT void JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeLoadModel(
    JNIEnv *env, jobject /* this */, jstring vvmPath) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return;
    }

    const char *pathStr = env->GetStringUTFChars(vvmPath, nullptr);

    VoicevoxVoiceModelFile *model = nullptr;
    VoicevoxResultCode result = voicevox_voice_model_file_open(pathStr, &model);
    env->ReleaseStringUTFChars(vvmPath, pathStr);

    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
        return;
    }

    result = voicevox_synthesizer_load_voice_model(g_synthesizer, model);
    voicevox_voice_model_file_delete(model);

    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
    }
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeAudioQuery(
    JNIEnv *env, jobject /* this */,
    jstring text, jint styleId) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return nullptr;
    }

    const char *textStr = env->GetStringUTFChars(text, nullptr);

    char *outputJson = nullptr;
    VoicevoxResultCode result = voicevox_synthesizer_create_audio_query(
        g_synthesizer, textStr, static_cast<VoicevoxStyleId>(styleId), &outputJson);

    env->ReleaseStringUTFChars(text, textStr);

    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
        return nullptr;
    }

    jstring jsonResult = env->NewStringUTF(outputJson);
    voicevox_json_free(outputJson);
    return jsonResult;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeAudioQueryFromKana(
    JNIEnv *env, jobject /* this */,
    jstring kana, jint styleId) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return nullptr;
    }

    const char *kanaStr = env->GetStringUTFChars(kana, nullptr);

    char *outputJson = nullptr;
    VoicevoxResultCode result = voicevox_synthesizer_create_audio_query_from_kana(
        g_synthesizer, kanaStr, static_cast<VoicevoxStyleId>(styleId), &outputJson);

    env->ReleaseStringUTFChars(kana, kanaStr);

    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
        return nullptr;
    }

    jstring jsonResult = env->NewStringUTF(outputJson);
    voicevox_json_free(outputJson);
    return jsonResult;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeSynthesis(
    JNIEnv *env, jobject /* this */,
    jstring audioQueryJson, jint styleId, jstring outputPath,
    jboolean enableInterrogativeUpspeak) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return nullptr;
    }

    const char *queryStr = env->GetStringUTFChars(audioQueryJson, nullptr);
    const char *pathStr = env->GetStringUTFChars(outputPath, nullptr);

    VoicevoxSynthesisOptions options = voicevox_make_default_synthesis_options();
    options.enable_interrogative_upspeak = enableInterrogativeUpspeak;

    uintptr_t wavLength = 0;
    uint8_t *wavData = nullptr;

    VoicevoxResultCode result = voicevox_synthesizer_synthesis(
        g_synthesizer, queryStr, static_cast<VoicevoxStyleId>(styleId),
        options, &wavLength, &wavData);

    env->ReleaseStringUTFChars(audioQueryJson, queryStr);

    if (result != VOICEVOX_RESULT_OK) {
        env->ReleaseStringUTFChars(outputPath, pathStr);
        throwVoicevoxException(env, result);
        return nullptr;
    }

    FILE *file = fopen(pathStr, "wb");
    if (file) {
        fwrite(wavData, 1, wavLength, file);
        fclose(file);
    } else {
        LOGE("Failed to open file for writing: %s", pathStr);
        voicevox_wav_free(wavData);
        env->ReleaseStringUTFChars(outputPath, pathStr);
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Failed to write WAV file");
        return nullptr;
    }

    voicevox_wav_free(wavData);

    jstring resultPath = env->NewStringUTF(pathStr);
    env->ReleaseStringUTFChars(outputPath, pathStr);
    return resultPath;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeTts(
    JNIEnv *env, jobject /* this */,
    jstring text, jint styleId, jstring outputPath,
    jboolean enableInterrogativeUpspeak) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return nullptr;
    }

    const char *textStr = env->GetStringUTFChars(text, nullptr);
    const char *pathStr = env->GetStringUTFChars(outputPath, nullptr);

    VoicevoxTtsOptions options = voicevox_make_default_tts_options();
    options.enable_interrogative_upspeak = enableInterrogativeUpspeak;

    uintptr_t wavLength = 0;
    uint8_t *wavData = nullptr;

    VoicevoxResultCode result = voicevox_synthesizer_tts(
        g_synthesizer, textStr, static_cast<VoicevoxStyleId>(styleId),
        options, &wavLength, &wavData);

    env->ReleaseStringUTFChars(text, textStr);

    if (result != VOICEVOX_RESULT_OK) {
        env->ReleaseStringUTFChars(outputPath, pathStr);
        throwVoicevoxException(env, result);
        return nullptr;
    }

    FILE *file = fopen(pathStr, "wb");
    if (file) {
        fwrite(wavData, 1, wavLength, file);
        fclose(file);
    } else {
        LOGE("Failed to open file for writing: %s", pathStr);
        voicevox_wav_free(wavData);
        env->ReleaseStringUTFChars(outputPath, pathStr);
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Failed to write WAV file");
        return nullptr;
    }

    voicevox_wav_free(wavData);

    jstring resultPath = env->NewStringUTF(pathStr);
    env->ReleaseStringUTFChars(outputPath, pathStr);
    return resultPath;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeTtsFromKana(
    JNIEnv *env, jobject /* this */,
    jstring kana, jint styleId, jstring outputPath,
    jboolean enableInterrogativeUpspeak) {

    if (g_synthesizer == nullptr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Synthesizer not initialized");
        return nullptr;
    }

    const char *kanaStr = env->GetStringUTFChars(kana, nullptr);
    const char *pathStr = env->GetStringUTFChars(outputPath, nullptr);

    VoicevoxTtsOptions options = voicevox_make_default_tts_options();
    options.enable_interrogative_upspeak = enableInterrogativeUpspeak;

    uintptr_t wavLength = 0;
    uint8_t *wavData = nullptr;

    VoicevoxResultCode result = voicevox_synthesizer_tts_from_kana(
        g_synthesizer, kanaStr, static_cast<VoicevoxStyleId>(styleId),
        options, &wavLength, &wavData);

    env->ReleaseStringUTFChars(kana, kanaStr);

    if (result != VOICEVOX_RESULT_OK) {
        env->ReleaseStringUTFChars(outputPath, pathStr);
        throwVoicevoxException(env, result);
        return nullptr;
    }

    FILE *file = fopen(pathStr, "wb");
    if (file) {
        fwrite(wavData, 1, wavLength, file);
        fclose(file);
    } else {
        LOGE("Failed to open file for writing: %s", pathStr);
        voicevox_wav_free(wavData);
        env->ReleaseStringUTFChars(outputPath, pathStr);
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Failed to write WAV file");
        return nullptr;
    }

    voicevox_wav_free(wavData);

    jstring resultPath = env->NewStringUTF(pathStr);
    env->ReleaseStringUTFChars(outputPath, pathStr);
    return resultPath;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeGetVersion(
    JNIEnv *env, jobject /* this */) {
    const char *version = voicevox_get_version();
    return env->NewStringUTF(version);
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeGetMetasJson(
    JNIEnv *env, jobject /* this */) {
    if (g_synthesizer == nullptr) {
        return env->NewStringUTF("[]");
    }
    char *metas = voicevox_synthesizer_create_metas_json(g_synthesizer);
    jstring result = env->NewStringUTF(metas);
    voicevox_json_free(metas);
    return result;
}

JNIEXPORT jstring JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeGetSupportedDevicesJson(
    JNIEnv *env, jobject /* this */) {
    if (g_onnxruntime == nullptr) {
        return env->NewStringUTF("{}");
    }
    char *devices = nullptr;
    VoicevoxResultCode result = voicevox_onnxruntime_create_supported_devices_json(
        g_onnxruntime, &devices);
    if (result != VOICEVOX_RESULT_OK) {
        throwVoicevoxException(env, result);
        return nullptr;
    }
    jstring jsonResult = env->NewStringUTF(devices);
    voicevox_json_free(devices);
    return jsonResult;
}

JNIEXPORT jboolean JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeIsGpuMode(
    JNIEnv *env, jobject /* this */) {
    if (g_synthesizer == nullptr) {
        return JNI_FALSE;
    }
    return voicevox_synthesizer_is_gpu_mode(g_synthesizer) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_expo_modules_voicevox_VoicevoxBridge_nativeFinalize(
    JNIEnv *env, jobject /* this */) {
    if (g_synthesizer != nullptr) {
        voicevox_synthesizer_delete(g_synthesizer);
        g_synthesizer = nullptr;
    }
    if (g_open_jtalk != nullptr) {
        voicevox_open_jtalk_rc_delete(g_open_jtalk);
        g_open_jtalk = nullptr;
    }
    // onnxruntime is a singleton, don't delete
}

} // extern "C"
