require 'json'

package = JSON.parse(File.read(File.join(__dir__, '..', 'package.json')))

Pod::Spec.new do |s|
  s.name           = 'ExpoVoicevox'
  s.version        = package['version']
  s.summary        = package['description']
  s.description    = package['description']
  s.license        = package['license']
  s.authors        = package['author'] || 'exqt'
  s.homepage       = package['homepage'] || 'https://github.com/exqt/expo-voicevox'
  s.platforms      = { :ios => '15.1' }
  s.swift_version  = '5.4'
  s.source         = { git: '' }
  s.static_framework = true

  s.dependency 'ExpoModulesCore'

  s.source_files = '*.swift'

  s.vendored_frameworks = 'Frameworks/voicevox_core.xcframework', 'Frameworks/voicevox_onnxruntime.xcframework'

  s.pod_target_xcconfig = {
    'OTHER_LDFLAGS' => '-framework voicevox_core -framework voicevox_onnxruntime',
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/ExpoVoicevox/Frameworks/voicevox_core.xcframework/ios-arm64/voicevox_core.framework/Headers"'
  }

  s.user_target_xcconfig = {
    'OTHER_LDFLAGS' => '-framework voicevox_core -framework voicevox_onnxruntime'
  }
end
