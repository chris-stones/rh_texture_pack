
#include "config.h"

#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <libimg.h>


const char *argp_program_version = "rh_texture_packer 0.1";
const char *argp_program_bug_address = NULL; // "NULL";
static char doc[] = "rh_texture_packer -- a program to generate texture atlas arrays.";
static char args_doc[] = "RESOURCE_ROOT_PATH";

static struct argp_option options[] = {
  {"verbose",  'v', 0,      OPTION_ARG_OPTIONAL,  "Produce verbose output" },
  {"quiet",    'q', 0,      OPTION_ARG_OPTIONAL,  "Don't produce any output" },
  {"output",   'o', "FILE", 0,  "Output to FILE" },
  {"width",    'w', "WIDTH",      0,  "texture atlas width" },
  {"height",   'h', "HEIGHT",      0,  "texture atlas height" },
  {"depth",    'd', "DEPTH",      0,  "texture atlas depth" },
  {"format",   'f', "FORMAT",	0, "output format" },
  {"pad",      'p', "PIXELS",	0, "pad sprites (reduce texture bleeding)" },
  {"debug",    'D', 0,      OPTION_ARG_OPTIONAL,  "debug mode" },
  {"logfile",  'l', "FILE", 0,  "write log to FILE"},
  { 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *)state->input;

  switch (key)
    {
    case 'q':
      arguments->quiet = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'o':
      arguments->output_file = arg;
      break;
    case 'l':
      arguments->log_file = arg;
      break;
    case 'w':
      arguments->width = atoi(arg);
      break;
    case 'h':
      arguments->height = atoi(arg);
      break;
    case 'p':
      arguments->pad = atoi(arg);
      break;
    case 'd':
      arguments->depth = atoi(arg);
      break;
    case 'D':
      arguments->debug = 1;
      break;
    case 'f':
	{
    if(strcasecmp(arg,"rgba16")==0) {
	  arguments->format = IMG_FMT_RGBA16_PMA;
	  break;
	}
	if(strcasecmp(arg,"rgba32")==0) {
	  arguments->format = IMG_FMT_RGBA32_PMA;
	  break;
	}
	if(strcasecmp(arg,"rgb24")==0) {
	  arguments->format = IMG_FMT_RGB24;
	  break;
	}
	if(strcasecmp(arg,"ycbcr")==0) {
	  arguments->format = IMG_FMT_YUV420P;
	  break;
	}
	if(strcasecmp(arg,"ycbcra")==0) {
	  arguments->format = IMG_FMT_YUVA420P_PMA;
	  break;
	}
	if(strcasecmp(arg,"dxt1")==0) {
	  arguments->format = IMG_FMT_DXT1;
	  break;
	}
	if(strcasecmp(arg,"dxt2")==0) {
	  arguments->format = IMG_FMT_DXT2;
	  break;
	}
	if(strcasecmp(arg,"dxt3")==0) {
	  arguments->format = IMG_FMT_DXT3;
	  break;
	}
	if(strcasecmp(arg,"dxt4")==0) {
	  arguments->format = IMG_FMT_DXT4;
	  break;
	}
	if(strcasecmp(arg,"dxt5")==0) {
	  arguments->format = IMG_FMT_DXT5;
	  break;
	}
    }
      argp_usage( state );
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
	/* Too many arguments. */
        argp_usage (state);

      arguments->resources = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1 || !arguments->output_file)
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

struct arguments read_args(int argc, char ** argv ) {

  struct arguments args;

  memset(&args,0,sizeof args);

  // defaults
  args.width = 2048;
  args.height = 2048;
  args.depth = 64;
  args.format = IMG_FMT_RGBA32_PMA;

  argp_parse (&argp, argc, argv, 0, 0, &args);

  return args;
}


