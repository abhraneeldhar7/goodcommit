#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <filesystem>

#include "goodcommit/prompt.h"
#include "goodcommit/json.h"
#include "goodcommit/key_manager.h"
#include "goodcommit/git_utils.h"
#include "goodcommit/http_client.h"
#include "goodcommit/terminal_ui.h"

static std::string join(const std::vector<std::string> &v, const std::string &sep)
{
    std::string result;
    for (size_t i = 0; i < v.size(); i++)
    {
        if (i > 0)
            result += sep;
        result += v[i];
    }
    return result;
}

static std::string strip_quotes(const std::string &s)
{
    if (s.size() >= 2)
    {
        if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))
        {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

static bool is_dev_mode()
{
    const char *env = std::getenv("ENV");
    return env && std::string(env) == "DEV";
}

static int cmd_help()
{
    const char *BOLD_COLOR = "\033[1m";
    const char *DIM_COLOR = "\033[2m";
    const char *GIT_COLOR = "\033[38;2;241;78;50m";
    const char *HIGHLIGHT_COLOR = "\033[1;32m";
    const char *RESET_COLOR = "\033[0m";

    std::cout
        << "\n\n"
        << GIT_COLOR << "  ██████╗  ██████╗  ██████╗ ██████╗ " << RESET_COLOR << " ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗████████╗\n"
        << GIT_COLOR << " ██╔════╝ ██╔═══██╗██╔═══██╗██╔══██╗" << RESET_COLOR << "██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║╚══██╔══╝\n"
        << GIT_COLOR << " ██║  ███╗██║   ██║██║   ██║██║  ██║" << RESET_COLOR << "██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║   \n"
        << GIT_COLOR << " ██║   ██║██║   ██║██║   ██║██║  ██║" << RESET_COLOR << "██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║   \n"
        << GIT_COLOR << " ╚██████╔╝╚██████╔╝╚██████╔╝██████╔╝" << RESET_COLOR << "╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║   ██║   \n"
        << GIT_COLOR << "  ╚═════╝  ╚═════╝  ╚═════╝ ╚═════╝ " << RESET_COLOR << " ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝   ╚═╝   \n"
        << "\n\n";

    std::cout << "How to use:\n";
    std::cout << BOLD_COLOR << "goodcommit" << DIM_COLOR << " <your_vague_message_here>\n ";
    std::cout << "or just" << BOLD_COLOR << RESET_COLOR << " goodcommit" << "\n\n";

    // std::cout<<"(make sure you have some staged files)";

    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --help" << RESET_COLOR << DIM_COLOR << "      show this tutorial\n";
    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --connect" << RESET_COLOR << DIM_COLOR << "   paste your groq apikey (locally stored)\n";
    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --reset" << RESET_COLOR << DIM_COLOR << "     remove groq apikey\n";
    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --update" << RESET_COLOR << DIM_COLOR << "    download latest release from github\n";
    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --version" << RESET_COLOR << DIM_COLOR << "   show version\n";
    std::cout << RESET_COLOR << "goodcommit" << HIGHLIGHT_COLOR << " --test" << RESET_COLOR << DIM_COLOR << "      show spinner and 3 options to test arrow functionality\n\n";
    std::cout << RESET_COLOR << "visit" << BOLD_COLOR << " \033]8;;https://commit.antk.in\07commit.antk.in\033]8;;\07\n";
    std::cout << RESET_COLOR << "repo" << BOLD_COLOR << " \033]8;;https://github.com/abhraneeldhar7/goodcommit\07github/goodcommit\033]8;;\07\n";
    std::cout << "\n\n";
    return 0;
}

static int cmd_connect()
{
    std::cout << "Paste your groq apikey (locally stored): " << std::flush;
    std::string key;
    if (!std::getline(std::cin, key))
    {
        std::cerr << "Error: failed to read input\n";
        return 1;
    }
    while (!key.empty() && (key.back() == '\n' || key.back() == '\r' || key.back() == ' '))
    {
        key.pop_back();
    }
    if (key.empty())
    {
        std::cerr << "Error: key cannot be empty\n";
        return 1;
    }
    if (store_key(key))
    {
        std::cout << "Key saved.\n";
    }
    else
    {
        std::cerr << "Error: failed to save key\n";
        return 1;
    }
    return 0;
}

static int cmd_reset()
{
    if (remove_stored_key())
    {
        std::cout << "Key removed.\n";
    }
    else
    {
        std::cerr << "No stored key found.\n";
    }
    return 0;
}

static int cmd_version()
{
    std::cout << "goodcommit v1.0.0\n";
    return 0;
}

static int cmd_update()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        std::cerr << "Error: failed to get executable path\n";
        return 1;
    }
    std::string exe_path(buf, len);
    std::string asset = "goodcommit-windows.exe";
#else
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        std::cerr << "Error: failed to get executable path\n";
        return 1;
    }
    buf[len] = '\0';
    std::string exe_path(buf);
#ifdef __APPLE__
    std::string asset = "goodcommit-macos";
#else
    std::string asset = "goodcommit-linux";
