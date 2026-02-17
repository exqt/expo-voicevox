import ExpoVoicevoxModule from "./ExpoVoicevoxModule";
import {
  AccelerationMode,
  InitializeOptions,
  SynthesisOptions,
  TtsOptions,
} from "./ExpoVoicevox.types";

export {
  AccelerationMode,
  InitializeOptions,
  SynthesisOptions,
  TtsOptions,
};

export async function initialize(options: InitializeOptions): Promise<void> {
  return ExpoVoicevoxModule.initialize(
    options.openJtalkDictDir,
    options.accelerationMode ?? AccelerationMode.CPU,
    options.cpuNumThreads ?? 0
  );
}

export async function loadModel(vvmPath: string): Promise<void> {
  return ExpoVoicevoxModule.loadModel(vvmPath);
}

export async function audioQuery(
  text: string,
  styleId: number
): Promise<string> {
  return ExpoVoicevoxModule.audioQuery(text, styleId);
}

export async function audioQueryFromKana(
  kana: string,
  styleId: number
): Promise<string> {
  return ExpoVoicevoxModule.audioQueryFromKana(kana, styleId);
}

export async function synthesis(
  query: string,
  styleId: number,
  outputPath: string,
  options?: SynthesisOptions
): Promise<string> {
  return ExpoVoicevoxModule.synthesis(
    query,
    styleId,
    outputPath,
    options?.enableInterrogativeUpspeak ?? true
  );
}

export async function tts(
  text: string,
  styleId: number,
  outputPath: string,
  options?: TtsOptions
): Promise<string> {
  return ExpoVoicevoxModule.tts(
    text,
    styleId,
    outputPath,
    options?.enableInterrogativeUpspeak ?? true
  );
}

export async function ttsFromKana(
  kana: string,
  styleId: number,
  outputPath: string,
  options?: TtsOptions
): Promise<string> {
  return ExpoVoicevoxModule.ttsFromKana(
    kana,
    styleId,
    outputPath,
    options?.enableInterrogativeUpspeak ?? true
  );
}

export async function finalize(): Promise<void> {
  return ExpoVoicevoxModule.finalize();
}

export function getVersion(): string {
  return ExpoVoicevoxModule.getVersion();
}

export function getMetasJson(): string {
  return ExpoVoicevoxModule.getMetasJson();
}

export function getSupportedDevicesJson(): string {
  return ExpoVoicevoxModule.getSupportedDevicesJson();
}

export function isGpuMode(): boolean {
  return ExpoVoicevoxModule.isGpuMode();
}
