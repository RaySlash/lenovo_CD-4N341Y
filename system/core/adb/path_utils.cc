// Copyright (c) 2016 Google Inc. All rights reserved.

// Functions for manipulating and validating Unix paths.

#include <locale>
#include <string>

#include <libgen.h>
#include <string.h>

#include "path_utils.h"  // NOLINT(build/include)

namespace {

// Only scripts below this path and with this extension may be executed.
constexpr char kScriptDir[] = "/home/flex/bin";
constexpr char kScriptExtension[] = ".sh";

// Only files below this path may be pulled. Must have a trailing slash.
constexpr char kPullPath[] = "/tmp/secure_adb/";

// Returns the directory part of the given path.
std::string GetDirname(const std::string& path) {
  // Make a mutable copy since dirname may modify its input
  char* path_copy = strdup(path.c_str());
  if (path_copy == nullptr) {
    return "";
  }

  const std::string dir(dirname(path_copy));
  free(path_copy);
  return dir;
}

// Returns the filename part of the given path.
std::string GetFilename(const std::string& path) {
  // Make a mutable copy since basename may modify its input
  char* path_copy = strdup(path.c_str());
  if (path_copy == nullptr) {
    return "";
  }

  const std::string file(basename(path_copy));
  free(path_copy);
  return file;
}

// Returns the extension of the given filename, including the '.' character.
// If there is no '.' char in the filename then the empty string is returned.
std::string GetExtension(const std::string& filename) {
  size_t index = filename.find_last_of(".");
  if (index == std::string::npos) {
    return "";
  }
  return filename.substr(index);
}

// Returns the canonicalized absolute representation of the given path,
// removing '..', '.', and extra '/'. If there is any error, for example
// if the file/directory specified by the path doesn't exist, the empty
// string is returned.
std::string SimplifyPath(const std::string& path) {
  char* ret = realpath(path.c_str(), NULL);
  if (ret == nullptr) {
    return "";
  }

  const std::string new_path(ret);
  free(ret);
  return new_path;
}

// Returns true if all characters in str are in the subset ok for "safe" shell
// commands. This subset notably excludes ';', '|', and '&', which can be
// used to chain commands.
bool IsSafeChars(const std::string& str) {
  for (auto c : str) {
    // Space for argument separation, underscore for test command names,
    // period for filenames, dashes to designate options in arg list
    if (std::isalnum(c, std::locale("C")) || c == ' ' || c == '_' || c == '.' || c == '-')
      continue;

    return false;
  }

  return true;
}

}  // namespace

// Returns true if the given string is a safe shell command. This is based
// on the location of the file to be executed and the characters present in
// the arguments.
extern "C" bool IsSafeCommand(const char* command) {
  const std::string cmd = std::string(command);

  // Get the script's path and args by splitting the command on the first space
  std::size_t index = cmd.find(" ");
  const std::string script_path = SimplifyPath(cmd.substr(0, index));

  // Check path and extension of script
  if (script_path.empty() || GetDirname(script_path) != kScriptDir ||
      GetExtension(GetFilename(script_path)) != kScriptExtension) {
    return false;
  }

  // Check arguments for unsafe characters
  std::string args = index == std::string::npos ? "" : cmd.substr(index + 1);
  return IsSafeChars(args);
}

// Returns true if the given path points to a file or dir that may be pulled.
// Things under kPullPath are allowed, but not that directory itself.
extern "C" bool IsAllowedPath(const char* path) {
  static_assert(kPullPath[sizeof(kPullPath) - 2] == '/',
                "kPullPath must end with a slash");

  const std::string simplified_path = SimplifyPath(path);

  // Simplified paths don't have trailing slashes, so this only allows things
  // under the allowed directory, not that directory itself.
  return simplified_path.compare(0, sizeof(kPullPath) - 1, kPullPath) == 0;
}
