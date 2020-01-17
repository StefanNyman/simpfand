#ifndef PARSE_H
#define PARSE_H

#include <stddef.h>

#define BASE_LEVEL 1
#define MAX_FAN_LVL 7
#define POLL_INTERVAL 10
#define BUFF_MAX 128

#define STR_STARTS_WITH(x, y) (strncmp((x), (y), strlen(y)) == 0)

typedef struct tmp_lvl {
  unsigned short tmp;
  unsigned short lvl;
} tmp_lvl_t;

struct config {
  unsigned short base_lvl;

  unsigned short poll_int;
  unsigned short max_temp;

  tmp_lvl_t inc_lvls[MAX_FAN_LVL];
  tmp_lvl_t dec_lvls[MAX_FAN_LVL];
};

size_t strtrim(char *str);
void parse_config(struct config *cfg);
int config_path_exists(char *path, int pathlen);
void set_defaults();

#endif
