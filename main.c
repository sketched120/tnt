
#define _GNU_SOURCE

#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "include/launch.h"
#include "include/fast.h"
#include "include/fabric.h"
#include "include/utils.h"
#include "include/version.h"

char *minecraft_path = NULL;
char *offline_username = NULL;
bool offline = false;

bool wayland = false;

static void print_help_main(const char *prog) {
  lprintf(INFO,
      "usage: %s [options]\n"
      "options:\n"
      "  --launch \t\tlaunch a version\n"
      "  --download \t\tdownload a version\n"
      "  --dry-run \t\t dry run (print java cmdline)\n"
      "  --list-available <type>\t\tlist available versions\n"
      "  --type <type>\t\trelease, snapshot, fabric\n"
      "  --path <path>\t\tminecraft path (default: ~/.minecraft)\n"
      "  --instance <instance>\t\tuse an instance directory\n"
      "  --list-installed\t\t\tlist installed versions\n"
      "  --help\t\t\tprint this help and exit",
      prog);
}

static void print_help_download(void) {
	lprintf(INFO,
	"\n--download : downloads a version\n"
	"options:\n"
	"\t--version\tspecifies a version to download\n"
	"\t--type\tspecifies the type to install (release, vanilla, fabric)\n"
	"\t--fabric-version\tspecifies the fabric loader version to install (used with '--type fabric' only)"
	);
}

static void print_help_launch(void) {
	lprintf(INFO,
	"\n--launch : launches a minecraft installation\n"
	"options: \n"
	"\t--version\tspecifies a version to launch\n"
	"\t--mem\tspecifies the amount of memory (in GB) to allocate for the jvm (default: 2)\n"
	"\t--offline\tlaunches the game in offline mode (without authentication)"
	);
}
#define NOARG 0
#define REQARG 1
#define OPARG 2
  
