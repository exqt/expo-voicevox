import ExpoModulesCore
import voicevox_core

private let VV_OK: Int32 = 0 // VOICEVOX_RESULT_OK

public class ExpoVoicevoxModule: Module {
    private var onnxruntime: OpaquePointer?
    private var openJtalk: OpaquePointer?
    private var synthesizer: OpaquePointer?
    private let queue = DispatchQueue(label: "expo.modules.voicevox.serial")

    public func definition() -> ModuleDefinition {
        Name("ExpoVoicevox")

        AsyncFunction("initialize") { (openJtalkDictDir: String, accelerationMode: Int32, cpuNumThreads: Int32) in
            try self.queue.sync {
                // Init ONNX Runtime (linked on iOS)
                if self.onnxruntime == nil {
                    var ort: OpaquePointer?
                    let ortResult = voicevox_onnxruntime_init_once(&ort)
                    guard ortResult == VV_OK else {
                        throw self.makeError(ortResult)
                    }
                    self.onnxruntime = ort
                }

                // Create OpenJtalk
                if let oldJtalk = self.openJtalk {
                    voicevox_open_jtalk_rc_delete(oldJtalk)
                    self.openJtalk = nil
                }
                var jtalk: OpaquePointer?
                let jtalkResult = voicevox_open_jtalk_rc_new(openJtalkDictDir, &jtalk)
                guard jtalkResult == VV_OK else {
                    throw self.makeError(jtalkResult)
                }
                self.openJtalk = jtalk

                // Create Synthesizer
                if let oldSynth = self.synthesizer {
                    voicevox_synthesizer_delete(oldSynth)
                    self.synthesizer = nil
                }
                var options = voicevox_make_default_initialize_options()
                options.acceleration_mode = VoicevoxAccelerationMode(accelerationMode)
                options.cpu_num_threads = UInt16(cpuNumThreads)

                var synth: OpaquePointer?
                let synthResult = voicevox_synthesizer_new(self.onnxruntime, self.openJtalk, options, &synth)
                guard synthResult == VV_OK else {
                    throw self.makeError(synthResult)
                }
                self.synthesizer = synth
            }
        }

        AsyncFunction("loadModel") { (vvmPath: String) in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var model: OpaquePointer?
                let openResult = voicevox_voice_model_file_open(vvmPath, &model)
                guard openResult == VV_OK else {
                    throw self.makeError(openResult)
                }
                let loadResult = voicevox_synthesizer_load_voice_model(synthesizer, model)
                voicevox_voice_model_file_delete(model)
                guard loadResult == VV_OK else {
                    throw self.makeError(loadResult)
                }
            }
        }