#endif
#endif

    std::string new_path = exe_path + ".new";
    std::string old_path = exe_path + ".old";

    std::string url = "https://github.com/abhraneeldhar7/goodcommit/releases/latest/download/" + asset;

    std::error_code ec;
    std::filesystem::remove(old_path, ec);

    std::cout << "Downloading " << asset << "...\n";
    if (!http_download_file(url, new_path))
    {
        std::cerr << "Error: download failed\n";
        std::filesystem::remove(new_path, ec);
        return 1;
    }

    std::filesystem::rename(exe_path, old_path, ec);
    if (ec)
    {
        std::cerr << "Error: failed to rename current binary\n";
        std::filesystem::remove(new_path, ec);
        return 1;
    }

    std::filesystem::rename(new_path, exe_path, ec);
    if (ec)
    {
        std::cerr << "Error: failed to replace binary\n";
        return 1;
    }

    std::cout << "Updated successfully!\n";
    return 0;
}

static int cmd_test()
{
    if (!is_dev_mode())
    {
        std::cerr << "Unknown command. Use --help for usage.\n";
        return 1;
    }
    run_test();
    return 0;
}

static int cmd_generate(const std::vector<std::string> &args)
{
    std::string git_check = run_cmd("git rev-parse --is-inside-work-tree");
    if (trim(git_check) != "true")
    {
        std::cerr << "Error: not a git repository. Run this inside a git repo.\n";
        return 1;
    }

    if (!has_staged_files())
    {
        std::cerr << "Error: No staged files found.\n";
        std::cerr << "Stage files first with: git add <files>\n";
        return 1;
    }

    std::string api_key = read_stored_key();
    if (api_key.empty())
    {
        std::cout << "No API key found.\n";
        return cmd_connect();
    }

    std::string message = strip_quotes(join(args, " "));
    std::string stat = get_staged_stat();

    int file_count = 0;
    {
        std::string f = run_cmd("git diff --cached --name-only");
        for (char c : f)
        {
            if (c == '\n')
                file_count++;
        }
        if (!trim(f).empty() && f.back() != '\n')
            file_count++;
    }

    std::string msg_prefix;
    if (!message.empty())
    {
        msg_prefix = "<UserPrompt>" + message + "</UserPrompt>\n\n";
    }
    msg_prefix += "Staged files (" + std::to_string(file_count) + " files):\n" + stat + "\n\n";

    int total_chars = 260000;
    int safety = 5000;
    int overhead = (int)SYSTEM_PROMPT.size() + (int)msg_prefix.size() + 200;
    int diff_budget = total_chars - safety - overhead;
    if (diff_budget < 2000)
        diff_budget = 2000;

    std::string diffs = get_staged_diffs(diff_budget);
    std::string user_content = msg_prefix + diffs;

    std::string body = std::string("{\"model\":\"openai/gpt-oss-20b\",\"temperature\":0.5,\"messages\":[") + "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"}," + "{\"role\":\"user\",\"content\":\"" + json_escape(user_content) + "\"}" + "]}";

    start_spinner("Generating...");

    std::string response;
    try
    {
        response = http_post("https://api.groq.com/openai/v1/chat/completions", body, api_key);
    }
    catch (const std::exception &e)
    {
        stop_spinner();
        std::cerr << "\nError calling Groq API: " << e.what() << "\n";
        return 1;
    }

    stop_spinner();

    if (response.find("\"error\"") != std::string::npos)
    {
        std::cerr << response << "\n";
        return 1;
    }

    std::string content = extract_json_string(response, "content");
    if (content.empty())
    {
        content = extract_json_string(response, "reasoning");
    }
    if (content.empty())
    {
        std::cerr << "\nError: Failed to extract response from API.\n";
        return 1;
    }

    std::vector<std::string> options = parse_options(content);
    if (options.empty())
    {
        std::string trimmed = trim(content);
        if (!trimmed.empty())
            options.push_back(trimmed);
    }

    if (options.empty())
    {
        std::cerr << "\nError: No valid options found.\n";
        return 1;
    }

    int idx = select_option(options);
    if (idx == -1)
    {
        std::cerr << "\nInvalid selection.\n";
        return 1;
    }

    std::cout << "\nCommitting: " << options[idx] << "\n";
    int ret = do_commit(options[idx]);
    if (ret != 0)
    {
        std::cerr << "git commit failed.\n";
        return 1;
    }
    std::cout << "Done!\n";
    return 0;
}

static int dispatch(int argc, char *argv[])
{
    if (argc < 2)
    {
        return cmd_generate({});
    }

    std::string first = argv[1];

    if (first == "-h")
    {
        return cmd_help();
    }

    if (first.rfind("--", 0) == 0)
    {
        std::string cmd = first.substr(2);
        if (cmd == "help" || cmd == "h")
            return cmd_help();
        if (cmd == "connect")
            return cmd_connect();
        if (cmd == "reset")
            return cmd_reset();
        if (cmd == "version")
            return cmd_version();
        if (cmd == "update")
            return cmd_update();
        if (cmd == "test")
            return cmd_test();
        std::cerr << "Unknown command: " << first << "\n";
        std::cerr << "Use --help for usage.\n";
        return 1;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return cmd_generate(args);
}

#endif
