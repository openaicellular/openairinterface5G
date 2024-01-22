#include "task.h"
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <time.h>

// Compatibility with previous TPool
void parse_num_threads(char const* params, span_core_id_t* out)
{
  assert(params != NULL);

  int const logical_cores = get_nprocs_conf();
  assert(logical_cores > 0);

  char *saveptr = NULL;
  char* params_cpy = strdup(params);
  char* curptr = strtok_r(params_cpy, ",", &saveptr);
  
  while (curptr != NULL) {
    int const c = toupper(curptr[0]);

    switch (c) {
      case 'N': {
        // pool->activated=false;
        free(params_cpy);
        out->core_id[out->sz++] = -1;
        return;
        break;
      }

      default: {
        assert(out->sz != out->cap && "Capacity limit passed!");
        int const core_id = atoi(curptr);
        assert((core_id == -1 || core_id < logical_cores) && "Invalid core ID passed");
        out->core_id[out->sz++] = core_id;
      }
    }
    curptr = strtok_r(NULL, ",", &saveptr);
  }

  free(params_cpy);
}

