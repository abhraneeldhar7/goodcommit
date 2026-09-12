#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <filesystem>

#include "xommit/prompt.h"
#include "xommit/json.h"
#include "xommit/key_manager.h"
#include "xommit/git_utils.h"
#include "xommit/http_client.h"
#include "xommit/terminal_ui.h"

#ifndef XOMMIT_VERSION
#define XOMMIT_VERSION "1.0.0"
#endif
#ifndef XOMMIT_GITHUB
#define XOMMIT_GITHUB "https://github.com/abhraneeldhar7/xommit"
#endif
#ifndef XOMMIT_WEBSITE
#define XOMMIT_WEBSITE "https://xommit.antk.in"
#endif
#ifndef XOMMIT_ASSET_WIN
#define XOMMIT_ASSET_WIN "xommit.exe"
#endif
#ifndef XOMMIT_ASSET_LINUX
#define XOMMIT_ASSET_LINUX "xommit"
#endif
#ifndef XOMMIT_ASSET_MACOS
#define XOMMIT_ASSET_MACOS "xommit-macos"
#endif

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
#ifdef XOMMIT_DEV
    return true;
#else
    return false;
#endif
}

static int fail(const std::string &msg)
{
    const char *DIM_COLOR = "\033[2m";
    const char *RESET_COLOR = "\033[0m";
    std::cerr << msg << "\n";
    std::cerr << DIM_COLOR << "Use --help for usage." << RESET_COLOR << "\n";
    return 1;
}

static int cmd_help()
{
    const char *BOLD_COLOR = "\033[1m";
    const char *DIM_COLOR = "\033[2m";
    const char *HIGHLIGHT_COLOR = "\033[1;32m";
    const char *RESET_COLOR = "\033[0m";

    std::cout << "\n\n";
    print_logo();
    std::cout << "\n\n"
              << std::flush;

    std::cout << "How to use:\n";
    std::cout << BOLD_COLOR << "xommit" << DIM_COLOR << " <your_vague_message_here>\n ";
    std::cout << "or just" << BOLD_COLOR << RESET_COLOR << " xommit" << "\n\n";

    // std::cout<<"(make sure you have some staged files)";

    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --help" << RESET_COLOR << DIM_COLOR << "      show this tutorial\n";
    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --connect" << RESET_COLOR << DIM_COLOR << "   paste your groq apikey (locally stored)\n";
    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --reset" << RESET_COLOR << DIM_COLOR << "     remove groq apikey\n";
    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --update" << RESET_COLOR << DIM_COLOR << "    download latest release from github\n";
    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --version" << RESET_COLOR << DIM_COLOR << "   show version\n";
    std::cout << RESET_COLOR << "xommit" << HIGHLIGHT_COLOR << " --test" << RESET_COLOR << DIM_COLOR << "      show spinner and 3 options to test arrow functionality\n\n";
    std::cout << RESET_COLOR << "visit" << BOLD_COLOR << " \033]8;;" XOMMIT_WEBSITE "\07xommit.antk.in\033]8;;\07\n";
    std::cout << RESET_COLOR << "repo" << BOLD_COLOR << " \033]8;;" XOMMIT_GITHUB "\07github/xommit\033]8;;\07\n";
    std::cout << "\n\n";
    return 0;
}

static int cmd_connect()
{
    std::cout << "Paste your groq apikey (locally stored): " << std::flush;
    std::string key = read_password();
    std::cout << "\r\033[2K" << std::flush;

    while (!key.empty() && (key.back() == '\n' || key.back() == '\r' || key.back() == ' '))
    {
        key.pop_back();
    }
    if (key.empty())
    {
        return fail("Error: key cannot be empty");
    }
    if (store_key(key))
    {
        std::cout << "Key saved\n";
    }
    else
    {
        return fail("Error: failed to save key");
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
    std::cout << "xommit v" XOMMIT_VERSION "\n";
    return 0;
}

static int cmd_update()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        return fail("Error: failed to get executable path");
    }
    std::string exe_path(buf, len);
    std::string asset = XOMMIT_ASSET_WIN;
#else
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        return fail("Error: failed to get executable path");
    }
    buf[len] = '\0';
    std::string exe_path(buf);
