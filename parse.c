#define _GNU_SOURCE
#include "parse.h"

#include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

size_t strtrim(char *str) {
  char *left = str, *right;

  if (!str || *str == '\0') {
    return 0;
  }
  while (isspace((unsigned char)*left)) {
    left++;
  }
  if (left != str) {
    memmove(str, left, (strlen(left) + 1));
  }
  if (*str == '\0') {
    return 0;
  }

  right = (char *)rawmemchr(str, '\0') - 1;
  while (isspace((unsigned char)*right)) {
    right--;
  }
  *++right = '\0';

  return right - left;
}

int config_path_exists(char *path, int pathlen) {
  char *cfg_path = "/etc/";
  struct stat st;

  if (stat(cfg_path, &st) == 0) {
    snprintf(path, pathlen, "%s/simpfand.conf", cfg_path);
    return 1;
  }
  return 0;
}

void parse_ul(char *val, unsigned short *out, unsigned short default_value) {
  char *ptr;
  unsigned short parsed = (unsigned short)strtoul(val, &ptr, 10);
  if (errno == ERANGE) {
    *out = default_value;
    return;
  }
  if (ptr == val) {
    *out = default_value;
    return;
  }
  *out = parsed;
}

void parse_tmp_lvls(tmp_lvl_t *lvls, char *val) {
  char *otoken;
  char *orest = val;
  int ocntr = 0;
  while ((otoken = strtok_r(orest, " ", &orest)) && ocntr < MAX_FAN_LVL) {
    char *ntoken;
    char *nrest = otoken;
    for (int i = 0; i < 2; i++) {
      ntoken = strtok_r(nrest, ":", &nrest);
      if (ntoken == NULL) {
        printf("invalid format for: %s\n", otoken);
        break;
      }
      switch (i) {
      case 0:
        parse_ul(ntoken, &(lvls[ocntr].tmp), lvls[ocntr].tmp);
        break;
      default:
        parse_ul(ntoken, &(lvls[ocntr].lvl), lvls[ocntr].lvl);
        break;
      }
    }
    ocntr++;
  }
}

void parse_config(struct config *cfg) {
  char line[BUFF_MAX];
  char conf_path[PATH_MAX];
  FILE *fp;

  if (!config_path_exists(conf_path, sizeof(conf_path))) {
    fprintf(stderr, "warning: could not find /etc/ "
                    "using defaults\n");
    return;
  }

  fp = fopen(conf_path, "r");
  if (!fp) {
    fprintf(stderr, "warning: no config file found "
                    "using defaults\n");
    return;
  }

  while (fgets(line, PATH_MAX, fp)) {
    char *key, *val = NULL;
    size_t linelen;

    linelen = strtrim(line);
    if (linelen == 0 || line[0] == '#') {
      /* go back to top if no line or is a comment*/
      continue;
    }

    if ((val = strchr(line, '#'))) {
      *val = '\0';
    }

    key = val = line;
    strsep(&val, "=");
    strtrim(key);

    if (val && !*val) {
      val = NULL;
    }

    if (STR_STARTS_WITH(key, "POLLING")) {
      parse_ul(val, &(cfg->poll_int), cfg->poll_int);
    } else if (STR_STARTS_WITH(key, "BASE_LVL")) {
      parse_ul(val, &(cfg->base_lvl), cfg->base_lvl);
    } else if (STR_STARTS_WITH(key, "INC_TMP_LVLS")) {
      parse_tmp_lvls(cfg->inc_lvls, val);
    } else if (STR_STARTS_WITH(key, "DEC_TMP_LVLS")) {
      parse_tmp_lvls(cfg->dec_lvls, val);
    }
  }
}

void set_defaults(struct config *cfg) {
  cfg->poll_int = POLL_INTERVAL;
  cfg->base_lvl = BASE_LEVEL;
  tmp_lvl_t incs[7] = {{40, 0}, {45, 1}, {50, 2}, {55, 3},
                       {60, 4}, {65, 6}, {70, 7}};
  tmp_lvl_t decs[7] = {{40, 0}, {45, 1}, {50, 2}, {55, 3},
                       {60, 6}, {65, 7}, {70, 7}};
  for (int i = 0; i < 7; i++) {
    cfg->inc_lvls[i] = incs[i];
    cfg->dec_lvls[i] = decs[i];
  }
}
