const { getDefaultConfig } = require('expo/metro-config');
const path = require('path');

const config = getDefaultConfig(__dirname);

// Watch the monorepo root so Metro can find expo-voicevox source files
config.watchFolders = [path.resolve(__dirname, '..')];

// Enable package.json "exports" field resolution (needed for react 19.2+)
config.resolver.unstable_enablePackageExports = true;

// Add binary asset extensions for voicevox model/dict files
config.resolver.assetExts = [
  ...config.resolver.assetExts,
  'onnx', 'bin', 'dic', 'def', 'vvm',
];

module.exports = config;
