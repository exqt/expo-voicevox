package expo.modules.voicevox

class VoicevoxBridge {
    companion object {
        init {
            System.loadLibrary("voicevox_onnxruntime")
            System.loadLibrary("voicevox_core")
            System.loadLibrary("voicevox_jni")
        }

        val instance = VoicevoxBridge()
    }

    external fun nativeInitialize(
        accelerationMode: Int,
        cpuNumThreads: Int,
        openJtalkDictDir: String
    )

    external fun nativeLoadModel(vvmPath: String)

    external fun nativeAudioQuery(
        text: String,
        styleId: Int
    ): String

    external fun nativeAudioQueryFromKana(
        kana: String,
        styleId: Int
    ): String

    external fun nativeSynthesis(
        audioQueryJson: String,
        styleId: Int,
        outputPath: String,
        enableInterrogativeUpspeak: Boolean
    ): String

    external fun nativeTts(
        text: String,
        styleId: Int,
        outputPath: String,
        enableInterrogativeUpspeak: Boolean
    ): String

    external fun nativeTtsFromKana(
        kana: String,
        styleId: Int,
        outputPath: String,
        enableInterrogativeUpspeak: Boolean
    ): String

    external fun nativeGetVersion(): String
    external fun nativeGetMetasJson(): String
    external fun nativeGetSupportedDevicesJson(): String
    external fun nativeIsGpuMode(): Boolean
    external fun nativeFinalize()
}
