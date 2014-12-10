#include <unistd.h>

#include <iostream>

#include "isolate.h"
#include "library.h"
#include "file.h"
#include "utils.h"

extern const uint8_t* snapshot_buffer;

namespace dart {

Dart_Handle LibraryTagHandler(Dart_LibraryTag tag,
                              Dart_Handle library,
                              Dart_Handle url) {
  const char* url_str = NULL;
  Dart_Handle result = Dart_StringToCString(url, &url_str);
  if (Dart_IsError(result))
    return result;
  assert(false);  // TODO: implement.
}

Dart_Handle ResolveScript(const char* script, Dart_Handle core_library) {
  char* cwd = getcwd(NULL, 0);
  Dart_Handle args[3] = {
    Dart_NewStringFromCString(cwd),
    Dart_NewStringFromCString(script),
    Dart_True()  // TODO: should this be true or false?
  };
  Dart_Handle ret = Dart_Invoke(
      core_library, Dart_NewStringFromCString("_resolveScriptUri"), 3, args);
  free(cwd);
  return ret;
}

Dart_Handle FilePathFromUri(Dart_Handle script, Dart_Handle core_library) {
  Dart_Handle args[2] = {
    script,
    Dart_True()  // TODO: should this be true or false?
  };
  return Dart_Invoke(core_library, Dart_NewStringFromCString("_filePathFromUri"), 2, args);
}

Dart_Handle ReadSource(Dart_Handle script, Dart_Handle core_library) {
  Dart_Handle script_path = FilePathFromUri(script, core_library);
  if (Dart_IsError(script_path))
    return script_path;

  const char* script_path_str;
  Dart_StringToCString(script_path, &script_path_str);

  FILE* file = fopen(script_path_str, "r");
  if (file == NULL)
    return Dart_NewApiError(
      // (std::ostringstream() << "Unable to read file '" << script_path_str << "'").str().c_str());
      STRING_STREAM("Unable to read file '" << script_path_str << "'").c_str());
    // return Dart_Error("Unable to read file '%s'", script_path_str);

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buffer = new char[length + 1];
  size_t read = fread(buffer, 1, length, file);
  fclose(file);
  buffer[read] = '\0';

  Dart_Handle source = Dart_NewStringFromCString(buffer);

  delete[] buffer;

  return source;
}

Dart_Handle LoadScript(const char* script,
                       bool resolve,
                       Dart_Handle core_library) {
  std::cout << __FUNCTION__ << ": " << script << ", " << resolve << std::endl;
  Dart_Handle resolved_script;

  if (resolve) {
    resolved_script = ResolveScript(script, core_library);
    if (Dart_IsError(resolved_script))
      return resolved_script;
  } else {
    resolved_script = Dart_NewStringFromCString(script);
  }

  Dart_Handle source = ReadSource(resolved_script, core_library);
  if (Dart_IsError(source))
    return source;

  return Dart_LoadScript(resolved_script, source, 0, 0);
}

Isolate* CreateIsolate(const char* script, const char* main, bool resolve,
                       void* data, char** error) {
  std::cout << __FUNCTION__ << ": " << script << ", " << main << ", "
            << resolve << std::endl;
  Dart_Isolate isolate = Dart_CreateIsolate(script, main, snapshot_buffer,
                                            NULL, error);
  assert(isolate);

  std::cout << "Created isolate" << std::endl;
  AutoEnterScope autoScope;

  auto isError = [&error] (Dart_Handle handle) {
    if (Dart_IsError(handle)) {
      *error = strdup(Dart_GetError(handle));
      Dart_ShutdownIsolate();
      return true;
    }
    return false;
  };

  Dart_Handle core_library;
  if (isError(Dart_SetLibraryTagHandler(LibraryTagHandler)) ||
      isError(Isolate::uri_library->Load()) ||
      isError(core_library = Isolate::core_library->Load()) ||
      isError(Isolate::io_library->Load())) {
    return NULL;
  }
  std::cout << "Loaded builtin libraries" << std::endl;

  std::cout << "About to load " << script << std::endl;
  Dart_Handle library = LoadScript(script, true, core_library);

  if (isError(library) ||
      isError(Dart_LibraryImportLibrary(library, core_library, Dart_Null())) {
    return NULL;
  }

  return new Isolate(isolate, library);
}

bool IsolateCreateCallback(const char* script_uri,
                           const char* main,
                           void* callback_data,
                           char** error) {
  Isolate* isolate = CreateIsolate(script_uri, main, true, NULL, error);
  if (isolate == NULL) {
    std::cerr << "Failed to create Isolate: " << script_uri << "|" << main
              << ": " << error << std::endl;
  }
  return isolate != NULL;
}

bool IsolateInterruptCallback() {
  return true;
}
Dart_Isolate IsolateCreateCallback(const char* script_uri,
                                   const char* main,
                                   const char* package_root,
                                   void* callback_data,
                                   char** error) {
  // TODO(ochafik): Implement this?
  return NULL;
}

Dart_Isolate ServiceIsolateCreateCalback(void* callback_data,
                                         char** error) {
  // TODO(ochafik): Implement this?
  return NULL;
}

void IsolateUnhandledExceptionCallback(Dart_Handle error) {
  // TODO(ochafik): Implement this?
}

void IsolateShutdownCallback(void* callback_data) {
  // TODO(ochafik): Implement this?
}


static std::string g_package_root;

void SetPackageRoot(const char* package_root) {
  g_package_root = package_root;
}

void* FileOpenCallback(const char* name, bool write) {
  std::string path = g_package_root + name;
  File *file = write ? File::openOutput(path) : File::openInput(path);
  if (file == nullptr) {
    printf("\nERROR: File not found: %s\n", path.c_str());
  }
  return file;
}

void FileReadCallback(const uint8_t** data, intptr_t* length, void* stream) {
  reinterpret_cast<File*>(stream)->read(data, length);
}

void FileWriteCallback(const void* data, intptr_t length, void* stream) {
  reinterpret_cast<File*>(stream)->write(data, length);
}

void FileCloseCallback(void* stream) {
  delete reinterpret_cast<File*>(stream);
}

bool EntropySource(uint8_t* buffer, intptr_t length) {
  // TODO(ochafik): Implement this?
  return false;
}

// public
void Initialize(int argc, const char* argv[]) {
  Dart_SetVMFlags(argc, argv);
  assert(Dart_Initialize(IsolateCreateCallback,
                         IsolateInterruptCallback,
                         IsolateUnhandledExceptionCallback,
                         IsolateShutdownCallback,
                         FileOpenCallback,
                         FileReadCallback,
                         FileWriteCallback,
                         FileCloseCallback,
                         EntropySource,
                         ServiceIsolateCreateCalback));

  Isolate::InitializeBuiltinLibraries();
}

// public
void Shutdown() {
  Isolate::ShutdownBuiltinLibraries();
}

// public
Isolate* LoadScript(const char* script) {
  std::cout << __FUNCTION__ << ": " << script << std::endl;
  char* error = NULL;
  Isolate* isolate = CreateIsolate(script, "main", true, NULL, &error);
  if (!isolate)
    std::cerr << "Failed to create Isolate: " << std::endl
              << "    " << error << std::endl;
  return isolate;
}

}
