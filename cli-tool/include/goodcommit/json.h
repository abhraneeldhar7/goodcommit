#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + s.size() / 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;  
            case '\\': out += "\\\\"; break;  
            case '\n': out += "\\n";  break;  
            case '\r': out += "\\r";  break;  
            case '\t': out += "\\t";  break;  
            default:   out += c;      break;  
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
                default:   out += s[i];      break;  
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

#endif
