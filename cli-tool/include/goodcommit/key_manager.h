#ifndef KEY_MANAGER_H
#define KEY_MANAGER_H

#include <string>
#include <fstream>
#include <filesystem>

static std::string get_key_path() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (!appdata) return "";
    return (std::filesystem::path(appdata) / "goodcommit" / "key").string();
#else
    const char* home = std::getenv("HOME");
    if (!home) return "";
    return (std::filesystem::path(home) / ".config" / "goodcommit" / "key").string();
#endif
}

static bool ensure_dir(const std::string& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return !ec;
}

static std::string read_stored_key() {
    std::string path = get_key_path();
    if (path.empty()) return "";
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::string key;
    std::getline(f, key);
    while (!key.empty() && (key.back() == '\n' || key.back() == '\r' || key.back() == ' ')) {
        key.pop_back();
    }
    return key;
}

static bool store_key(const std::string& key) {
    std::string path = get_key_path();
    if (path.empty()) return false;

    
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        ensure_dir(p.parent_path().string());
    }

    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << key << "\n";
    f.close();

#ifndef _WIN32
    
    std::filesystem::permissions(path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace);
#endif
    return true;
}

static bool remove_stored_key() {
    std::string path = get_key_path();
    if (path.empty()) return false;
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

#endif
