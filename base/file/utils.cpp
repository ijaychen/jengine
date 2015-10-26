#include "utils.h"
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <sys/stat.h>
#include <cassert>
#include <string.h>

namespace base
{
    namespace file
    {
        using namespace std;

        std::string getcwd()
        {
            static char buf[256];
            char* dir = ::getcwd(buf, 256);
            assert(dir != NULL);
            return std::string(dir);
        }

        // 检查目录是否存在，如果不存在，则创建
        bool dir_ensure(const char* dir, mode_t mode)
        {
            struct stat st;
            if (stat(dir, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return mkdir(dir, mode) == 0;
            }
        }

        std::string file_get_content(const char* filename)
        {
            string content;
            FILE* f = fopen(filename, "r");
            if (f) {
                size_t bSize = 512;
                char buff[bSize];
                while (true) {
                    size_t s = fread(buff, 1, bSize, f);
                    if (s > 0) {
                        content.append(buff, s);
                    } else {
                        break;
                    }
                }
                fclose(f);
            }

            return content;
        }

        bool file_save_content(const char* filename, const string& content)
        {
            FILE* f = fopen(filename, "w+");
            if (f) {
                fputs(content.c_str(), f);
                fflush(f);
                fclose(f);
                return true;
            }
            return false;
        }

        bool file_append_content(const char* filename, const string& content)
        {
            FILE* f = fopen(filename, "a");
            if (f) {
                fputs(content.c_str(), f);
                fflush(f);
                fclose(f);
                return true;
            }
            return false;
        }


        vector< string > dir_get_files(const char* dir)
        {
            vector<string> ret;
            DIR* dirp = opendir(dir);
            if (dirp != NULL) {
                struct dirent* dircnt;
                struct stat s;
                while ((dircnt = readdir(dirp)) != NULL) {
                    string filename(dir);
                    if (filename[filename.size() - 1] != '/') {
                        filename.append("/");
                    }
                    filename.append(dircnt->d_name);
                    stat(filename.c_str(), &s);
                    if (S_ISREG(s.st_mode)) {
                        ret.push_back(filename);
                    }
                }
                closedir(dirp);
            }
#ifdef HAS_CXX11
            return move(ret);
#else
            return ret;
#endif
        }

        bool filename_is_extension(const char* filename, const char* ext)
        {
            size_t pos = strlen(filename) - strlen(ext);
            return strcmp(filename + pos, ext) == 0;
        }

        bool file_is_exist(const char* filename)
        {
            return access(filename, F_OK) == 0;
        }

        bool file_can_read(const char* filename)
        {
            return access(filename, R_OK) == 0;
        }

        bool file_can_write(const char* filename)
        {
            return access(filename, W_OK) == 0;
        }

        bool file_can_read_write(const char* filename)
        {
            return access(filename, R_OK | W_OK) == 0;
        }

        string filename_extenstion(const string& filename)
        {
            string::size_type pos = filename.rfind('.');
            if (pos == string::npos) {
                return "";
            } else {
                return filename.substr(pos, filename.size() - pos);
            }
        }

        string filename_shortname(const string& filename)
        {
            string::size_type pos1 = filename.rfind('/');
            string::size_type pos2 = filename.rfind('.');
            if (pos2 == string::npos) {
                return "";
            } else {
                if (pos1 == string::npos) {
                    pos1 = 0;
                } else {
                    pos1 += 1;
                }
                return filename.substr(pos1, pos2 - pos1);
            }
        }
    }
}