#ifdef __APPLE__
    std::string asset = XOMMIT_ASSET_MACOS;
#else
    std::string asset = XOMMIT_ASSET_LINUX;
#endif
#endif

    std::string new_path = exe_path + ".new";
    std::string old_path = exe_path + ".old";

    std::string url = XOMMIT_GITHUB "/releases/latest/download/" + asset;

    std::error_code ec;
    std::filesystem::remove(old_path, ec);

    std::cout << "Downloading " << asset << "...\n";
    if (!http_download_file(url, new_path))
    {
        std::filesystem::remove(new_path, ec);
        return fail("Error: download failed");
    }

    std::error_code size_ec;
    auto dl_size = std::filesystem::file_size(new_path, size_ec);
    if (size_ec || dl_size < 100000)
    {
        std::filesystem::remove(new_path, ec);
        return fail("Error: downloaded file looks invalid");
    }

    std::filesystem::rename(exe_path, old_path, ec);
    if (ec)
    {
        std::filesystem::remove(new_path, ec);
        return fail("Error: failed to rename current binary");
    }

    std::filesystem::rename(new_path, exe_path, ec);
    if (ec)
    {
        return fail("Error: failed to replace binary");
    }

    std::cout << "Updated successfully!\n";
    return 0;
}

static int cmd_test()
{
    if (!is_dev_mode())
    {
        return fail("Unknown command: --test");
    }
    run_test();
    return 0;
}

static int cmd_generate(const std::vector<std::string> &args)
{
    std::string git_check = run_cmd("git rev-parse --is-inside-work-tree");
    if (trim(git_check) != "true")
    {
        return fail("Error: not a git repository. Run this inside a git repo.");
    }

    if (!has_staged_files())
    {
        return fail("Error: No staged files found.\nStage files first with: git add <files>");
    }

    std::string api_key = read_stored_key();
    if (api_key.empty())
    {
        std::cout << "No API key found.\n";
        if (cmd_connect() != 0)
        {
            return 1;
        }
        api_key = read_stored_key();
        if (api_key.empty())
        {
            return fail("Error: key was not saved");
        }
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

    int total_chars = 120000;
    int safety = 5000;
    int overhead = (int)SYSTEM_PROMPT.size() + (int)msg_prefix.size() + 200;
    int diff_budget = total_chars - safety - overhead;
    if (diff_budget < 2000)
        diff_budget = 2000;

    std::string diffs = get_staged_diffs(diff_budget);
    std::string user_content = msg_prefix + diffs;

    std::string body = std::string("{\"model\":\"openai/gpt-oss-120b\",\"temperature\":0.3,\"top_p\":0.9,\"messages\":[") + "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"}," + "{\"role\":\"user\",\"content\":\"" + json_escape(user_content) + "\"}" + "]}";

    std::cout << "\n\n";
    print_logo();
    std::cout << "\n\n"
              << std::flush;

    start_spinner("Generating...");

    std::string response;
    try
    {
        response = http_post("https://api.groq.com/openai/v1/chat/completions", body, api_key);
    }
    catch (const std::exception &e)
    {
        stop_spinner();
        return fail(std::string("Error calling Groq API: ") + e.what());
    }

    stop_spinner();

    if (response.find("\"error\"") != std::string::npos)
    {
        return fail(response);
    }

    std::string content = extract_json_string(response, "content");
    if (content.empty())
    {
        content = extract_json_string(response, "reasoning");
    }
    if (content.empty())
    {
        return fail("Error: Failed to extract response from API.");
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
        return fail("Error: No valid options found.");
    }

    int idx = select_option(options);
#ifdef _WIN32
    erase_logo((int)options.size() + (idx == -1 ? 3 : 2));
#else
    erase_logo((int)options.size() + (idx == -1 ? 3 : 1));
#endif

    if (idx == -1)
    {
        return 1;
    }

    std::cout << "\nCommitting: " << options[idx] << "\n";
    int ret = do_commit(options[idx]);
    if (ret != 0)
    {
        return fail("git commit failed.");
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
        return fail("Unknown command: " + first);
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return cmd_generate(args);
}

#endif
