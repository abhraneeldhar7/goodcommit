// key_manager.h - API key storage and retrieval
//
// This file handles saving, loading, and deleting the Groq API key.
// The key is stored in a file on disk, not in environment variables.
//
// Where is the key stored?
//   Windows: %APPDATA%\goodcommit\key
//   Linux:   ~/.config/goodcommit/key
//   macOS:   ~/.config/goodcommit/key
//
// Why store in a file instead of environment variables?
// Environment variables are lost when you close the terminal.
// A file persists across sessions. The user runs "goodcommit --key add"
// once, and the key is available forever (until they delete it).

// Security considerations:
//   - On Linux/macOS, the file is created with mode 0600 (owner read/write only)
//   - On Windows, the file is in the user's AppData folder (only the user can access it)

#ifndef KEY_MANAGER_H
#define KEY_MANAGER_H

#include <string>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

// get_key_path - Returns the full path to the key file.
//
// This is platform-specific:
//   - Windows uses %APPDATA% (usually C:\Users\<you>\AppData\Roaming)
//   - Linux/macOS uses $HOME (usually /home/<you> or /Users/<you>)
//
// Why not hardcode the path?
// Different users have different home directories. Using environment
// variables makes the tool work for everyone without configuration.
static std::string get_key_path() {
#ifdef _WIN32
    const char* appdata = getenv("APPDATA");
    if (!appdata) return "";
    return std::string(appdata) + "\\goodcommit\\key";
#else
    const char* home = getenv("HOME");
    if (!home) return "";
    return std::string(home) + "/.config/goodcommit/key";
#endif
}



// mkdir only creates one directory level. If the parent directories don't exist, we need to create them first. This loop handles that.
static bool ensure_dir(const std::string& path) {
    size_t pos = 0;
#ifdef _WIN32
    while ((pos = path.find('\\', pos + 1)) != std::string::npos) {
        _mkdir(path.substr(0, pos).c_str());
    }
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    while ((pos = path.find('/', pos + 1)) != std::string::npos) {
        mkdir(path.substr(0, pos).c_str(), 0700);
    }
    return mkdir(path.c_str(), 0700) == 0 || errno == EEXIST;
#endif
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

    size_t sep = path.find_last_of("/\\");
    if (sep != std::string::npos) {
        ensure_dir(path.substr(0, sep));
    }

    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << key << "\n";
    f.close();

#ifndef _WIN32
    chmod(path.c_str(), 0600);
#endif
    return true;
}

static bool remove_stored_key() {
    std::string path = get_key_path();
    if (path.empty()) return false;
    return remove(path.c_str()) == 0;
}

#endif
