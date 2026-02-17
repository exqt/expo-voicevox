# expo-voicevox
An Expo module for [VOICEVOX](https://voicevox.hiroshiba.jp/) Japanese text-to-speech engine. 

# VOICEVOX & ONNX Runtime Version
- **voicevox_core 0.16.3** 
- **voicevox_onnxruntime 1.17.3**

## Supported Platforms

- ✅ **Android**: arm64-v8a
- ✅ **iOS**: arm64 
- ❌ **Web**: Not supported


## Project Structure

```
expo-voicevox/
├── src/                              # TypeScript API
│   ├── index.ts                      # Public API (Promise Based)
│   ├── ExpoVoicevox.types.ts         # Type Definition (AccelerationMode, Options)
│   └── ExpoVoicevoxModule.ts         # requireNativeModule Wrapper
├── android/
│   ├── build.gradle                  # CMake Build
│   ├── src/main/
│   │   ├── java/expo/modules/voicevox/
│   │   │   ├── ExpoVoicevoxModule.kt # Expo Module (AsyncFunction/Function)
│   │   │   └── VoicevoxBridge.kt     # JNI external function definition
│   │   ├── jniLibs/arm64-v8a/
│   │   │   ├── libvoicevox_core.so
│   │   │   ├── libvoicevox_onnxruntime.so
│   │   │   └── libc++_shared.so
│   │   └── cpp/
│   │       ├── voicevox_jni.cpp      # JNI Implementaiton
│   │       ├── voicevox_core.h       # C API Header (0.16.3)
│   │       └── CMakeLists.txt        # NDK Build
├── ios/
│   ├── ExpoVoicevox.podspec          # CocoaPods Setting
│   ├── ExpoVoicevoxModule.swift      # Expo Module (Swift)
│   └── Frameworks/                    # Pre-bundled XCFrameworks
│       ├── voicevox_core.xcframework/
│       └── voicevox_onnxruntime.xcframework/
├── example/                          # Expo Example App
├── expo-module.config.json
├── package.json
└── tsconfig.json
```

## Prerequisites

You need the following files to use this module:

1. **Open JTalk dictionary** — `open_jtalk_dic_utf_8-1.11` ([download](https://sourceforge.net/projects/open-jtalk/files/Dictionary/open_jtalk_dic-1.11/))
2. **VVM model file** — `.vvm` format from [voicevox_vvm releases](https://github.com/VOICEVOX/voicevox_vvm)

### Native Libraries (Pre-bundled)

The native libraries are pre-bundled in this repository. Below are the original download sources for reference:

| Platform | Library | Source |
|----------|---------|--------|
| Android | `libvoicevox_core.so` | [voicevox_core-android-arm64-0.16.3.zip](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.16.3) |
| Android | `libvoicevox_onnxruntime.so` | [voicevox_onnxruntime-android-arm64-1.17.3.tgz](https://github.com/VOICEVOX/onnxruntime-builder/releases/tag/voicevox_onnxruntime-1.17.3) |
| iOS | `voicevox_core.xcframework` | [voicevox_core-ios-xcframework-cpu-0.16.3.zip](https://github.com/VOICEVOX/voicevox_core/releases/tag/0.16.3) |
| iOS | `voicevox_onnxruntime.xcframework` | [voicevox_onnxruntime-ios-xcframework-1.17.3.zip](https://github.com/VOICEVOX/onnxruntime-builder/releases/tag/voicevox_onnxruntime-1.17.3) |

## Installation

```bash
npm install expo-voicevox
```

Then rebuild your native project:

```bash
cd example
npx expo run:android  # Android
npx expo run:ios      # iOS
```

## Usage

### 1. Copy assets to the device filesystem

The dictionary and model files must be accessible as native filesystem paths. A common pattern is to bundle them as assets and copy them on first launch:

```typescript
import * as FileSystem from "expo-file-system/legacy";
import { Asset } from "expo-asset";

const DICT_ASSETS: Record<string, number> = {
  "char.bin": require("./assets/open_jtalk_dic_utf_8-1.11/char.bin"),
  "left-id.def": require("./assets/open_jtalk_dic_utf_8-1.11/left-id.def"),
  "matrix.bin": require("./assets/open_jtalk_dic_utf_8-1.11/matrix.bin"),
  "pos-id.def": require("./assets/open_jtalk_dic_utf_8-1.11/pos-id.def"),
  "rewrite.def": require("./assets/open_jtalk_dic_utf_8-1.11/rewrite.def"),
  "right-id.def": require("./assets/open_jtalk_dic_utf_8-1.11/right-id.def"),
  "sys.dic": require("./assets/open_jtalk_dic_utf_8-1.11/sys.dic"),
  "unk.dic": require("./assets/open_jtalk_dic_utf_8-1.11/unk.dic"),
};

const VVM_ASSET = require("./assets/model/0.vvm");

// Copy each file to documentDirectory if not already present
const docDir = FileSystem.documentDirectory!;
for (const [filename, moduleId] of Object.entries(DICT_ASSETS)) {
  const targetPath = docDir + "dict/" + filename;
  const info = await FileSystem.getInfoAsync(targetPath);
  if (!info.exists) {
    const [asset] = await Asset.loadAsync(moduleId);
    await FileSystem.copyAsync({ from: asset.localUri!, to: targetPath });
  }
}
```

> **Note:** Add `onnx`, `bin`, `dic`, `def`, `vvm` to `assetExts` in your `metro.config.js`:
> ```js
> config.resolver.assetExts = [
>   ...config.resolver.assetExts,
>   'onnx', 'bin', 'dic', 'def', 'vvm',
> ];
> ```

### 2. Initialize the engine

```typescript
import * as Voicevox from "expo-voicevox";

// voicevox_core expects native filesystem paths (no file:// prefix)
const docDir = FileSystem.documentDirectory!.replace("file://", "");
const dictDir = docDir + "dict";
const vvmPath = docDir + "model/0.vvm";

await Voicevox.initialize({
  openJtalkDictDir: dictDir,
  cpuNumThreads: 4,
});

await Voicevox.loadModel(vvmPath);
```

### 3. Generate speech

**One-step TTS** (text to WAV file directly):

```typescript
const outputPath = docDir + "voice.wav";
await Voicevox.tts("こんにちは", 0, outputPath);
```

**Two-step** (audioQuery + synthesis, allows modifying query parameters):

```typescript
const queryJson = await Voicevox.audioQuery("こんにちは", 0);
await Voicevox.synthesis(queryJson, 0, outputPath);
```

### 4. Play the audio

```typescript
import { useAudioPlayer } from "expo-audio";

const player = useAudioPlayer(null);

// After generating the WAV file:
const fileUri = FileSystem.documentDirectory + "voice.wav";
player.replace({ uri: fileUri });
player.play();
```

### 5. Clean up

```typescript
await Voicevox.finalize();
```

## API Reference

### Async Functions

| Function | Description |
|----------|-------------|
| `initialize(options)` | Initialize the engine with Open JTalk dictionary path |
| `loadModel(vvmPath)` | Load a VVM model file |
| `audioQuery(text, styleId)` | Generate an AudioQuery JSON from text |
| `audioQueryFromKana(kana, styleId)` | Generate an AudioQuery JSON from kana |
| `synthesis(queryJson, styleId, outputPath, options?)` | Synthesize WAV from AudioQuery JSON |
| `tts(text, styleId, outputPath, options?)` | One-step text to WAV |
| `ttsFromKana(kana, styleId, outputPath, options?)` | One-step kana to WAV |
| `finalize()` | Release all resources |

### Sync Functions

| Function | Description |
|----------|-------------|
| `getVersion()` | Get voicevox_core version string |
| `getMetasJson()` | Get speaker metadata as JSON |
| `getSupportedDevicesJson()` | Get supported devices as JSON |
| `isGpuMode()` | Check if GPU mode is active |

### Types

```typescript
interface InitializeOptions {
  openJtalkDictDir: string;
  accelerationMode?: AccelerationMode; // default: CPU
  cpuNumThreads?: number;              // default: 0 (auto)
}

enum AccelerationMode {
  AUTO = 0,
  CPU = 1,
  GPU = 2,
}

interface SynthesisOptions {
  enableInterrogativeUpspeak?: boolean; // default: true
}

interface TtsOptions {
  enableInterrogativeUpspeak?: boolean; // default: true
}
```

## Example

See the [`example/`](./example) directory for a complete working app with asset copying, initialization, TTS, and audio playback.

## License

MIT
