#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <csignal>
#include <fstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "git_utils.h"
#include "http_client.h"
#include "terminal_ui.h"
#include "prompt.h"

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + s.size() / 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
        }
    }
    return out;
}

static std::string json_unescape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 'r':  out += '\r'; i++; break;
                case 't':  out += '\t'; i++; break;
                default:   out += s[i]; break;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

static std::string extract_json_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && json[pos] == ' ') pos++;
    if (pos >= json.size() || json[pos] != '"') return "";
    pos++;
    std::string result;
    while (pos < json.size()) {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            result += json[pos];
            result += json[pos + 1];
            pos += 2;
        } else if (json[pos] == '"') {
            break;
        } else {
            result += json[pos];
            pos++;
        }
    }
    return json_unescape(result);
}

static std::vector<std::string> parse_options(const std::string& content) {
    std::vector<std::string> options;
    std::string line;
    for (size_t i = 0; i <= content.size(); i++) {
        if (i == content.size() || content[i] == '\n') {
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
                line.pop_back();
            }
            if (!line.empty() && line[0] >= '1' && line[0] <= '9') {
                size_t dot = line.find(". ");
                if (dot != std::string::npos) {
                    std::string msg = line.substr(dot + 2);
                    while (!msg.empty() && (msg.back() == ' ' || msg.back() == '\r')) {
                        msg.pop_back();
                    }
                    options.push_back(msg);
                }
            }
            line.clear();
        } else {
            line += content[i];
        }
    }
    return options;
}

static std::string join(const std::vector<std::string>& v, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) result += sep;
        result += v[i];
    }
    return result;
}

static std::string strip_quotes(const std::string& s) {
    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

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

static std::string resolve_key() {
    const char* env_key = getenv("GROQ_API_KEY");
    if (env_key && strlen(env_key) > 0) {
        return std::string(env_key);
    }
    return read_stored_key();
}

static void print_help() {
    printf("Usage: goodcommit [options] <commit message>\n\n");
    printf("Turn a vague commit message into a Conventional Commit.\n\n");
    printf("Options:\n");
    printf("  --key add      Store your Groq API key\n");
    printf("  --key remove   Remove stored API key\n");
    printf("  --print        Print the commit message without committing\n");
    printf("  --json         Output options as JSON (for JS wrapper)\n");
    printf("  --help         Show this help message\n");
    printf("  --             Separator: everything after this is the message\n");
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
#else
    struct sigaction sa = {};
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif

    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "--key") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: --key requires 'add' or 'remove'\n");
            return 1;
        }
        if (strcmp(argv[2], "add") == 0) {
            printf("Paste your Groq API key: ");
            fflush(stdout);
            char buf[512];
            if (!fgets(buf, sizeof(buf), stdin)) {
                fprintf(stderr, "Error: failed to read input\n");
                return 1;
            }
            std::string key(buf);
            while (!key.empty() && (key.back() == '\n' || key.back() == '\r' || key.back() == ' ')) {
                key.pop_back();
            }
            if (key.empty()) {
                fprintf(stderr, "Error: key cannot be empty\n");
                return 1;
            }
            if (store_key(key)) {
                printf("Key saved.\n");
            } else {
                fprintf(stderr, "Error: failed to save key\n");
                return 1;
            }
            return 0;
        } else if (strcmp(argv[2], "remove") == 0) {
            if (remove_stored_key()) {
                printf("Key removed.\n");
            } else {
                fprintf(stderr, "No stored key found.\n");
            }
            return 0;
        } else {
            fprintf(stderr, "Error: unknown --key option '%s'. Use 'add' or 'remove'.\n", argv[2]);
            return 1;
        }
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    bool print_only = false;
    bool json_mode = false;
    std::vector<std::string> args;
    bool past_separator = false;

    for (int i = 1; i < argc; i++) {
        if (past_separator) {
            args.push_back(argv[i]);
        } else if (strcmp(argv[i], "--") == 0) {
            past_separator = true;
        } else if (strcmp(argv[i], "--print") == 0) {
            print_only = true;
        } else if (strcmp(argv[i], "--json") == 0) {
            json_mode = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else {
            args.push_back(argv[i]);
        }
    }

    if (args.empty()) {
        fprintf(stderr, "Error: No commit message provided.\n");
        fprintf(stderr, "Usage: goodcommit <your vague message>\n");
        return 1;
    }

    std::string api_key = resolve_key();
    if (api_key.empty()) {
        fprintf(stderr, "No API key found.\n");
        fprintf(stderr, "Run 'goodcommit --key add' to store your Groq API key.\n");
        return 1;
    }

    if (!has_staged_files()) {
        fprintf(stderr, "Error: No staged files found.\n");
        fprintf(stderr, "Stage files first with: git add <files>\n");
        return 1;
    }

    std::string message = strip_quotes(join(args, " "));
    std::string stat = get_staged_stat();
    int file_count = 0;
    {
        std::string f = run_cmd("git diff --cached --name-only");
        for (char c : f) {
            if (c == '\n') file_count++;
        }
        if (!trim(f).empty() && f.back() != '\n') file_count++;
    }

    std::string sys_prompt = json_escape(SYSTEM_PROMPT);
    std::string msg_prefix = "Vague commit message: " + message + "\n\n";
    msg_prefix += "Staged files (" + std::to_string(file_count) + " files):\n" + stat + "\n\n";
    msg_prefix += "Diffs:\n";

    int total_chars = 390000;
    int safety = 5000;
    int overhead = (int)sys_prompt.size() + (int)msg_prefix.size() + 200;
    int diff_budget = total_chars - safety - overhead;
    if (diff_budget < 2000) diff_budget = 2000;

    std::string diffs = get_staged_diffs(diff_budget);

    std::string user_content = msg_prefix + diffs;

    std::string body = std::string("{\"model\":\"openai/gpt-oss-20b\",\"temperature\":0.3,\"max_tokens\":32768,\"messages\":[")
        + "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"},"
        + "{\"role\":\"user\",\"content\":\"" + json_escape(user_content) + "\"}"
        + "]}";

    std::string response;
    try {
        response = http_post("https://api.groq.com/openai/v1/chat/completions", body, api_key);
    } catch (const std::exception& e) {
        fprintf(stderr, "\nError calling Groq API: %s\n", e.what());
        return 1;
    }

    if (response.find("\"error\"") != std::string::npos) {
        fprintf(stderr, "\nGroq API error:\n%s\n", response.c_str());
        return 1;
    }

    std::string content = extract_json_string(response, "content");
    if (content.empty()) {
        content = extract_json_string(response, "reasoning");
    }
    if (content.empty()) {
        fprintf(stderr, "\nError: Failed to extract response from API.\n");
        return 1;
    }

    std::vector<std::string> options = parse_options(content);
    if (options.empty()) {
        std::string trimmed = trim(content);
        if (!trimmed.empty()) options.push_back(trimmed);
    }

    if (options.empty()) {
        fprintf(stderr, "\nError: No valid options found.\n");
        return 1;
    }

    std::string selected;
    if (json_mode) {
        printf("{\"options\":[");
        for (size_t i = 0; i < options.size(); i++) {
            if (i > 0) printf(",");
            printf("\"%s\"", json_escape(options[i]).c_str());
        }
        printf("]}\n");
        return 0;
    } else if (print_only) {
        selected = options[0];
        printf("%s\n", selected.c_str());
    } else {
        selected = options[0];
        printf("%s\n", selected.c_str());
    }

    return 0;
}