int main(int argc, char *argv[]) {
  curl_global_init(CURL_GLOBAL_ALL);

  char *version = NULL;
  char *dry_arg = NULL;
  int launch = 0, download = 0, list = 0, listi = 0, zmm = 0;
  char *type = NULL;
  char *instance = NULL;
  char *fabric_version = NULL;
  int fabric_alloced = 0;
  float mem = 2;
  int fast = 0;
  int opt;
  int exit_code = 0;
  int refresh = 0;
  int help = 0;
  opterr = 0;


  struct option longopts[] = {
  	{"launch", NOARG, NULL, 'l'},
  	{"dry-run",	REQARG,	NULL,	'D'},
  	{"list-available",	REQARG,	NULL,	'L'},
  	{"version", REQARG,	NULL,	'v'},
  	{"download", NOARG,	NULL,	'd'},
  	{"minecraft-path", REQARG, NULL, 'p'},
  	{"type",	REQARG, 	NULL, 	't'},
  	{"fast",	NOARG,		NULL,	'F'},
  	{"fabric-version",	REQARG, NULL,	'f'},
  	{"list-installed",	NOARG,	NULL, 'i'},
  	{"instance",	REQARG,	NULL, 'n'},
  	{"offline", 	REQARG,	NULL,	'o'},
  	{"mem",	REQARG,	NULL,	'm'},
  	{"refresh", NOARG,	NULL, 'r'},
  	{"help", 	NOARG,	NULL,	'h'},
  	{NULL,	0,	NULL, 0}
  	
  };
  while ((opt = getopt_long(argc, argv, "lD:L:dv:t:Ff:p:in:m:zho:wr", longopts, NULL)) != -1) {
    switch (opt) {
    case 'l': launch = 1; break;
    case 'D': dry_arg = optarg; break;
    case 'L': list = 1; type = optarg; break;
    case 'v': version = optarg; break;
    case 'd': download = 1; break;
    case 'p': minecraft_path = strdup(optarg); break;
    case 't': type = optarg; break;
    case 'F': fast = 1; break;
    case 'f': fabric_version = optarg; break;
    case 'i': listi = 1; break;
    case 'n': instance = optarg; break;
    // case 'z': zmm = 1; break;
    case 'o': offline = true; offline_username = optarg; break; // global change
    case 'm': mem = strtof(optarg, NULL); break;
    case 'r': refresh = 1; break;
    case 'h':
      help = 1;
	  break;
    case '?':
      if (optopt)
        lprintf(ERROR, "Unknown option '%-c'", optopt);
      else
       lprintf(ERROR,"Unknown option '%s'", argv[optind - 1]);
      exit_code = 1;
      goto cleanup;
    }
  }

	
	
  if (!minecraft_path) {
    asprintf(&minecraft_path, "%s/.minecraft", getenv("HOME"));
  }
 
  mkdir(minecraft_path, 0755);
  chdir(minecraft_path);

  if (fast == 1) {
    fastlaunch();
    return 0;
  }

  lprintf(INFO, "zap 0.0.2" );

  if (getenv("WAYLAND_DISPLAY") != NULL) wayland = true;
  if (refresh) {
    	unlink("version_manifest.json");
    	download_version_manifest();
    	lprintf(INFO, "refreshed version manifest. you may now see the latest available versions.");
  	return 0;
  }
  
  if (instance) {
    char *inst_path = NULL;
    asprintf(&inst_path, "%s/instances/%s", minecraft_path, instance);
    mkdirs(inst_path);
    mkdir(inst_path, 0755);
    chdir(inst_path);
    free(inst_path);
  }

  if (zmm == 1) {
    char path[BUF_MID];
    snprintf(path, sizeof(path), "%s/zmm", minecraft_path);
    execl(path, path, NULL);
    perror("execlp");
    exit_code = 1;
    goto cleanup;
  } 

  if (listi == 1) {
    list_installed();
    goto cleanup;
  }

  if (optind < argc) {
    lprintf(ERROR,"Unexpected argument '%s'", argv[optind]);
    exit_code = 1;
    goto cleanup;
  }

  if (launch == 1) {
  	if (help) {
  		print_help_launch();
  		goto cleanup;
  	}
    if (version) {
      fastcreate(argc, argv);
      launchmc(0, mem, version);
      goto cleanup;
    } else {
      lprintf(ERROR, "Specify a version to launch.");
      exit_code = 1;
      goto cleanup;
    }
  }

  if (dry_arg) {
    launchmc(1, mem, dry_arg);
    goto cleanup;
  }

  if (list == 1) {
    list_available_versions(type);
    goto cleanup;
  }

  if (download == 1) {
  	if (help) {
    		print_help_download();
    		goto cleanup;
    	}
    if (version) {
      if (!type) {
        lprintf(ERROR,"Specify a type!");
        exit_code = 1;
        goto cleanup;
      }
      if (streq(type, "release") || streq(type, "snapshot")) {
        download_version(version);
      } else if (streq(type, "fabric")) {
        if (!fabric_version) {
          fabric_version = get_latest_loader(version);
          fabric_alloced = 1;
          if (!fabric_version) {
            exit_code = 1;
            goto cleanup;
          }
        }
        download_fabric_manifest(version, fabric_version);
        if (download_version(version) != 0) {
          lprintf(ERROR, "Download failed!");
          exit_code = 1;
        }
      } else {
        lprintf(ERROR, "Invalid type!");
        exit_code = 1;
      }
      goto cleanup;
    }
  }

  if (fabric_version && !download) {
    list_fabric_versions(fabric_version);
    goto cleanup;
  }

  if (help) {
  		print_help_main(argv[0]);
  		goto cleanup;
  	}
  lprintf(ERROR, "No mode specified, try -h");
  exit_code = 1;

cleanup:
  if (fabric_alloced && fabric_version) free(fabric_version);
  if (minecraft_path) free(minecraft_path);
  curl_global_cleanup();
  return exit_code;
}
