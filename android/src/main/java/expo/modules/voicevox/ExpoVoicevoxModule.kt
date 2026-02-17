package expo.modules.voicevox

import expo.modules.kotlin.modules.Module
import expo.modules.kotlin.modules.ModuleDefinition

class ExpoVoicevoxModule : Module() {
    private val bridge = VoicevoxBridge.instance
    private val lock = Any()

    override fun definition() = ModuleDefinition {
        Name("ExpoVoicevox")

        AsyncFunction("initialize") { openJtalkDictDir: String, accelerationMode: Int, cpuNumThreads: Int ->
            synchronized(lock) {
                bridge.nativeInitialize(accelerationMode, cpuNumThreads, openJtalkDictDir)
            }
        }

        AsyncFunction("loadModel") { vvmPath: String ->
            synchronized(lock) {
                bridge.nativeLoadModel(vvmPath)
            }
        }

        AsyncFunction("audioQuery") { text: String, styleId: Int ->
            synchronized(lock) {
                bridge.nativeAudioQuery(text, styleId)
            }
        }

        AsyncFunction("audioQueryFromKana") { kana: String, styleId: Int ->
            synchronized(lock) {
                bridge.nativeAudioQueryFromKana(kana, styleId)
            }
        }

        AsyncFunction("synthesis") { audioQueryJson: String, styleId: Int, outputPath: String, enableInterrogativeUpspeak: Boolean ->
            synchronized(lock) {
                bridge.nativeSynthesis(audioQueryJson, styleId, outputPath, enableInterrogativeUpspeak)
            }
        }

        AsyncFunction("tts") { text: String, styleId: Int, outputPath: String, enableInterrogativeUpspeak: Boolean ->
            synchronized(lock) {
                bridge.nativeTts(text, styleId, outputPath, enableInterrogativeUpspeak)
            }
        }

        AsyncFunction("ttsFromKana") { kana: String, styleId: Int, outputPath: String, enableInterrogativeUpspeak: Boolean ->
            synchronized(lock) {
                bridge.nativeTtsFromKana(kana, styleId, outputPath, enableInterrogativeUpspeak)
            }
        }

        AsyncFunction("finalize") {
            synchronized(lock) {
                bridge.nativeFinalize()
            }
        }

        Function("getVersion") {
            bridge.nativeGetVersion()
        }

        Function("getMetasJson") {
            bridge.nativeGetMetasJson()
        }

        Function("getSupportedDevicesJson") {
            bridge.nativeGetSupportedDevicesJson()
        }

        Function("isGpuMode") {
            bridge.nativeIsGpuMode()
        }
    }
}
