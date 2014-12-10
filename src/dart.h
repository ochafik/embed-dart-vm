namespace dart {
  class Isolate;

  void SetPackageRoot(const char* package_root);
  void Initialize(int argc, const char* argv[]);
  void Shutdown();
  Isolate* LoadScript(const char* script);
}
