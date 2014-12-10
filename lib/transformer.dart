import 'package:barback/barback.dart';
import 'dart:async';
import 'dart_jvm_builder.dart';

class NativeExtensionBuilder extends Transformer {
  final BarbackSettings _settings;

  NativeExtensionBuilder.asPlugin(this._settings);
  
  static bool builtOnce = false;

  Future apply(Transform transform) {
    var id = transform.primaryInput.id;
    print(id);
    if (!builtOnce) {
      JVMBuilder.buildExtension();
      builtOnce = true;
    }
    return new Future.value();
  }
}
