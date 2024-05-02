/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

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
        assert(out->sz != out->cap && "Capacity limit passed!. Please augment the span size");
        int const core_id = atoi(curptr);
        assert((core_id == -1 || core_id < logical_cores) && "Invalid core ID passed");
        out->core_id[out->sz++] = core_id;
      }
    }
    curptr = strtok_r(NULL, ",", &saveptr);
  }

  free(params_cpy);
}

