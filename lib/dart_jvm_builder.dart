library dart_jvm.builder;

import 'package:ccompile/ccompile.dart';

class JVMBuilder {
  static void buildExtension([String workingDir = "packages/dart_jvm"]) {
    const yamlFile = "dart_jvm_extension_ccompile.yaml";

    var builder = new ProjectBuilder();
    var project = builder.loadProject("$workingDir/$yamlFile");
    var result = builder.buildAndClean(project, workingDir);
    for (var s in [result.stdout, result.stderr]) {
      if (s.trim().length > 0) {
        print(s);
      }
    }
    assert(result.exitCode == 0);
  }
}