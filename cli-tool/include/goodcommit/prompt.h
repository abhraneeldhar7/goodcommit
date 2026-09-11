// prompt.h - System prompt for the AI model

// Keeping the prompt in its own file makes it easy to tweak the AI's
// behavior without touching the rest of the code.
// You can experiment with different prompts by editing this one string.
//
// The prompt follows the Conventional Commits specification (v1.0.0).
// See: https://www.conventionalcommits.org/

#ifndef PROMPT_H
#define PROMPT_H

// using namespace std;

#include <string>

// The system prompt is a single string with embedded newlines.
// It instructs the AI to:
//   1. Generate conventional commit messages
//   2. Use the format: type(scope): description
//   3. Return exactly 3 numbered options
//   4. Follow specific rules (imperative mood, lowercase, no period, under 72 chars)
//
// This is sent as the "system" message in the API request.
// The user's message (vague commit message + git diffs) is sent as the "user" message.
const static std::string SYSTEM_PROMPT =
    "You are a commit message generator. Convert vague messages into Conventional Commits (v1.0.0).\n"
    "\n"
    "FORMAT: <type>[optional scope]: <description>\n"
    "Types: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert\n"
    "\n"
    "Rules:\n"
    "- Imperative mood (\"add\" not \"added\")\n"
    "- Lowercase, no period at end\n"
    "- Scope optional in parentheses: feat(parser):\n"
    "- Under 72 characters\n"
    "\n"
    "Return exactly 3 options, first one being most recommended and appropriate one, one per line, numbered:\n"
    "1. <first>\n"
    "2. <second>\n"
    "3. <third>\n"
    "\n"
    "Do NOT include any text before or after the numbered lines.";

#endif
