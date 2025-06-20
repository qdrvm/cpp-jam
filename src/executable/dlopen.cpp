/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C" {
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mach-o/dyld.h>
}

int main(int argc, const char **argv, const char **env) {
  uint32_t size = 0;
  _NSGetExecutablePath(NULL, &size);
  char *exe = (char *)malloc(size);
  _NSGetExecutablePath(exe, &size);

  const char *suffix = ".dylib";
  char *dylib = (char *)malloc(strlen(exe) + strlen(suffix) + 1);
  strcpy(dylib, exe);
  strcpy(dylib + strlen(exe), suffix);

  void *lib = dlopen(dylib, RTLD_NOW);
  if (lib == NULL) {
    printf("dlopen: %s\n", dlerror());
    return -1;
  }
  void *sym = dlsym(lib, "main");
  if (sym == NULL) {
    printf("dlsym: %s\n", dlerror());
    return -1;
  }
  return ((int (*)(int, const char **, const char **))sym)(argc, argv, env);
}
