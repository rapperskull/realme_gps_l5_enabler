/*
 * Copyright 2023 Rapper_skull
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(_MSC_VER)
  #define _CRT_NONSTDC_NO_WARNINGS  // POSIX function names
  #define _CRT_SECURE_NO_WARNINGS   // Unsafe CRT Library functions
#endif

#include <stdlib.h>
#include <getopt.h>		// Provided by CMake on Windows
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <panzi/portable_endian.h>
#include <libnvbk/libnvbk.h>
#include <libnvbk/nvbk_error.h>

#define GNSS_RF_ID          0xFF
#define GNSS_ENTRY_LEN      0x4D
#define GNSS_ENTRY_DATA_LEN 5

unsigned char gnss_entry[GNSS_ENTRY_LEN - sizeof(uint32_t) - GNSS_ENTRY_DATA_LEN] = {
  0x02, 0x19, 0xFF, 0x18, 0x01, 0x00, 0x38, 0x00, 0x2F, 0x6E, 0x76, 0x2F, 0x69, 0x74, 0x65, 0x6D,
  0x5F, 0x66, 0x69, 0x6C, 0x65, 0x73, 0x2F, 0x67, 0x70, 0x73, 0x2F, 0x63, 0x67, 0x70, 0x73, 0x2F,
  0x6D, 0x65, 0x2F, 0x67, 0x6E, 0x73, 0x73, 0x5F, 0x6D, 0x75, 0x6C, 0x74, 0x69, 0x62, 0x61, 0x6E,
  0x64, 0x5F, 0x63, 0x6F, 0x6E, 0x66, 0x69, 0x67, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x00,
  0x02, 0x00, 0x05, 0x00
};

char gnss_old_data[GNSS_ENTRY_DATA_LEN] = {0x07, 0x00, 0x00, 0x00, 0x00};
char gnss_new_data[GNSS_ENTRY_DATA_LEN] = {0x07, 0x27, 0x00, 0x00, 0x00};

#define GETOPT_STRING	"hi:o:f"

void usage(char* name) {
  printf("%s: %s [-h|-i INFILE -o OUTFILE [-f]]\n", name, name);
  printf("Patch OPPO/realme/OnePlus NVBK to enable GNSS L5/E5a/B2a bands.\n");
}

void error(int err) {
  printf("Error: %s\n", strerror(err));
  exit(2);
}

int main(int argc, char** argv) {
  int opt, res;
  char* infile = NULL;
  char* outfile = NULL;
  bool force = false, done = false;
  nvbk_header_t* header;
  unsigned char* ptr;
  uint32_t len;

  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }

  while ((opt = getopt(argc, argv, GETOPT_STRING)) != -1) {
    switch (opt) {
    case 'i':
      if ((infile = strdup(optarg)) == NULL) error(errno);
      break;
    case 'o':
      if ((outfile = strdup(optarg)) == NULL) error(errno);
      break;
    case 'f':
      force = true;
      break;
    default:
      usage(argv[0]);
      exit(opt != 'h' ? 1 : 0);
    }
  }

  if (!infile || !outfile) {
    usage(argv[0]);
    exit(1);
  }

  header = nvbk_open(infile, &res);
  if (header == NULL || res != 0) {
    printf("Error: %s\n", nvbk_strerror(res));
  }
  else {
    if (header->type == NVBK_TYPE_STATIC) {
      int i;
      for (i = 0; i < header->num_entries; i++) {
        if (header->entries[i].rf_id == GNSS_RF_ID) break;
      }
      if (i < header->num_entries) {
        printf("Found RF_ID 0x%02X: entry %d, size 0x%lX, offset 0x%lX.\n", GNSS_RF_ID, i, header->entries[i].size, header->entries[i].pos);

        res = nvbk_read_entry(header, i);
        if (res != NVBK_ERR_OK) {
          printf("Error: %s\n", nvbk_strerror(res));
        }
        else {
          ptr = header->entries[i].entry_data;
          while (ptr < header->entries[i].entry_data + header->entries[i].size) {
            len = le32toh(*(uint32_t*)ptr);
            if (len == 0) break;
            if (ptr + len <= header->entries[i].entry_data + header->entries[i].size && len == GNSS_ENTRY_LEN) {
              if (!memcmp(ptr + sizeof(uint32_t), gnss_entry, GNSS_ENTRY_LEN - sizeof(uint32_t) - GNSS_ENTRY_DATA_LEN)) {
                printf("Found gnss_multiband_configuration at pos 0x%tX.\n", ptr - header->entries[i].entry_data + header->entries[i].pos);

                unsigned char* data_start = ptr + GNSS_ENTRY_LEN - GNSS_ENTRY_DATA_LEN;
                if (!memcmp(data_start, gnss_old_data, GNSS_ENTRY_DATA_LEN)) {
                  printf("gnss_multiband_configuration data match!\n");

                  memcpy(data_start, gnss_new_data, GNSS_ENTRY_DATA_LEN);
                  res = nvbk_update_hash(header, i);
                  if (res != NVBK_ERR_OK) {
                    printf("Error: %s\n", nvbk_strerror(res));
                  }
                  else {
                    res = nvbk_write_to_file(header, outfile, force);
                    if (res != NVBK_ERR_OK) {
                      printf("Error: %s\n", nvbk_strerror(res));
                    }
                    else {
                      done = true;
                      printf("Success!!!\n");
                    }
                  }
                }
                else {
                  printf("gnss_multiband_configuration data does not match:");
                  for (int j = 0; j < GNSS_ENTRY_DATA_LEN; j++) {
                    printf(" 0x%02X", *(data_start + j));
                  }
                  printf("\n");
                }
                break;
              }
            }
            ptr += len;
          }
        }
      }
    }
  }

  nvbk_free_header(header);
  if (infile) free(infile);
  if (outfile) free(outfile);

  if (!done) return 3;
  return 0;
}