        AsyncFunction("audioQuery") { (text: String, styleId: Int32) -> String in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var outputJson: UnsafeMutablePointer<CChar>?
                let result = voicevox_synthesizer_create_audio_query(synthesizer, text, UInt32(styleId), &outputJson)
                guard result == VV_OK else {
                    throw self.makeError(result)
                }
                let jsonString = String(cString: outputJson!)
                voicevox_json_free(outputJson)
                return jsonString
            }
        }

        AsyncFunction("audioQueryFromKana") { (kana: String, styleId: Int32) -> String in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var outputJson: UnsafeMutablePointer<CChar>?
                let result = voicevox_synthesizer_create_audio_query_from_kana(synthesizer, kana, UInt32(styleId), &outputJson)
                guard result == VV_OK else {
                    throw self.makeError(result)
                }
                let jsonString = String(cString: outputJson!)
                voicevox_json_free(outputJson)
                return jsonString
            }
        }

        AsyncFunction("synthesis") { (audioQueryJson: String, styleId: Int32, outputPath: String, enableInterrogativeUpspeak: Bool) -> String in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var options = voicevox_make_default_synthesis_options()
                options.enable_interrogative_upspeak = enableInterrogativeUpspeak

                var wavLength: UInt = 0
                var wavData: UnsafeMutablePointer<UInt8>?
                let result = voicevox_synthesizer_synthesis(synthesizer, audioQueryJson, UInt32(styleId), options, &wavLength, &wavData)
                guard result == VV_OK else {
                    throw self.makeError(result)
                }

                let data = Data(bytes: wavData!, count: Int(wavLength))
                voicevox_wav_free(wavData)

                let url = URL(fileURLWithPath: outputPath)
                try data.write(to: url)
                return outputPath
            }
        }

        AsyncFunction("tts") { (text: String, styleId: Int32, outputPath: String, enableInterrogativeUpspeak: Bool) -> String in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var options = voicevox_make_default_tts_options()
                options.enable_interrogative_upspeak = enableInterrogativeUpspeak

                var wavLength: UInt = 0
                var wavData: UnsafeMutablePointer<UInt8>?
                let result = voicevox_synthesizer_tts(synthesizer, text, UInt32(styleId), options, &wavLength, &wavData)
                guard result == VV_OK else {
                    throw self.makeError(result)
                }

                let data = Data(bytes: wavData!, count: Int(wavLength))
                voicevox_wav_free(wavData)

                let url = URL(fileURLWithPath: outputPath)
                try data.write(to: url)
                return outputPath
            }
        }

        AsyncFunction("ttsFromKana") { (kana: String, styleId: Int32, outputPath: String, enableInterrogativeUpspeak: Bool) -> String in
            try self.queue.sync {
                guard let synthesizer = self.synthesizer else {
                    throw NSError(domain: "ExpoVoicevox", code: -1, userInfo: [NSLocalizedDescriptionKey: "Synthesizer not initialized"])
                }
                var options = voicevox_make_default_tts_options()
                options.enable_interrogative_upspeak = enableInterrogativeUpspeak

                var wavLength: UInt = 0
                var wavData: UnsafeMutablePointer<UInt8>?
                let result = voicevox_synthesizer_tts_from_kana(synthesizer, kana, UInt32(styleId), options, &wavLength, &wavData)
                guard result == VV_OK else {
                    throw self.makeError(result)
                }

                let data = Data(bytes: wavData!, count: Int(wavLength))
                voicevox_wav_free(wavData)

                let url = URL(fileURLWithPath: outputPath)
                try data.write(to: url)
                return outputPath
            }
        }

        AsyncFunction("finalize") {
            self.queue.sync {
                if let synth = self.synthesizer {
                    voicevox_synthesizer_delete(synth)
                    self.synthesizer = nil
                }
                if let jtalk = self.openJtalk {
                    voicevox_open_jtalk_rc_delete(jtalk)
                    self.openJtalk = nil
                }
            }
        }

        Function("getVersion") { () -> String in
            return String(cString: voicevox_get_version())
        }

        Function("getMetasJson") { () -> String in
            guard let synthesizer = self.synthesizer else {
                return "[]"
            }
            let metas = voicevox_synthesizer_create_metas_json(synthesizer)!
            let jsonString = String(cString: metas)
            voicevox_json_free(metas)
            return jsonString
        }

        Function("getSupportedDevicesJson") { () -> String in
            guard let ort = self.onnxruntime else {
                return "{}"
            }
            var devices: UnsafeMutablePointer<CChar>?
            let result = voicevox_onnxruntime_create_supported_devices_json(ort, &devices)
            guard result == VV_OK else {
                return "{}"
            }
            let jsonString = String(cString: devices!)
            voicevox_json_free(devices)
            return jsonString
        }

        Function("isGpuMode") { () -> Bool in
            guard let synthesizer = self.synthesizer else {
                return false
            }
            return voicevox_synthesizer_is_gpu_mode(synthesizer)
        }
    }

    private func makeError(_ code: Int32) -> NSError {
        let message = String(cString: voicevox_error_result_to_message(code))
        return NSError(domain: "ExpoVoicevox", code: Int(code), userInfo: [NSLocalizedDescriptionKey: message])
    }
}
