/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "QVRConfigLine"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <linux/fuse.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cutils/fs.h>
#include <cutils/hashmap.h>
#include <cutils/log.h>
#include <cutils/multiuser.h>
#include <cutils/properties.h>

#include <private/android_filesystem_config.h>


#define ENABLE_TRACE 1

#if ENABLE_TRACE

#ifdef ANDROID
#define TRACE(x...) ALOGD(x)
#define ERROR(x...) ALOGE(x)
#else
#define ERROR(x...) printf(x)
#define TRACE(x...) printf(x)
#endif

#else
#define TRACE(x...) do {} while (0)
#endif


static int usage() {
    ERROR("usage: sdcard [OPTIONS] <source_path> <label>\n"
            "    -a: specify append line\n"
            "    -l: specify append line length\n"
            "    -d: specify dump enable or not\n"
            "\n");
    return 1;
}

//if append line exist, then no append op.
static bool process(const char* src_file, 
                    const char* append_line,
                    int append_line_len,  
                    bool dump_enable) {
    FILE *fp;
    char *buf = NULL;
    size_t buflen = 0;
    int line_number = 0;
    char *mark_str = NULL;
    char *change_line = "\n";

    if(src_file == NULL) {
        ERROR("Not specified src file ,exit!!!\n");
        return false;
    }
    
    fp = fopen(src_file, "r+");
    if (!fp) {
        ERROR("Could not read(write) %s, error: %s\n",src_file,  strerror(errno));
        return false;
    }
    
    if (append_line) {
        mark_str = (char*)malloc(append_line_len + 1);    
        memset(mark_str,'\0',append_line_len + 1);
        memcpy(mark_str, append_line, append_line_len);
        ERROR("mark_string: [%s]\n", mark_str );
        while ((getline(&buf, &buflen, fp)) > 0) {
            if (strstr(buf,mark_str)) {
            ERROR("already has config line for this property\n");
            free(mark_str);
            goto ret;
            }
        }
        free(mark_str);
        //apend file. todo
        ERROR("no match line , so append it\n");
        fseek(fp, 0L, SEEK_END);
        fwrite(change_line,strlen(change_line),1,fp);
        fwrite(append_line,strlen(append_line),1,fp);
        fwrite(change_line,strlen(change_line),1,fp);
        fseek(fp, 0L, SEEK_SET);
    }    
    
    //dump file
    if (dump_enable) {
        ERROR("Dump File:\n");
        line_number = 0;
        while ((getline(&buf, &buflen, fp)) > 0) {
            ERROR("%d:\t %s", line_number++, buf);   
        }
    }
ret:
    free(buf);
    fclose(fp);
    return true;
}


int main(int argc, char **argv) {
    const char *source_path = NULL;
    const char *append_line = NULL;
    int append_check_len = strlen("force_drift_free_3dof");
    int i;
    bool dump_enable=0;

    int opt;
    while ((opt = getopt(argc, argv, "a:l:d")) != -1) {
        switch (opt) {
            case 'l':
                append_check_len = strtoul(optarg, NULL, 10);
                ERROR("append line check length: %d\n",append_check_len);
                break;
            case 'a':
                append_line = optarg;
                ERROR("append line: %s\n",append_line);
                break;
            case 'd':
                dump_enable = true;
                ERROR("dump enable \n");
                break;
            case '?':
            default:
                return usage();
        }
    }

    for (i = optind; i < argc; i++) {
        char* arg = argv[i];
        if (!source_path) {
            source_path = arg;
        } else {
            ERROR("too many arguments\n");
            return usage();
        }
    }

    if (!source_path) {
        ERROR("no source path specified\n");
        return usage();
    }
    process(source_path, append_line, append_check_len, dump_enable);

    return 1;
}
