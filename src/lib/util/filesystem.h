// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef UTIL_FILESYSTEM_H
#define UTIL_FILESYSTEM_H

#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <exceptions/exceptions.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace isc {
namespace util {

struct FileInfo {
    std::string name_;
    std::string full_path_;
};

struct Realpath {
    static std::string get(std::string const &path) {
        std::unique_ptr<char[]> absolute_path(realpath(path.c_str(), nullptr));
        if (!absolute_path) {
            isc_throw(isc::Unexpected, std::strerror(errno));
        }
        return absolute_path.get();
    }

private:
    Realpath() = delete;
};

struct filename_t {
    filename_t() {
        filename_ = new char[FILENAME_MAX];
    }

    filename_t(std::size_t const size) {
        filename_ = new char[size];
    }

    ~filename_t() {
        delete[] filename_;
    }

    char *const &get() {
        return filename_;
    }

    char &operator[](std::size_t const i) {
        return filename_[i];
    }

    char *filename_;
};

struct Filesystem {
    static void createDirectory(std::string const &path) {
        // read/write/search permissions for owner and group,
        // and with read/search permissions for others
        std::filesystem::create_directories(path);
    }

    static std::vector<FileInfo> directoriesAtPath(std::string const &directory_path) {
        // Open directory.
        DIR *dir(opendir(directory_path.c_str()));
        if (dir == NULL) {
            isc_throw(isc::BadValue, "directory does not exist: '" << directory_path << "'");
        }

        std::vector<FileInfo> result;
        while (true) {
            // Get contents of directory.
            dirent *ent(readdir(dir));
            if (ent == nullptr) {
                break;
            }

            // Skip current directory '.'.
            if (ent->d_name[0] == '.') {
                continue;
            }

            // Get absolute path.
            std::string absolute_path(Realpath::get(directory_path + '/' + ent->d_name));

            // For any error, continue.
            struct stat st;
            if (stat(absolute_path.c_str(), &st) == -1) {
                isc_throw(isc::Unexpected, std::strerror(errno));
            }

            // Check if it should be populated.
            const bool is_directory = (st.st_mode & S_IFDIR) != 0;
            if (is_directory) {
                FileInfo directory;
                directory.name_ = ent->d_name;
                directory.full_path_ = absolute_path;
                result.push_back(directory);
            }
        }

        closedir(dir);
        return result;
    }

    static bool exists(std::string const &path) {
        try {
            return std::filesystem::exists(path) || std::filesystem::is_directory(path);
        } catch (std::filesystem::filesystem_error const &) {
            // It's probably "name too long". Assume the file doesn't exist.
            return false;
        }
    }

    /// @brief Reads symbolic link from /proc/$pid/exe to get the absolute path
    ///     to the directory of the executable which is currently running.
    static std::string getDirname() {
        // Get /proc/<pid>/exe path.
        std::stringstream proc_path_stream;
        proc_path_stream << "/proc/" << getpid() << "/exe";
        std::string proc_path = proc_path_stream.str();

        // Get path to executable.
        filename_t exe_path;
        ssize_t bytes = readlink(proc_path.c_str(), exe_path.get(), FILENAME_MAX);
        if (bytes < 0) {
            bytes = 0;
        }
        exe_path[bytes] = '\0';

        return std::string(dirname(exe_path.get()));
    }

    static void removeRecursively(std::string const &name) {
        if (!exists(name)) {
            return;
        }
        std::filesystem::remove_all(std::filesystem::path(name));
    }

    template <typename T>
    static void removeRecursively(std::string const &name, T const &&condition) {
        if (!exists(name)) {
            return;
        }
        for (std::filesystem::path const &p :
             std::filesystem::recursive_directory_iterator(std::filesystem::path(name))) {
            if (!std::filesystem::is_directory(p)) {
                if (condition(p.filename().string())) {
                    std::filesystem::remove(p);
                }
            }
        }
    }
};

struct Dir {
    Dir() : dirname_(), persistent_(true) {
    }

