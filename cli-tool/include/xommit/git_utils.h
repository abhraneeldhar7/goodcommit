#ifndef GIT_UTILS_H
#define GIT_UTILS_H

#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32

static std::string run_cmd(const char* cmd) {
    std::string result;
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return result;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    char cmd_buf[1024];
    snprintf(cmd_buf, sizeof(cmd_buf), "cmd /c %s", cmd);

    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, cmd_buf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);
        char buf[4096];
        DWORD bytes_read;
        while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytes_read, NULL) && bytes_read > 0) {
            buf[bytes_read] = '\0';
            result += buf;
        }
        CloseHandle(hRead);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        CloseHandle(hRead);
        CloseHandle(hWrite);
    }

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

#else

static std::string run_cmd(const char* cmd) {
    std::string result;
    std::array<char, 4096> buf;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return result;
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
        result += buf.data();
    }
    pclose(pipe);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

#endif

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool has_staged_files() {
    std::string output = run_cmd("git diff --cached --name-only");
    return !trim(output).empty();
}

static std::string get_staged_stat() {
    return run_cmd("git diff --cached --stat");
}

struct FileDiff {
    std::string name;
    std::string diff;
    int lines;
};

static bool is_generated_file(const std::string& name) {
    static const char* patterns[] = {
        "package-lock.json", "yarn.lock", "pnpm-lock.yaml", "composer.lock",
        "Gemfile.lock", "Cargo.lock", "poetry.lock", "go.sum",
        "node_modules/", ".min.js", ".min.css", ".map",
    };
    for (const char* p : patterns) {
        if (name.find(p) != std::string::npos) return true;
    }
    return false;
}

static std::string clean_diff(const std::string& diff) {
    std::string out;
    size_t i = 0;
    while (i < diff.size()) {
        size_t line_end = diff.find('\n', i);
        if (line_end == std::string::npos) line_end = diff.size();
        std::string line = diff.substr(i, line_end - i);
        bool keep = false;
        if (line.rfind("@@", 0) == 0) keep = true;
        else if (line.rfind("+++", 0) == 0) keep = false;
        else if (line.rfind("---", 0) == 0) keep = false;
        else if (line.rfind("+", 0) == 0) keep = true;
        else if (line.rfind("-", 0) == 0) keep = true;
        else if (line.rfind("new file mode", 0) == 0) keep = true;
        else if (line.rfind("deleted file mode", 0) == 0) keep = true;
        else if (line.rfind("rename from", 0) == 0) keep = true;
        else if (line.rfind("rename to", 0) == 0) keep = true;
        else if (line.rfind("Binary files", 0) == 0) keep = true;
        if (keep) {
            out += line;
            out += '\n';
        }
        i = line_end + 1;
    }
    return out;
}

static std::vector<FileDiff> collect_file_diffs() {
    std::vector<FileDiff> files;
    std::string full_diff = run_cmd("git diff --cached");
    if (full_diff.empty()) return files;

    std::string current_file;
    std::string current_diff;
    int current_lines = 0;
    bool in_file = false;

    size_t i = 0;
    while (i < full_diff.size()) {
        if (full_diff[i] == 'd' && full_diff.compare(i, 10, "diff --git ") == 0) {
            if (in_file && !current_file.empty()) {
                files.push_back({current_file, clean_diff(current_diff), current_lines});
            }
            size_t line_end = full_diff.find('\n', i);
            if (line_end == std::string::npos) line_end = full_diff.size();
            std::string line = full_diff.substr(i, line_end - i);
            size_t b_pos = line.rfind(" b/");
            if (b_pos != std::string::npos) {
                current_file = line.substr(b_pos + 3);
            } else {
                current_file = line.substr(11);
            }
            current_diff.clear();
            current_lines = 0;
            in_file = true;
            i = line_end + 1;
        } else {
            size_t line_end = full_diff.find('\n', i);
            if (line_end == std::string::npos) line_end = full_diff.size();
            current_diff += full_diff.substr(i, line_end - i + 1);
            current_lines++;
            i = line_end + 1;
        }
    }

    if (in_file && !current_file.empty()) {
        files.push_back({current_file, clean_diff(current_diff), current_lines});
    }

    return files;
}

static std::string get_staged_diffs(int total_budget) {
    std::vector<FileDiff> all = collect_file_diffs();
    if (all.empty()) return "";

    std::vector<FileDiff> files;
    std::vector<std::string> generated;
    for (const auto& f : all) {
        if (is_generated_file(f.name)) generated.push_back(f.name);
        else files.push_back(f);
    }
    if (files.empty()) {
        files = all;
        generated.clear();
    }

    size_t total_diff_size = 0;
    for (const auto& f : files) {
        total_diff_size += f.diff.size();
    }

    std::string result;

    if ((int)total_diff_size <= total_budget) {
        for (const auto& f : files) {
            result += "<" + f.name + ">\n" + f.diff + "</" + f.name + ">\n\n";
        }
    } else {
        int per_file_min = 400;
        int remaining = total_budget;

        for (size_t idx = 0; idx < files.size(); idx++) {
            const auto& f = files[idx];
            int file_budget;
            if (idx == files.size() - 1) {
                file_budget = remaining;
            } else {
                file_budget = (int)((double)total_budget * ((double)f.diff.size() / (double)total_diff_size));
                if (file_budget < per_file_min) file_budget = per_file_min;
            }
            remaining -= file_budget;

            std::string diff = f.diff;
            if ((int)diff.size() > file_budget) {
                size_t cut = diff.rfind('\n', (size_t)file_budget);
                if (cut == std::string::npos) cut = (size_t)file_budget;
                diff = diff.substr(0, cut) + "\n[...truncated]";
            }
            result += "<" + f.name + ">\n" + diff + "</" + f.name + ">\n\n";
        }
    }

    if (!generated.empty()) {
        result += "<GeneratedFiles>\n";
        for (const auto& g : generated) result += g + "\n";
        result += "</GeneratedFiles>\n\n";
    }

    return result;
}

static int do_commit(const std::string& message) {
    std::string cmd = "git commit -m \"" + message + "\"";
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    char cmd_buf[1024];
    snprintf(cmd_buf, sizeof(cmd_buf), "cmd /c %s", cmd.c_str());
    if (CreateProcessA(NULL, cmd_buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (int)exit_code;
    }
    return -1;
#else
    return pclose(popen(cmd.c_str(), "r"));
#endif
}

#endif
