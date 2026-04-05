// src/utils/Config.cpp
#include "utils/Config.h"
#include "utils/Logger.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace trading {

static const char* TAG = "Config";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Config::Config(const std::string& filepath)
    : filepath_(filepath)
{
    load(filepath);
}

// ─────────────────────────────────────────────
//  load() — parse INI file
//
//  Format:
//    [section]        → starts a new section
//    key = value      → key-value pair
//    # comment        → ignored
//    ; comment        → ignored
//    blank lines      → ignored
// ─────────────────────────────────────────────
void Config::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("[Config] Cannot open config file: " + filepath);
    }

    std::string currentSection = "global";
    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        ++lineNum;
        line = trim(line);

        // Skip blanks and comments
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // Section header: [section]
        if (line.front() == '[' && line.back() == ']') {
            currentSection = toLower(trim(line.substr(1, line.size() - 2)));
            continue;
        }

        // Key = value
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            LOG_WARNING(TAG, "Line " + std::to_string(lineNum)
                + ": no '=' found, skipping: " + line);
            continue;
        }

        std::string key = toLower(trim(line.substr(0, eq)));
        std::string val = trim(line.substr(eq + 1));

        // Strip inline comment
        auto commentPos = val.find('#');
        if (commentPos != std::string::npos)
            val = trim(val.substr(0, commentPos));
        commentPos = val.find(';');
        if (commentPos != std::string::npos)
            val = trim(val.substr(0, commentPos));

        data_[currentSection][key] = val;
    }

    int total = 0;
    for (const auto& s : data_) total += (int)s.second.size();
    LOG_INFO(TAG, "Loaded config: " + filepath
        + " | Sections=" + std::to_string(data_.size())
        + " | Keys=" + std::to_string(total));
}

// ─────────────────────────────────────────────
//  Getters
// ─────────────────────────────────────────────
std::string Config::getString(const std::string& section,
                               const std::string& key,
                               const std::string& defaultVal) const
{
    auto sit = data_.find(toLower(section));
    if (sit == data_.end()) return defaultVal;
    auto kit = sit->second.find(toLower(key));
    if (kit == sit->second.end()) return defaultVal;
    return kit->second;
}

double Config::getDouble(const std::string& section,
                          const std::string& key,
                          double defaultVal) const
{
    std::string val = getString(section, key);
    if (val.empty()) return defaultVal;
    try { return std::stod(val); }
    catch (...) {
        LOG_WARNING(TAG, "Cannot parse double for [" + section + "]." + key + "=" + val);
        return defaultVal;
    }
}

int Config::getInt(const std::string& section,
                   const std::string& key,
                   int defaultVal) const
{
    std::string val = getString(section, key);
    if (val.empty()) return defaultVal;
    try { return std::stoi(val); }
    catch (...) {
        LOG_WARNING(TAG, "Cannot parse int for [" + section + "]." + key + "=" + val);
        return defaultVal;
    }
}

bool Config::getBool(const std::string& section,
                     const std::string& key,
                     bool defaultVal) const
{
    std::string val = toLower(getString(section, key));
    if (val.empty()) return defaultVal;
    if (val == "true"  || val == "1" || val == "yes" || val == "on")  return true;
    if (val == "false" || val == "0" || val == "no"  || val == "off") return false;
    LOG_WARNING(TAG, "Cannot parse bool for [" + section + "]." + key + "=" + val);
    return defaultVal;
}

// ─────────────────────────────────────────────
//  Existence checks
// ─────────────────────────────────────────────
bool Config::hasSection(const std::string& section) const {
    return data_.count(toLower(section)) > 0;
}

bool Config::hasKey(const std::string& section, const std::string& key) const {
    auto sit = data_.find(toLower(section));
    if (sit == data_.end()) return false;
    return sit->second.count(toLower(key)) > 0;
}

// ─────────────────────────────────────────────
//  List sections / keys
// ─────────────────────────────────────────────
std::vector<std::string> Config::sections() const {
    std::vector<std::string> out;
    for (const auto& s : data_) out.push_back(s.first);
    return out;
}

std::vector<std::string> Config::keys(const std::string& section) const {
    std::vector<std::string> out;
    auto sit = data_.find(toLower(section));
    if (sit == data_.end()) return out;
    for (const auto& k : sit->second) out.push_back(k.first);
    return out;
}

// ─────────────────────────────────────────────
//  print()
// ─────────────────────────────────────────────
void Config::print() const {
    std::cout << "\n======================================\n";
    std::cout << "  Config: " << filepath_ << "\n";
    std::cout << "======================================\n";
    for (const auto& section : data_) {
        std::cout << "  [" << section.first << "]\n";
        for (const auto& kv : section.second)
            std::cout << "    " << std::left << std::setw(20)
                      << kv.first << " = " << kv.second << "\n";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
std::string Config::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string Config::toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return out;
}

} // namespace trading