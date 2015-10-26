#ifndef BASE_FILE_UTILS_H
#define BASE_FILE_UTILS_H

#include <string>
#include <vector>
#include <sys/types.h>

namespace base
{
    namespace file
    {
        // 获取当前目录
        std::string getcwd();

        // 如果指定目录不存在，就创建目录
        bool dir_ensure(const char* dir, mode_t mode);

        // 获取文件内容
        std::string file_get_content(const char* filename);

        // 检查文件是否存在
        bool file_is_exist(const char* filename);

        // 检查文件是否可读
        bool file_can_read(const char* filename);

        // 检查文件是否可写
        bool file_can_write(const char* filename);

        // 检查文件是否可读写
        bool file_can_read_write(const char* filename);

        // 获取目录中的所有文件列表
        std::vector<std::string> dir_get_files(const char* dir);

        // 是否为某种类型的扩展文件
        // ext = ".txt | .exe | .cpp |..."
        bool filename_is_extension(const char* filename, const char* ext);

        // 获取文件短名
        std::string filename_shortname(const std::string& filename);

        // 获取文件扩展名
        std::string filename_extenstion(const std::string& filename);
        
        // 保存文件内容（如果不存在则创建）
        bool file_save_content(const char* filename, const std::string& content);
        
        // 追加文件内容（如果不存在则创建）
        bool file_append_content(const char* filename, const std::string& content);
    }

}

#endif
