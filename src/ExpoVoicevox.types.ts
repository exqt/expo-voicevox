export enum AccelerationMode {
  AUTO = 0,
  CPU = 1,
  GPU = 2,
}

export interface InitializeOptions {
  openJtalkDictDir: string;
  accelerationMode?: AccelerationMode;
  cpuNumThreads?: number;
}

export interface SynthesisOptions {
  enableInterrogativeUpspeak?: boolean;
}

export interface TtsOptions {
  enableInterrogativeUpspeak?: boolean;
}
