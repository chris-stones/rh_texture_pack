

#pragma once

#include <libimgutil.h>

#ifdef __cplusplus
extern "C" {
#endif

struct arguments
{
  int verbose;
  int quiet;
  int width;
  int height;
  int depth;
  int format;
  int pad;
  char *output_file;
  char *log_file;
  char *resources;
  int quality;
  err_diffuse_kernel_t edk;
  int edk_precision;
  int debug;
};

struct arguments read_args(int argc, char ** argv );

#ifdef __cplusplus
} // extern "C" {
#endif

