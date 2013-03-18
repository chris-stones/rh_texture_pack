

#pragma once

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
  int debug;
};

arguments read_args(int argc, char ** argv );

