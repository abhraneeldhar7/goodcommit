#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <csignal>

#include "goodcommit/prompt.h"
#include "goodcommit/json.h"
#include "goodcommit/key_manager.h"
#include "goodcommit/git_utils.h"
#include "goodcommit/http_client.h"
#include "goodcommit/terminal_ui.h"

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

static bool is_dev_mode() {
    const char* env = getenv("ENV");
    return env && std::string(env) == "DEV";
}

static void print_help() {
    std::cout << "Usage: goodcommit [options] <commit message>\n\n";
    std::cout << "Turn a vague commit message into a Conventional Commit.\n\n";
    std::cout << "Options:\n";
    std::cout << "  --key add      Store your Groq API key\n";
    std::cout << "  --key remove   Remove stored API key\n";
    std::cout << "  --print        Print the commit message without committing\n";
    std::cout << "  --json         Output options as JSON (for JS wrapper)\n";
    if (is_dev_mode()) {
        std::cout << "  --test         Test the UI without calling the API (DEV only)\n";
    }
    std::cout << "  --help         Show this help message\n";
    std::cout << "  --             Separator: everything after this is the message\n";
    if (is_dev_mode()) {
        std::cout << "\nDEV mode: set ENV=DEV in your shell environment\n";
    }
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

    
    enable_vt();

    
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h") {
        print_help();
        return 0;
    }

    
    
    
    if (arg1 == "--key") {
        if (argc < 3) {
            std::cerr << "Error: --key requires 'add' or 'remove'\n";
            return 1;
        }
        std::string arg2 = argv[2];

        if (arg2 == "add") {
            std::cout << "Paste your Groq API key: " << std::flush;
            std::string key;
            if (!std::getline(std::cin, key)) {
                std::cerr << "Error: failed to read input\n";
                return 1;
            }
            
            while (!key.empty() && (key.back() == '\n' || key.back() == '\r' || key.back() == ' ')) {
                key.pop_back();
            }
            if (key.empty()) {
                std::cerr << "Error: key cannot be empty\n";
                return 1;
            }
            if (store_key(key)) {
                std::cout << "Key saved.\n";
            } else {
                std::cerr << "Error: failed to save key\n";
                return 1;
            }
            return 0;
        } else if (arg2 == "remove") {
            if (remove_stored_key()) {
                std::cout << "Key removed.\n";
            } else {
                std::cerr << "No stored key found.\n";
            }
            return 0;
        } else {
            std::cerr << "Error: unknown --key option '" << arg2 << "'. Use 'add' or 'remove'.\n";
            return 1;
        }
    }

    
    if (arg1 == "--test") {
        if (!is_dev_mode()) {
            std::cerr << "Error: --test is only available in DEV mode.\n";
            std::cerr << "Set ENV=DEV in your shell environment.\n";
            return 1;
        }
        run_test();
        return 0;
    }

    
    bool print_only = false;
    bool json_mode = false;
    std::vector<std::string> args;
    bool past_separator = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (past_separator) {
            args.push_back(arg);
        } else if (arg == "--") {
            past_separator = true;
        } else if (arg == "--print") {
            print_only = true;
        } else if (arg == "--json") {
            json_mode = true;
        } else if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        } else {
            args.push_back(arg);
        }
    }

    
    if (args.empty()) {
        std::cerr << "Error: No commit message provided.\n";
        std::cerr << "Usage: goodcommit <your vague message>\n";
        return 1;
    }

    
    std::string api_key = read_stored_key();
    if (api_key.empty()) {
        std::cerr << "No API key found.\n";
        std::cerr << "Run 'goodcommit --key add' to store your Groq API key.\n";
        return 1;
    }

    
    if (!has_staged_files()) {
        std::cerr << "Error: No staged files found.\n";
        std::cerr << "Stage files first with: git add <files>\n";
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

    
    
    std::string msg_prefix = "Vague commit message: " + message + "\n\n";
    msg_prefix += "Staged files (" + std::to_string(file_count) + " files):\n" + stat + "\n\n";
    msg_prefix += "Diffs:\n";

    
    
    
    
    int total_chars = 390000;
    int safety = 5000;
    int overhead = (int)SYSTEM_PROMPT.size() + (int)msg_prefix.size() + 200;
    int diff_budget = total_chars - safety - overhead;
    if (diff_budget < 2000) diff_budget = 2000;

    
    std::string diffs = get_staged_diffs(diff_budget);
    std::string user_content = msg_prefix + diffs;

    
    
    std::string body = std::string("{\"model\":\"openai/gpt-oss-20b\",\"temperature\":0.3,\"max_tokens\":32768,\"messages\":[")
        + "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"},"
        + "{\"role\":\"user\",\"content\":\"" + json_escape(user_content) + "\"}"
        + "]}";

    
    start_spinner("Generating...");

    
    std::string response;
    try {
        response = http_post("https://api.groq.com/openai/v1/chat/completions", body, api_key);
    } catch (const std::exception& e) {
        stop_spinner();
        std::cerr << "\nError calling Groq API: " << e.what() << "\n";
        return 1;
    }

    stop_spinner();

    
    if (response.find("\"error\"") != std::string::npos) {
        std::cerr << "\nGroq API error:\n" << response << "\n";
        return 1;
    }

    
    
    std::string content = extract_json_string(response, "content");
    if (content.empty()) {
        content = extract_json_string(response, "reasoning");
    }
    if (content.empty()) {
        std::cerr << "\nError: Failed to extract response from API.\n";
        return 1;
    }

    
    std::vector<std::string> options = parse_options(content);
    if (options.empty()) {
        std::string trimmed = trim(content);
        if (!trimmed.empty()) options.push_back(trimmed);
    }

    if (options.empty()) {
        std::cerr << "\nError: No valid options found.\n";
        return 1;
    }

    
    if (json_mode) {
        
        std::cout << "{\"options\":[";
        for (size_t i = 0; i < options.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << "\"" << json_escape(options[i]) << "\"";
        }
        std::cout << "]}\n";
        return 0;
    }

    if (print_only) {
        
        std::cout << options[0] << "\n";
        return 0;
    }

    
    int idx = select_option(options);
    if (idx == -1) {
        std::cerr << "\nInvalid selection.\n";
        return 1;
    }

    
    std::cout << "\nCommitting: " << options[idx] << "\n";
    int ret = do_commit(options[idx]);
    if (ret != 0) {
        std::cerr << "git commit failed.\n";
        return 1;
    }
    std::cout << "Done!\n";

    return 0;
}
