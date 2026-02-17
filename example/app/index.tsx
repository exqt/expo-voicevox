import React, { useState, useCallback, useEffect } from "react";
import {
  View,
  Text,
  Button,
  StyleSheet,
  ScrollView,
  TextInput,
  ActivityIndicator,
} from "react-native";
import * as FileSystem from "expo-file-system/legacy";
import { Asset } from "expo-asset";
import { useAudioPlayer } from "expo-audio";
import * as Voicevox from "expo-voicevox";

const STYLE_ID = 0;

// Dict assets
const DICT_ASSETS: Record<string, number> = {
  "char.bin": require("../assets/open_jtalk_dic_utf_8-1.11/char.bin"),
  "left-id.def": require("../assets/open_jtalk_dic_utf_8-1.11/left-id.def"),
  "matrix.bin": require("../assets/open_jtalk_dic_utf_8-1.11/matrix.bin"),
  "pos-id.def": require("../assets/open_jtalk_dic_utf_8-1.11/pos-id.def"),
  "rewrite.def": require("../assets/open_jtalk_dic_utf_8-1.11/rewrite.def"),
  "right-id.def": require("../assets/open_jtalk_dic_utf_8-1.11/right-id.def"),
  "sys.dic": require("../assets/open_jtalk_dic_utf_8-1.11/sys.dic"),
  "unk.dic": require("../assets/open_jtalk_dic_utf_8-1.11/unk.dic"),
};

// VVM model file (voicevox_core 0.16.3 format)
// Place your .vvm file in assets/model/ directory
// Download from https://github.com/VOICEVOX/voicevox_vvm
let VVM_ASSET: number | null = null;
try {
  VVM_ASSET = require("../assets/model/0.vvm");
} catch {
  // VVM file not bundled — user must provide path manually
}

async function copyBinaryAssetsToDir(
  assets: Record<string, number>,
  targetDir: string,
  log: (msg: string) => void,
) {
  await FileSystem.makeDirectoryAsync(targetDir, { intermediates: true });
  for (const [filename, moduleId] of Object.entries(assets)) {
    const targetPath = targetDir + filename;
    const info = await FileSystem.getInfoAsync(targetPath);
    if (info.exists) continue;

    log(`Copying ${filename}...`);
    const [asset] = await Asset.loadAsync(moduleId);
    if (!asset.localUri) {
      throw new Error(`Failed to load asset: ${filename}`);
    }
    await FileSystem.copyAsync({
      from: asset.localUri,
      to: targetPath,
    });
  }
}

async function copyAllAssets(docDir: string, log: (msg: string) => void) {
  const dictDir = docDir + "open_jtalk_dic_utf_8-1.11/";
  const modelDir = docDir + "model/";

  // Copy dict files
  log("Copying dict assets...");
  await copyBinaryAssetsToDir(DICT_ASSETS, dictDir, log);

  // Copy VVM model file if bundled
  if (VVM_ASSET != null) {
    await FileSystem.makeDirectoryAsync(modelDir, { intermediates: true });
    const vvmPath = modelDir + "0.vvm";
    const info = await FileSystem.getInfoAsync(vvmPath);
    if (!info.exists) {
      log("Copying 0.vvm...");
      const [asset] = await Asset.loadAsync(VVM_ASSET);
      if (!asset.localUri) {
        throw new Error("Failed to load VVM asset");
      }
      await FileSystem.copyAsync({
        from: asset.localUri,
        to: vvmPath,
      });
    }
  }
}