    Dir(std::string const &dir_name, bool const use_template = false, bool persistent = true)
        : dirname_(), persistent_(persistent) {
        // Do we want to use templates?
        if (use_template) {
            filename_t dirname_c_str;
            strcpy(dirname_c_str.get(), dir_name.c_str());
            if (mkdtemp(dirname_c_str.get())) {
                dirname_ = std::string(dirname_c_str.get());
            } else {
                isc_throw(isc::Unexpected,
                          "could not resolve template on directory '" + dir_name + "'");
            }
        } else {
            dirname_ = dir_name;
        }

        // It either exists or it is created.
        if (!Filesystem::exists(dirname_)) {
            Filesystem::createDirectory(dirname_);
        }

        // Get absolute path.
        dirname_ = Realpath::get(dirname_);
    }

    ~Dir() {
        if (dirname_.empty() || persistent_) {
            return;
        }
        Filesystem::removeRecursively(dirname_);
    }

    std::string operator()() const {
        return dirname_;
    }

private:
    /// @brief non-copyable
    /// @{
    Dir(Dir const &) = delete;
    Dir &operator=(Dir const &) = delete;
    /// @}

    std::string dirname_;
    bool persistent_;
};

struct File {
    File() : filename_(), persistent_(true) {
    }

    File(std::string const &file_name, bool const persistent, std::string const &content)
        : File(file_name, content, false, std::string(), persistent) {
    }

    File(std::string const &file_name,
         std::string const &content = std::string(),
         bool const use_template = false,
         std::string const &termination = std::string(),
         bool const persistent = true)
        : filename_(), persistent_(persistent) {
        // Do we want to use templates?
        if (use_template) {
            filename_t filename_c_str(file_name.length() + 1);
            strcpy(filename_c_str.get(), file_name.c_str());
            close(mkstemps(filename_c_str.get(), termination.length()));
            filename_ = std::string(filename_c_str.get());
        } else {
            filename_ = file_name;
        }

        try {
            if (content.empty()) {
                std::ofstream used_only_to_create_file(filename_,
                                                       std::ios_base::out | std::ios_base::app);
                used_only_to_create_file.exceptions(std::fstream::badbit | std::fstream::failbit);
            } else {
                *this << content;
            }
        } catch (std::ios_base::failure const &exception) {
            isc_throw(isc::Exception, exception.what()
                                          << ": " << filename_ << ": " << std::strerror(errno));
        }

        // Get absolute path.
        filename_ = Realpath::get(filename_);
    }

    ~File() {
        if (filename_.empty() || persistent_) {
            return;
        }
        Filesystem::removeRecursively(filename_);
    }

    std::string operator()() const {
        return filename_;
    }

    template <typename T>
    void operator>>(T &result) {
        if (filename_.empty()) {
            isc_throw(isc::Unexpected,
                      "initialize your File with the non-default constructor first");
        }
        std::ifstream in(filename_);
        in.exceptions(std::fstream::badbit | std::fstream::failbit);
        in.seekg(0, in.beg);
        if (in.peek() != std::ifstream::traits_type::eof()) {
            // Read only if file isn't empty.
            in >> result;
        }
        in.close();
    }

    void operator>>(std::string &result) {
        if (filename_.empty()) {
            isc_throw(isc::Unexpected,
                      "initialize your File with the non-default constructor first");
        }
        std::ifstream in(filename_);
        in.exceptions(std::fstream::badbit | std::fstream::failbit);
        in.seekg(0, in.beg);
        if (in.peek() != std::ifstream::traits_type::eof()) {
            // Read only if file isn't empty.
            result =
                std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }
        in.close();
    }

    template <typename T>
    void operator<<(T const &content) {
        if (filename_.empty()) {
            isc_throw(isc::Unexpected,
                      "initialize your File with the non-default constructor first");
        }
        std::ofstream out(filename_, std::ios_base::out | std::ios_base::trunc);
        out.exceptions(std::fstream::badbit | std::fstream::failbit);
        out << content;
        out.flush();
        out.close();
    }

private:
    std::string filename_;
    bool persistent_;

    /// @brief non-copyable
    /// @{
    File(File const &) = delete;
    File &operator=(File const &) = delete;
    /// @}
};

}  // namespace util
}  // namespace isc

#endif  // UTIL_FILESYSTEM_H