export default function App() {
  const [status, setStatus] = useState("Preparing assets...");
  const [logs, setLogs] = useState<string[]>([]);
  const [text, setText] = useState("こんにちは");
  const [loading, setLoading] = useState(false);
  const [ready, setReady] = useState(false);
  const player = useAudioPlayer(null);

  const log = useCallback((msg: string) => {
    console.log(`[Voicevox] ${msg}`);
    setLogs((prev) => [...prev.slice(-29), `${new Date().toLocaleTimeString()} ${msg}`]);
  }, []);

  // Copy assets to documentDirectory on first launch
  useEffect(() => {
    (async () => {
      try {
        const docDir = FileSystem.documentDirectory!;
        log(`documentDirectory: ${docDir}`);

        await copyAllAssets(docDir, log);

        setReady(true);
        setStatus("Assets ready. Tap Initialize.");
        log("All assets ready!");
      } catch (e: any) {
        log(`Asset copy error: ${e.message}`);
        setStatus(`Asset error: ${e.message}`);
      }
    })();
  }, []);

  const handleInitialize = useCallback(async () => {
    try {
      setLoading(true);
      log("Initializing voicevox...");
      setStatus("Initializing...");

      // voicevox_core expects filesystem paths, not file:// URIs
      const docDir = FileSystem.documentDirectory!.replace("file://", "");
      const dictDir = docDir + "open_jtalk_dic_utf_8-1.11";
      const vvmPath = docDir + "model/0.vvm";

      log(`dictPath: ${dictDir}`);

      await Voicevox.initialize({
        openJtalkDictDir: dictDir,
        cpuNumThreads: 4,
      });

      log("Initialized! Loading VVM model...");
      setStatus("Initialized! Loading VVM model...");

      log(`vvmPath: ${vvmPath}`);
      await Voicevox.loadModel(vvmPath);

      const version = Voicevox.getVersion();
      log(`Ready (version: ${version})`);
      setStatus(`Ready (version: ${version})`);
    } catch (e: any) {
      log(`Init error: ${e.message}`);
      setStatus(`Init error: ${e.message}`);
    } finally {
      setLoading(false);
    }
  }, [log]);

  const handleTts = useCallback(async () => {
    try {
      setLoading(true);
      log("Starting TTS...");
      setStatus("Generating speech...");

      const fileUri = FileSystem.documentDirectory + "voice.wav";
      const nativePath = fileUri.replace("file://", "");
      const startTime = Date.now();

      await Voicevox.tts(text, STYLE_ID, nativePath);

      const elapsed = Date.now() - startTime;
      log(`TTS done in ${elapsed}ms`);
      setStatus(`TTS done in ${elapsed}ms. Playing...`);

      player.replace({ uri: fileUri });
      player.play();
      log("Playing audio");
    } catch (e: any) {
      log(`TTS error: ${e.message}`);
      setStatus(`TTS error: ${e.message}`);
    } finally {
      setLoading(false);
    }
  }, [text, player, log]);

  const handleAudioQueryAndSynthesis = useCallback(async () => {
    try {
      setLoading(true);
      log("Starting audioQuery...");
      setStatus("Running audioQuery...");

      const startTime = Date.now();
      const query = await Voicevox.audioQuery(text, STYLE_ID);

      const queryTime = Date.now() - startTime;
      log(`audioQuery done in ${queryTime}ms`);
      setStatus(`audioQuery: ${queryTime}ms. Synthesizing...`);

      const fileUri = FileSystem.documentDirectory + "voice.wav";
      const nativePath = fileUri.replace("file://", "");
      await Voicevox.synthesis(query, STYLE_ID, nativePath);

      const totalTime = Date.now() - startTime;
      log(`Synthesis done. Total: ${totalTime}ms`);
      setStatus(`Total: ${totalTime}ms. Playing...`);

      player.replace({ uri: fileUri });
      player.play();
      log("Playing audio");
    } catch (e: any) {
      log(`Error: ${e.message}`);
      setStatus(`Error: ${e.message}`);
    } finally {
      setLoading(false);
    }
  }, [text, player, log]);

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <Text style={styles.title}>VOICEVOX Expo Example</Text>

      <Text style={styles.status}>{status}</Text>

      {loading && <ActivityIndicator size="large" />}

      <View style={styles.buttonContainer}>
        <Button
          title="Initialize"
          onPress={handleInitialize}
          disabled={loading || !ready}
        />
      </View>

      <TextInput
        style={styles.input}
        value={text}
        onChangeText={setText}
        placeholder="Enter Japanese text"
      />

      <View style={styles.buttonContainer}>
        <Button
          title="TTS (one-step)"
          onPress={handleTts}
          disabled={loading}
        />
      </View>

      <View style={styles.buttonContainer}>
        <Button
          title="AudioQuery + Synthesis"
          onPress={handleAudioQueryAndSynthesis}
          disabled={loading}
        />
      </View>

      <View style={styles.logContainer}>
        <Text style={styles.logTitle}>Log:</Text>
        {logs.map((l, i) => (
          <Text key={i} style={styles.logText}>{l}</Text>
        ))}
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    padding: 24,
    paddingTop: 80,
    paddingBottom: 40,
  },
  title: {
    fontSize: 24,
    fontWeight: "bold",
    textAlign: "center",
  },
  status: {
    fontSize: 14,
    color: "#666",
    textAlign: "center",
    minHeight: 40,
    marginVertical: 8,
  },
  input: {
    borderWidth: 1,
    borderColor: "#ccc",
    borderRadius: 8,
    padding: 12,
    fontSize: 16,
    marginVertical: 8,
  },
  buttonContainer: {
    marginVertical: 6,
  },
  logContainer: {
    marginTop: 12,
    padding: 12,
    backgroundColor: "#f5f5f5",
    borderRadius: 8,
  },
  logTitle: {
    fontWeight: "bold",
    marginBottom: 4,
  },
  logText: {
    fontSize: 11,
    fontFamily: "monospace",
    color: "#333",
    lineHeight: 16,
  },
});
