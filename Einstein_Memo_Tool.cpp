/*
 * ============================================================
 *   EINSTEIN MEMO TOOL V12.0 - EXTENDED EDITION
 *   Author   : juiu9494
 *   Version  : 12.0.0
 *   Build    : 2026-ENHANCED
 *   Language : C++17
 * ============================================================
 *
 *   NEW V12 FEATURES :
 *   - Search history with timestamps
 *   - JSON and CSV export
 *   - Import from text file
 *   - Advanced statistics and dashboards
 *   - Categories / tags for data
 *   - Auto-backup
 *   - Simple RLE compression
 *   - Integrity verification (checksum)
 *   - Multi-user mode (profiles)
 *   - Search by categories
 *   - Advanced sorting and filtering
 *   - Read-only mode (protected profile)
 *   - Notes with priority (high/medium/low)
 *   - Multiple tags per entry
 *   - Audit log
 *   - Result pagination
 *   - Interactive rename mode
 *   - Delete entries
 *   - Update existing entries
 *   - Encrypted export/import
 *   - Debug mode
 *   - Customizable colors
 *   - Keyboard shortcuts
 *   - Table display
 *
 * ============================================================
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <iomanip>
#include <ctime>
#include <functional>
#include <numeric>
#include <random>
#include <regex>
#include <stdexcept>
#include <climits>
#include <cassert>

using namespace std;

// ============================================================
//  GLOBAL CONSTANTS
// ============================================================

const string VERSION                = "12.0.0";
const string BUILD_DATE             = "2026-ENHANCED";
const string MASTER_KEY_A           = "H4ckPr00f_IA_Bl0ck_Ch41n_2026_V9!";
const string MASTER_KEY_B           = "XOR_S3cur1ty_L4y3r_Unbr34k4bl3##";
const string MEMORY_FILE            = "memory_ia.txt";
const string HISTORY_FILE           = "history_ia.txt";
const string AUDIT_FILE             = "audit_ia.txt";
const string PROFILES_FILE          = "profiles_ia.txt";
const string CONFIG_FILE            = "config_ia.txt";
const string BACKUP_PREFIX          = "backup_";
const string TAG_SEPARATOR          = "|";
const int    MAX_RESULTS_PER_PAGE   = 5;
const int    MAX_HISTORY            = 100;
const int    FILE_VERSION           = 3;

// ============================================================
//  ANSI COLOR CODES
// ============================================================

namespace Color {
    const string RESET      = "\033[0m";
    const string BOLD       = "\033[1m";
    const string DIM        = "\033[2m";
    const string UNDERLINE  = "\033[4m";
    const string BLINK      = "\033[5m";

    const string BLACK      = "\033[30m";
    const string RED        = "\033[31m";
    const string GREEN      = "\033[32m";
    const string YELLOW     = "\033[33m";
    const string BLUE       = "\033[34m";
    const string MAGENTA    = "\033[35m";
    const string CYAN       = "\033[36m";
    const string WHITE      = "\033[37m";

    const string BRIGHT_RED   = "\033[1;31m";
    const string BRIGHT_GREEN = "\033[1;32m";
    const string BRIGHT_YELLOW= "\033[1;33m";
    const string BRIGHT_BLUE  = "\033[1;34m";
    const string BRIGHT_MAGENTA="\033[1;35m";
    const string BRIGHT_CYAN  = "\033[1;36m";
    const string BRIGHT_WHITE = "\033[1;37m";

    const string BG_BLACK   = "\033[40m";
    const string BG_RED     = "\033[41m";
    const string BG_GREEN   = "\033[42m";
    const string BG_BLUE    = "\033[44m";
    const string BG_MAGENTA = "\033[45m";
    const string BG_CYAN    = "\033[46m";
}

// ============================================================
//  DATA STRUCTURES
// ============================================================

enum class Priority {
    LOW    = 0,
    MEDIUM = 1,
    HIGH   = 2
};

string priorityToString(Priority p) {
    switch (p) {
        case Priority::HIGH:   return "HIGH";
        case Priority::MEDIUM: return "MEDIUM";
        default:               return "LOW";
    }
}

Priority stringToPriority(const string& s) {
    if (s == "HIGH")   return Priority::HIGH;
    if (s == "MEDIUM") return Priority::MEDIUM;
    return Priority::LOW;
}

string priorityColor(Priority p) {
    switch (p) {
        case Priority::HIGH:   return Color::BRIGHT_RED;
        case Priority::MEDIUM: return Color::BRIGHT_YELLOW;
        default:               return Color::BRIGHT_GREEN;
    }
}

struct DataEntry {
    string          name;
    string          info;
    string          category;
    vector<string>  tags;
    Priority        priority;
    string          creationDate;
    string          modificationDate;
    long long       accessCount;
    unsigned long   checksum;

    DataEntry() : priority(Priority::LOW), accessCount(0), checksum(0) {}

    string tagsToString() const {
        string res;
        for (size_t i = 0; i < tags.size(); ++i) {
            res += tags[i];
            if (i + 1 < tags.size()) res += TAG_SEPARATOR;
        }
        return res;
    }

    void tagsFromString(const string& s) {
        tags.clear();
        if (s.empty()) return;
        stringstream ss(s);
        string token;
        while (getline(ss, token, '|')) {
            if (!token.empty()) tags.push_back(token);
        }
    }
};

struct SearchHistory {
    string      term;
    string      result;
    string      timestamp;
    bool        found;
};

struct UserProfile {
    string  name;
    string  passwordHash;      // stored hashed
    bool    isAdmin;
    bool    isProtected;
    string  creationDate;
};

struct Statistics {
    long long   totalEntries;
    long long   totalSearches;
    long long   totalInjections;
    long long   searchesFound;
    long long   searchesNotFound;
    map<string, long long> accessByCategory;
    map<string, long long> mostSearchedTerms;   // most searched terms
};

struct AppConfig {
    bool    colorsEnabled;
    bool    animationsEnabled;
    bool    debugMode;
    bool    autoBackup;
    int     animationDelayMs;
    string  activeProfile;
    int     maxResultsPerPage;
    bool    auditEnabled;
};

// ============================================================
//  GLOBAL STATE
// ============================================================

AppConfig         gConfig;
UserProfile       gActiveProfile;
Statistics        gStats;
vector<SearchHistory> gHistory;
bool              gDebugMode = false;

// ============================================================
//  UTILITY: DATE / TIME
// ============================================================

string getDateTime() {
    auto now = chrono::system_clock::now();
    time_t t  = chrono::system_clock::to_time_t(now);
    tm*    tm_info = localtime(&t);
    char   buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return string(buf);
}

string getDate() {
    auto now = chrono::system_clock::now();
    time_t t  = chrono::system_clock::to_time_t(now);
    tm*    tm_info = localtime(&t);
    char   buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_info);
    return string(buf);
}

long long getTimestamp() {
    return chrono::duration_cast<chrono::seconds>(
        chrono::system_clock::now().time_since_epoch()
    ).count();
}

// ============================================================
//  UTILITY: STRINGS
// ============================================================

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string toLower(const string& s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

string toUpper(const string& s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::toupper);
    return res;
}

bool startsWith(const string& s, const string& prefix) {
    return s.size() >= prefix.size() &&
           s.compare(0, prefix.size(), prefix) == 0;
}

bool contains(const string& s, const string& substr) {
    return s.find(substr) != string::npos;
}

vector<string> split(const string& s, char sep) {
    vector<string> res;
    stringstream ss(s);
    string token;
    while (getline(ss, token, sep)) res.push_back(token);
    return res;
}

string repeatString(const string& s, int n) {
    string res;
    for (int i = 0; i < n; ++i) res += s;
    return res;
}

string centerText(const string& text, int width, char fill = ' ') {
    int padding = (width - (int)text.size()) / 2;
    if (padding < 0) padding = 0;
    return string(padding, fill) + text + string(width - padding - text.size(), fill);
}

string truncate(const string& s, size_t maxLen, const string& suffix = "...") {
    if (s.size() <= maxLen) return s;
    return s.substr(0, maxLen - suffix.size()) + suffix;
}

// ============================================================
//  ENCRYPTION & SECURITY
// ============================================================

string encryptData(const string& text) {
    string tmp = text;
    for (size_t i = 0; i < tmp.size(); ++i) {
        tmp[i] = tmp[i] ^ MASTER_KEY_A[i % MASTER_KEY_A.size()];
    }
    for (size_t i = 0; i < tmp.size(); ++i) {
        tmp[i] = tmp[i] ^ MASTER_KEY_B[(i + 7) % MASTER_KEY_B.size()];
    }
    return tmp;
}

string decryptData(const string& text) {
    return encryptData(text); // symmetric XOR
}

unsigned long calculateChecksum(const string& s) {
    unsigned long hash = 5381;
    for (char c : s)
        hash = ((hash << 5) + hash) + (unsigned char)c;
    return hash;
}

bool verifyChecksum(const DataEntry& e) {
    unsigned long cs = calculateChecksum(e.name + e.info + e.category);
    return cs == e.checksum;
}

void updateChecksum(DataEntry& e) {
    e.checksum = calculateChecksum(e.name + e.info + e.category);
}

string hashPassword(const string& pwd) {
    unsigned long h = calculateChecksum(pwd + MASTER_KEY_A);
    stringstream ss;
    ss << hex << h;
    return ss.str();
}

// ============================================================
//  SIMPLE RLE COMPRESSION
// ============================================================

string compressRLE(const string& s) {
    if (s.empty()) return "";
    string res;
    int count = 1;
    for (size_t i = 1; i <= s.size(); ++i) {
        if (i < s.size() && s[i] == s[i - 1]) {
            ++count;
        } else {
            if (count > 3)
                res += string(1, s[i - 1]) + "#" + to_string(count) + "#";
            else
                res += string(count, s[i - 1]);
            count = 1;
        }
    }
    return res;
}

string decompressRLE(const string& s) {
    string res;
    size_t i = 0;
    while (i < s.size()) {
        if (i + 1 < s.size() && s[i + 1] == '#') {
            char   c   = s[i];
            size_t end = s.find('#', i + 2);
            if (end == string::npos) { res += s[i]; ++i; continue; }
            int    cnt = stoi(s.substr(i + 2, end - i - 2));
            res += string(cnt, c);
            i = end + 1;
        } else {
            res += s[i++];
        }
    }
    return res;
}

// ============================================================
//  SEARCH ALGORITHMS
// ============================================================

int calculateLevenshteinDistance(const string& s1, const string& s2) {
    int m = s1.size(), n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) dp[i][j] = dp[i - 1][j - 1];
            else dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
        }
    }
    return dp[m][n];
}

double diceScore(const string& s1, const string& s2) {
    if (s1.size() < 2 || s2.size() < 2) return 0.0;
    multiset<string> bi1, bi2;
    for (size_t i = 0; i < s1.size() - 1; ++i)
        bi1.insert(s1.substr(i, 2));
    for (size_t i = 0; i < s2.size() - 1; ++i)
        bi2.insert(s2.substr(i, 2));
    int intersection = 0;
    for (const string& b : bi1)
        if (bi2.count(b)) ++intersection;
    return (2.0 * intersection) / (bi1.size() + bi2.size());
}

bool caseInsensitiveSearch(const string& haystack, const string& needle) {
    return contains(toLower(haystack), toLower(needle));
}

double combinedScore(const string& a, const string& b) {
    int lev   = calculateLevenshteinDistance(toLower(a), toLower(b));
    double dice = diceScore(toLower(a), toLower(b));
    int maxLen = max(a.size(), b.size());
    double simLev = (maxLen == 0) ? 1.0 : 1.0 - (double)lev / maxLen;
    return 0.6 * simLev + 0.4 * dice;
}

// ============================================================
//  TERMINAL INTERFACE / DISPLAY
// ============================================================

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseMs(int ms) {
    if (gConfig.animationsEnabled)
        this_thread::sleep_for(chrono::milliseconds(ms));
}

void displayLine(int width = 60, char ch = '=') {
    cout << " " << Color::CYAN << string(width, ch) << Color::RESET << endl;
}

void displayEinsteinInterface(const string& status) {
    if (!gConfig.colorsEnabled) {
        cout << "=== EINSTEIN MEMO TOOL V" << VERSION << " === [" << status << "]\n";
        return;
    }
    clearScreen();
    cout << "  " << Color::BRIGHT_CYAN
         << "/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\"
         << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "| [=]==============================================[=] |" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "|  "
         << Color::BRIGHT_CYAN
         << "___  _  _  _   _  ____  ___  ____  _  _  _  _ "
         << Color::BRIGHT_MAGENTA << "    |" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "|  "
         << Color::BRIGHT_CYAN
         << "|__  |  |\\ |  _\\  |___  |__  |___  |  |\\ |  | "
         << Color::BRIGHT_MAGENTA << "    |" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "|  "
         << Color::BRIGHT_CYAN
         << "|___ |  | \\|  _\\  |___  ___| |___  |  | \\|  | "
         << Color::BRIGHT_MAGENTA << "    |" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "|                    "
         << Color::BRIGHT_BLUE << "[ MEMO TOOL V" << VERSION << " ]"
         << Color::BRIGHT_MAGENTA << "             |" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "| [=]==============================================[=] |" << Color::RESET << endl;

    string statusTrunc = truncate(status, 43);
    cout << " " << Color::BRIGHT_MAGENTA << "| "
         << Color::BRIGHT_GREEN << " STATUS : " << statusTrunc
         << Color::BRIGHT_MAGENTA << string(44 - statusTrunc.length(), ' ') << "|" << Color::RESET << endl;

    string profile = "PROFILE : " + (gActiveProfile.name.empty() ? "ANONYMOUS" : gActiveProfile.name);
    cout << " " << Color::BRIGHT_MAGENTA << "| "
         << Color::BRIGHT_YELLOW << " " << profile
         << Color::BRIGHT_MAGENTA << string(53 - profile.length(), ' ') << "|" << Color::RESET << endl;

    cout << " " << Color::BRIGHT_MAGENTA << "| "
         << Color::BRIGHT_BLUE << " BUILD : " << BUILD_DATE
         << " | CORE 200TB+ UNLIMITED   "
         << Color::BRIGHT_MAGENTA << "          |" << Color::RESET << endl;

    cout << "  " << Color::BRIGHT_CYAN
         << "\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/"
         << Color::RESET << endl;
}

void executeCyberScan(const string& message) {
    if (!gConfig.animationsEnabled) return;
    string bars[] = {
        "[=         ]", "[==        ]", "[====      ]",
        "[======    ]", "[========  ]", "[==========]"
    };
    for (const string& bar : bars) {
        displayEinsteinInterface(message + " " + bar);
        pauseMs(gConfig.animationDelayMs);
    }
}

void displaySuccessBanner(const string& msg) {
    cout << "\n " << Color::BG_GREEN << Color::BLACK << "  ✔ " << msg << "  " << Color::RESET << endl;
}

void displayErrorBanner(const string& msg) {
    cout << "\n " << Color::BG_RED << Color::BRIGHT_WHITE << "  ✘ " << msg << "  " << Color::RESET << endl;
}

void displayInfoBanner(const string& msg) {
    cout << "\n " << Color::BG_BLUE << Color::BRIGHT_WHITE << "  ℹ " << msg << "  " << Color::RESET << endl;
}

void displayWarningBanner(const string& msg) {
    cout << "\n " << Color::BG_MAGENTA << Color::BRIGHT_WHITE << "  ⚠ " << msg << "  " << Color::RESET << endl;
}

void waitForEnter(const string& prompt = "Press Enter to continue...") {
    cout << "\n " << Color::DIM << prompt << Color::RESET;
    cin.get();
}

void displayTable(const vector<string>& columns, const vector<vector<string>>& rows) {
    // Calculate column widths
    vector<size_t> widths(columns.size(), 0);
    for (size_t c = 0; c < columns.size(); ++c)
        widths[c] = columns[c].size();
    for (const auto& row : rows)
        for (size_t c = 0; c < row.size() && c < columns.size(); ++c)
            widths[c] = max(widths[c], row[c].size());

    // Header
    cout << " " << Color::CYAN;
    for (size_t c = 0; c < columns.size(); ++c)
        cout << "+-" << string(widths[c] + 1, '-');
    cout << "+" << Color::RESET << endl;

    cout << " " << Color::BRIGHT_CYAN;
    for (size_t c = 0; c < columns.size(); ++c)
        cout << "| " << left << setw(widths[c] + 1) << columns[c];
    cout << "|" << Color::RESET << endl;

    cout << " " << Color::CYAN;
    for (size_t c = 0; c < columns.size(); ++c)
        cout << "+-" << string(widths[c] + 1, '-');
    cout << "+" << Color::RESET << endl;

    // Data rows
    for (const auto& row : rows) {
        cout << " ";
        for (size_t c = 0; c < columns.size(); ++c) {
            string cell = (c < row.size()) ? row[c] : "";
            cout << "| " << left << setw(widths[c] + 1) << cell;
        }
        cout << "|" << endl;
    }

    // Footer
    cout << " " << Color::CYAN;
    for (size_t c = 0; c < columns.size(); ++c)
        cout << "+-" << string(widths[c] + 1, '-');
    cout << "+" << Color::RESET << endl;
}

void displayProgress(int current, int total, int barWidth = 40) {
    int filled = (total > 0) ? (current * barWidth / total) : 0;
    cout << " [" << Color::BRIGHT_GREEN
         << string(filled, '#') << string(barWidth - filled, ' ')
         << Color::RESET << "] "
         << current << "/" << total << "\r" << flush;
}

// ============================================================
//  AUDIT LOG MANAGEMENT
// ============================================================

void logAudit(const string& action, const string& details = "") {
    if (!gConfig.auditEnabled) return;
    ofstream f(AUDIT_FILE, ios::app);
    if (!f.is_open()) return;
    f << "[" << getDateTime() << "] "
      << "[" << (gActiveProfile.name.empty() ? "ANON" : gActiveProfile.name) << "] "
      << action;
    if (!details.empty()) f << " | " << details;
    f << "\n";
    f.close();
}

// ============================================================
//  HISTORY MANAGEMENT
// ============================================================

void addHistory(const string& term, const string& result, bool found) {
    SearchHistory h;
    h.term      = term;
    h.result    = result;
    h.timestamp = getDateTime();
    h.found     = found;
    gHistory.push_back(h);

    if ((int)gHistory.size() > MAX_HISTORY)
        gHistory.erase(gHistory.begin());

    // Save to file
    ofstream f(HISTORY_FILE, ios::app);
    if (f.is_open()) {
        f << encryptData(h.timestamp) << "\n"
          << encryptData(h.term)      << "\n"
          << encryptData(h.result)    << "\n"
          << encryptData(found ? "1" : "0") << "\n";
        f.close();
    }
}

void loadHistory() {
    gHistory.clear();
    ifstream f(HISTORY_FILE, ios::binary);
    if (!f.is_open()) return;

    string l1, l2, l3, l4;
    while (getline(f, l1) && getline(f, l2) && getline(f, l3) && getline(f, l4)) {
        SearchHistory h;
        h.timestamp = decryptData(l1);
        h.term      = decryptData(l2);
        h.result    = decryptData(l3);
        h.found     = decryptData(l4) == "1";
        gHistory.push_back(h);
    }
    f.close();
}

void displayHistory(int page = 1) {
    if (gHistory.empty()) {
        displayInfoBanner("No history available.");
        return;
    }

    int total   = (int)gHistory.size();
    int start   = (page - 1) * MAX_RESULTS_PER_PAGE;
    int end     = min(start + MAX_RESULTS_PER_PAGE, total);
    int nbPages = (total + MAX_RESULTS_PER_PAGE - 1) / MAX_RESULTS_PER_PAGE;

    cout << "\n " << Color::BRIGHT_CYAN
         << "=== SEARCH HISTORY (Page " << page << "/" << nbPages << ") ==="
         << Color::RESET << endl;

    vector<string> cols = {"#", "Date/Time", "Term", "Found", "Result"};
    vector<vector<string>> rows;
    for (int i = start; i < end; ++i) {
        const auto& h = gHistory[total - 1 - i]; // most recent first
        rows.push_back({
            to_string(i + 1),
            h.timestamp,
            truncate(h.term, 20),
            h.found ? "YES" : "NO",
            truncate(h.result, 30)
        });
    }
    displayTable(cols, rows);

    cout << "\n " << Color::DIM
         << "[N] Next page  [P] Previous page  [Q] Back"
         << Color::RESET << "\n >> ";
    string nav;
    getline(cin, nav);
    nav = toUpper(trim(nav));
    if (nav == "N" && page < nbPages) displayHistory(page + 1);
    else if (nav == "P" && page > 1)  displayHistory(page - 1);
}

// ============================================================
//  CONFIGURATION MANAGEMENT
// ============================================================

void saveConfig() {
    ofstream f(CONFIG_FILE);
    if (!f.is_open()) return;
    f << "colors="      << gConfig.colorsEnabled      << "\n"
      << "animations="   << gConfig.animationsEnabled   << "\n"
      << "debug="        << gConfig.debugMode           << "\n"
      << "autoBackup="   << gConfig.autoBackup           << "\n"
      << "delay="        << gConfig.animationDelayMs     << "\n"
      << "profile="      << gConfig.activeProfile        << "\n"
      << "maxPage="      << gConfig.maxResultsPerPage    << "\n"
      << "audit="        << gConfig.auditEnabled         << "\n";
    f.close();
}

void loadConfig() {
    // Default values
    gConfig.colorsEnabled      = true;
    gConfig.animationsEnabled  = true;
    gConfig.debugMode          = false;
    gConfig.autoBackup         = true;
    gConfig.animationDelayMs   = 120;
    gConfig.activeProfile      = "ANONYMOUS";
    gConfig.maxResultsPerPage  = MAX_RESULTS_PER_PAGE;
    gConfig.auditEnabled       = true;

    ifstream f(CONFIG_FILE);
    if (!f.is_open()) return;

    string line;
    while (getline(f, line)) {
        auto pos = line.find('=');
        if (pos == string::npos) continue;
        string key = line.substr(0, pos);
        string val = line.substr(pos + 1);
        if (key == "colors")      gConfig.colorsEnabled      = val == "1";
        if (key == "animations")  gConfig.animationsEnabled   = val == "1";
        if (key == "debug")       gConfig.debugMode           = val == "1";
        if (key == "autoBackup")  gConfig.autoBackup          = val == "1";
        if (key == "delay")       gConfig.animationDelayMs    = stoi(val);
        if (key == "profile")     gConfig.activeProfile       = val;
        if (key == "maxPage")     gConfig.maxResultsPerPage   = stoi(val);
        if (key == "audit")       gConfig.auditEnabled        = val == "1";
    }
    f.close();
    gDebugMode = gConfig.debugMode;
}

void displayConfigMenu() {
    displayEinsteinInterface("CONFIGURATION");
    cout << "\n " << Color::BRIGHT_CYAN << "=== SETTINGS ===" << Color::RESET << endl;
    cout << " [1] Colors          : " << (gConfig.colorsEnabled      ? Color::BRIGHT_GREEN + "ON" : Color::BRIGHT_RED + "OFF") << Color::RESET << endl;
    cout << " [2] Animations      : " << (gConfig.animationsEnabled   ? Color::BRIGHT_GREEN + "ON" : Color::BRIGHT_RED + "OFF") << Color::RESET << endl;
    cout << " [3] Debug Mode      : " << (gConfig.debugMode          ? Color::BRIGHT_GREEN + "ON" : Color::BRIGHT_RED + "OFF") << Color::RESET << endl;
    cout << " [4] Auto-Backup     : " << (gConfig.autoBackup         ? Color::BRIGHT_GREEN + "ON" : Color::BRIGHT_RED + "OFF") << Color::RESET << endl;
    cout << " [5] Audit Log       : " << (gConfig.auditEnabled       ? Color::BRIGHT_GREEN + "ON" : Color::BRIGHT_RED + "OFF") << Color::RESET << endl;
    cout << " [6] Animation delay : " << Color::BRIGHT_YELLOW << gConfig.animationDelayMs << " ms" << Color::RESET << endl;
    cout << " [7] Results per page: " << Color::BRIGHT_YELLOW << gConfig.maxResultsPerPage << Color::RESET << endl;
    cout << " [0] Back\n >> ";

    string c;
    getline(cin, c);
    c = trim(c);
    if (c == "1") { gConfig.colorsEnabled      = !gConfig.colorsEnabled;   }
    else if (c == "2") { gConfig.animationsEnabled   = !gConfig.animationsEnabled;  }
    else if (c == "3") { gConfig.debugMode = !gConfig.debugMode; gDebugMode = gConfig.debugMode; }
    else if (c == "4") { gConfig.autoBackup = !gConfig.autoBackup; }
    else if (c == "5") { gConfig.auditEnabled = !gConfig.auditEnabled; }
    else if (c == "6") {
        cout << " New delay (ms) : ";
        string v; getline(cin, v);
        try { gConfig.animationDelayMs = stoi(v); } catch (...) {}
    }
    else if (c == "7") {
        cout << " Max results per page : ";
        string v; getline(cin, v);
        try { gConfig.maxResultsPerPage = max(1, stoi(v)); } catch (...) {}
    }
    saveConfig();
}

// ============================================================
//  USER PROFILE MANAGEMENT
// ============================================================

vector<UserProfile> loadProfiles() {
    vector<UserProfile> profiles;
    ifstream f(PROFILES_FILE, ios::binary);
    if (!f.is_open()) return profiles;

    string l1, l2, l3, l4, l5;
    while (getline(f, l1) && getline(f, l2) && getline(f, l3) && getline(f, l4) && getline(f, l5)) {
        UserProfile p;
        p.name         = decryptData(l1);
        p.passwordHash = decryptData(l2);
        p.isAdmin      = decryptData(l3) == "1";
        p.isProtected  = decryptData(l4) == "1";
        p.creationDate = decryptData(l5);
        profiles.push_back(p);
    }
    f.close();
    return profiles;
}

void saveProfiles(const vector<UserProfile>& profiles) {
    ofstream f(PROFILES_FILE, ios::binary | ios::trunc);
    if (!f.is_open()) return;
    for (const auto& p : profiles) {
        f << encryptData(p.name)           << "\n"
          << encryptData(p.passwordHash)   << "\n"
          << encryptData(p.isAdmin  ? "1" : "0") << "\n"
          << encryptData(p.isProtected ? "1" : "0") << "\n"
          << encryptData(p.creationDate)  << "\n";
    }
    f.close();
}

bool loginProfile() {
    auto profiles = loadProfiles();
    if (profiles.empty()) {
        // Create default admin profile
        displayInfoBanner("No profile detected. Creating ADMIN profile.");
        cout << " Admin profile name : ";
        string name; getline(cin, name);
        name = trim(name);
        if (name.empty()) name = "ADMIN";
        cout << " Password           : ";
        string pwd; getline(cin, pwd);
        UserProfile p;
        p.name         = name;
        p.passwordHash = hashPassword(pwd);
        p.isAdmin      = true;
        p.isProtected  = false;
        p.creationDate = getDateTime();
        profiles.push_back(p);
        saveProfiles(profiles);
        gActiveProfile = p;
        logAudit("PROFILE_CREATED", name);
        return true;
    }

    cout << "\n " << Color::BRIGHT_CYAN << "=== LOGIN ===" << Color::RESET << endl;
    cout << " Available profiles :\n";
    for (size_t i = 0; i < profiles.size(); ++i)
        cout << "   [" << i + 1 << "] " << profiles[i].name
             << (profiles[i].isAdmin ? " (ADMIN)" : "") << endl;
    cout << "   [0] Anonymous mode\n >> ";
    string c; getline(cin, c);
    int idx = -1;
    try { idx = stoi(c) - 1; } catch (...) {}

    if (idx < 0 || idx >= (int)profiles.size()) {
        gActiveProfile.name = "ANONYMOUS";
        gActiveProfile.isAdmin = false;
        gActiveProfile.isProtected = false;
        return true;
    }

    cout << " Password : ";
    string pwd; getline(cin, pwd);
    if (hashPassword(pwd) == profiles[idx].passwordHash) {
        gActiveProfile = profiles[idx];
        logAudit("LOGIN", gActiveProfile.name);
        displaySuccessBanner("Welcome, " + gActiveProfile.name + "!");
        pauseMs(800);
        return true;
    } else {
        displayErrorBanner("Incorrect password.");
        pauseMs(1000);
        return false;
    }
}

void profileManagementMenu() {
    if (!gActiveProfile.isAdmin) {
        displayErrorBanner("Access restricted to administrators.");
        waitForEnter();
        return;
    }
    auto profiles = loadProfiles();
    displayEinsteinInterface("PROFILE MANAGEMENT");
    cout << "\n [1] List profiles\n"
         << " [2] Create profile\n"
         << " [3] Delete profile\n"
         << " [4] Change password\n"
         << " [0] Back\n >> ";
    string c; getline(cin, c);

    if (c == "1") {
        cout << "\n";
        vector<string> cols = {"#", "Name", "Admin", "Protected", "Created"};
        vector<vector<string>> rows;
        for (size_t i = 0; i < profiles.size(); ++i)
            rows.push_back({
                to_string(i + 1),
                profiles[i].name,
                profiles[i].isAdmin    ? "YES" : "NO",
                profiles[i].isProtected ? "YES" : "NO",
                profiles[i].creationDate
            });
        displayTable(cols, rows);
        waitForEnter();

    } else if (c == "2") {
        cout << " Name          : "; string name; getline(cin, name);
        cout << " Password      : "; string pwd; getline(cin, pwd);
        cout << " Admin (y/n)   : "; string adm; getline(cin, adm);
        cout << " Protected (y/n): "; string prt; getline(cin, prt);
        UserProfile p;
        p.name         = trim(name);
        p.passwordHash = hashPassword(pwd);
        p.isAdmin      = toLower(trim(adm)) == "y";
        p.isProtected  = toLower(trim(prt)) == "y";
        p.creationDate = getDateTime();
        profiles.push_back(p);
        saveProfiles(profiles);
        displaySuccessBanner("Profile created: " + p.name);
        logAudit("PROFILE_CREATED", p.name);
        waitForEnter();

    } else if (c == "3") {
        cout << " Profile number to delete: ";
        string n; getline(cin, n);
        int idx = -1;
        try { idx = stoi(n) - 1; } catch (...) {}
        if (idx >= 0 && idx < (int)profiles.size()) {
            string name = profiles[idx].name;
            profiles.erase(profiles.begin() + idx);
            saveProfiles(profiles);
            displaySuccessBanner("Profile deleted: " + name);
            logAudit("PROFILE_DELETED", name);
        } else displayErrorBanner("Invalid index.");
        waitForEnter();

    } else if (c == "4") {
        cout << " Profile number: ";
        string n; getline(cin, n);
        int idx = -1;
        try { idx = stoi(n) - 1; } catch (...) {}
        if (idx >= 0 && idx < (int)profiles.size()) {
            cout << " New password: ";
            string pwd; getline(cin, pwd);
            profiles[idx].passwordHash = hashPassword(pwd);
            saveProfiles(profiles);
            displaySuccessBanner("Password updated.");
            logAudit("PASSWORD_CHANGED", profiles[idx].name);
        } else displayErrorBanner("Invalid index.");
        waitForEnter();
    }
}

// ============================================================
//  DATA STORAGE (load / save all entries)
// ============================================================

vector<DataEntry> loadAllEntries() {
    vector<DataEntry> entries;
    ifstream f(MEMORY_FILE, ios::binary);
    if (!f.is_open()) return entries;

    // 9 lines per entry (with tags and metadata)
    string l1, l2, l3, l4, l5, l6, l7, l8, l9;
    while (getline(f, l1) && getline(f, l2) && getline(f, l3) &&
           getline(f, l4) && getline(f, l5) && getline(f, l6) &&
           getline(f, l7) && getline(f, l8) && getline(f, l9)) {
        DataEntry e;
        e.name              = decryptData(l1);
        e.info              = decryptData(l2);
        e.category          = decryptData(l3);
        e.tagsFromString(decryptData(l4));
        e.priority          = stringToPriority(decryptData(l5));
        e.creationDate      = decryptData(l6);
        e.modificationDate  = decryptData(l7);
        try { e.accessCount = stoll(decryptData(l8)); } catch (...) { e.accessCount = 0; }
        try { e.checksum   = stoul(decryptData(l9)); } catch (...) { e.checksum = 0; }
        entries.push_back(e);
    }
    f.close();
    return entries;
}

void saveAllEntries(const vector<DataEntry>& entries) {
    ofstream f(MEMORY_FILE, ios::binary | ios::trunc);
    if (!f.is_open()) {
        displayErrorBanner("Error: cannot write to " + MEMORY_FILE);
        return;
    }
    for (const auto& e : entries) {
        f << encryptData(e.name)                      << "\n"
          << encryptData(e.info)                      << "\n"
          << encryptData(e.category)                  << "\n"
          << encryptData(e.tagsToString())            << "\n"
          << encryptData(priorityToString(e.priority))<< "\n"
          << encryptData(e.creationDate)              << "\n"
          << encryptData(e.modificationDate)          << "\n"
          << encryptData(to_string(e.accessCount))    << "\n"
          << encryptData(to_string(e.checksum))       << "\n";
    }
    f.close();
}

void saveEntry(const DataEntry& e) {
    auto entries = loadAllEntries();

    // Check if name already exists
    for (auto& ex : entries) {
        if (toLower(ex.name) == toLower(e.name)) {
            ex = e;
            ex.modificationDate = getDateTime();
            updateChecksum(ex);
            saveAllEntries(entries);
            return;
        }
    }
    DataEntry copy = e;
    copy.modificationDate = getDateTime();
    if (copy.creationDate.empty()) copy.creationDate = getDateTime();
    updateChecksum(copy);
    entries.push_back(copy);
    saveAllEntries(entries);
}

// ============================================================
//  ADVANCED SEARCH
// ============================================================

struct SearchResult {
    DataEntry entry;
    double    score;
    int       levDistance;
};

vector<SearchResult> advancedSearch(
    const string& term,
    const string& categoryFilter = "",
    const string& tagFilter = "",
    Priority      priorityFilter = Priority::LOW,
    bool          filterByPriority = false
) {
    auto entries = loadAllEntries();
    vector<SearchResult> results;

    for (auto& e : entries) {
        // Filters
        if (!categoryFilter.empty() && toLower(e.category) != toLower(categoryFilter)) continue;
        if (!tagFilter.empty()) {
            bool hasTag = false;
            for (const auto& t : e.tags)
                if (toLower(t) == toLower(tagFilter)) { hasTag = true; break; }
            if (!hasTag) continue;
        }
        if (filterByPriority && e.priority != priorityFilter) continue;

        // Score
        double score = combinedScore(term, e.name);
        int lev = calculateLevenshteinDistance(toLower(term), toLower(e.name));

        // Bonus if substring found in name or info
        if (caseInsensitiveSearch(e.name, term))  score += 0.3;
        if (caseInsensitiveSearch(e.info, term))  score += 0.1;

        // Threshold: keep if score > 0.3 or term is empty
        if (term.empty() || score > 0.3)
            results.push_back({e, score, lev});
    }

    // Sort by decreasing score
    sort(results.begin(), results.end(),
         [](const SearchResult& a, const SearchResult& b) {
             return a.score > b.score;
         });

    return results;
}

void displaySearchResult(const SearchResult& r, bool showScore = false) {
    const auto& e = r.entry;
    cout << "\n " << Color::BRIGHT_CYAN << "[ " << e.name << " ]" << Color::RESET << endl;
    cout << " " << Color::BRIGHT_YELLOW << "Info      : " << Color::RESET << e.info << endl;
    cout << " " << Color::BRIGHT_BLUE  << "Category  : " << Color::RESET << e.category << endl;
    cout << " " << Color::BRIGHT_MAGENTA << "Tags      : " << Color::RESET
         << (e.tags.empty() ? "(none)" : e.tagsToString()) << endl;
    cout << " " << priorityColor(e.priority) << "Priority  : " << priorityToString(e.priority) << Color::RESET << endl;
    cout << " " << Color::DIM << "Created   : " << e.creationDate
         << " | Modified : " << e.modificationDate
         << " | Accesses: " << e.accessCount << Color::RESET << endl;
    if (showScore && gDebugMode)
        cout << " " << Color::DIM << "[ DEBUG ] Score=" << fixed << setprecision(3) << r.score
             << " | Lev=" << r.levDistance << Color::RESET << endl;
}

void searchMode() {
    displayEinsteinInterface("ADVANCED SEARCH MODE");

    cout << "\n " << Color::CYAN << "Search term        : " << Color::RESET;
    string term; getline(cin, term);
    term = trim(term);

    cout << " " << Color::CYAN << "Category (empty=all): " << Color::RESET;
    string cat; getline(cin, cat);

    cout << " " << Color::CYAN << "Tag (empty=all)     : " << Color::RESET;
    string tag; getline(cin, tag);

    executeCyberScan("VECTOR ANALYSIS IN PROGRESS");

    auto results = advancedSearch(term, trim(cat), trim(tag));
    gStats.totalSearches++;

    if (results.empty()) {
        displayEinsteinInterface("NO RESULTS");
        displayErrorBanner("No data matches your search.");
        gStats.searchesNotFound++;
        addHistory(term, "(none)", false);
        logAudit("SEARCH_EMPTY", term);
        waitForEnter();
        return;
    }

    gStats.searchesFound++;
    addHistory(term, results[0].entry.name, true);
    logAudit("SEARCH", term + " => " + results[0].entry.name);

    // Increment access counter
    {
        auto entries = loadAllEntries();
        for (auto& e : entries)
            if (e.name == results[0].entry.name) { ++e.accessCount; break; }
        saveAllEntries(entries);
    }

    displayEinsteinInterface("RESULT(S) FOUND");

    // Pagination
    int total = (int)results.size();
    int nbPages = (total + gConfig.maxResultsPerPage - 1) / gConfig.maxResultsPerPage;
    int page = 1;

    while (true) {
        int start = (page - 1) * gConfig.maxResultsPerPage;
        int end   = min(start + gConfig.maxResultsPerPage, total);
        cout << "\n " << Color::BRIGHT_GREEN << total << " result(s) found"
             << " (Page " << page << "/" << nbPages << ")"
             << Color::RESET << endl;
        displayLine(60);
        for (int i = start; i < end; ++i) {
            if (i == start && results[i].levDistance > 0 && !term.empty())
                cout << " " << Color::BRIGHT_YELLOW << "💡 Suggestion : '"
                     << results[i].entry.name << "' ?" << Color::RESET << endl;
            displaySearchResult(results[i], true);
            displayLine(60, '-');
        }

        if (nbPages <= 1) break;
        cout << " [N] Next  [P] Previous  [Q] Back\n >> ";
        string nav; getline(cin, nav);
        nav = toUpper(trim(nav));
        if (nav == "Q") break;
        if (nav == "N" && page < nbPages) ++page;
        if (nav == "P" && page > 1)       --page;
    }

    waitForEnter();
}

// ============================================================
//  DATA INJECTION MODE
// ============================================================

void injectionMode() {
    if (gActiveProfile.isProtected) {
        displayErrorBanner("Protected mode active: injection forbidden.");
        waitForEnter();
        return;
    }

    displayEinsteinInterface("DATA INJECTION");

    cout << "\n " << Color::CYAN << "Data name          : " << Color::RESET;
    string name; getline(cin, name);
    name = trim(name);
    if (name.empty()) { displayErrorBanner("Empty name invalid."); waitForEnter(); return; }

    cout << " " << Color::CYAN << "Information / value: " << Color::RESET;
    string info; getline(cin, info);

    cout << " " << Color::CYAN << "Category           : " << Color::RESET;
    string cat; getline(cin, cat);

    cout << " " << Color::CYAN << "Tags (comma separated): " << Color::RESET;
    string tagsStr; getline(cin, tagsStr);

    cout << " " << Color::CYAN << "Priority [1=LOW 2=MEDIUM 3=HIGH] : " << Color::RESET;
    string prio; getline(cin, prio);

    Priority pr = Priority::LOW;
    if (prio == "2") pr = Priority::MEDIUM;
    if (prio == "3") pr = Priority::HIGH;

    // Check if entry already exists
    auto entries = loadAllEntries();
    bool exists = false;
    for (const auto& e : entries)
        if (toLower(e.name) == toLower(name)) { exists = true; break; }

    if (exists) {
        cout << " " << Color::BRIGHT_YELLOW << "This entry already exists. Update? (y/n) : " << Color::RESET;
        string rep; getline(cin, rep);
        if (toLower(trim(rep)) != "y") { waitForEnter(); return; }
    }

    executeCyberScan("CASCADE ENCRYPTION");

    DataEntry e;
    e.name       = name;
    e.info       = info;
    e.category   = trim(cat);
    e.priority   = pr;
    e.creationDate = getDateTime();

    // Parse tags separated by commas
    auto ts = split(tagsStr, ',');
    for (auto& t : ts) {
        t = trim(t);
        if (!t.empty()) e.tags.push_back(t);
    }

    updateChecksum(e);
    saveEntry(e);

    executeCyberScan("WRITING TO SECURE SECTOR");
    displayEinsteinInterface("INJECTION SUCCESSFUL");
    displaySuccessBanner("Entry '" + name + "' injected successfully!");
    gStats.totalInjections++;
    gStats.totalEntries++;
    logAudit("INJECTION", name);
    waitForEnter();
}

// ============================================================
//  DELETION MODE
// ============================================================

void deletionMode() {
    if (gActiveProfile.isProtected) {
        displayErrorBanner("Protected mode active: deletion forbidden.");
        waitForEnter(); return;
    }
    if (!gActiveProfile.isAdmin) {
        displayErrorBanner("Deletion restricted to administrators.");
        waitForEnter(); return;
    }

    displayEinsteinInterface("DELETE ENTRY");
    cout << "\n " << Color::CYAN << "Exact name to delete: " << Color::RESET;
    string name; getline(cin, name);
    name = trim(name);

    auto entries = loadAllEntries();
    size_t before = entries.size();
    entries.erase(remove_if(entries.begin(), entries.end(),
        [&](const DataEntry& e) { return toLower(e.name) == toLower(name); }
    ), entries.end());

    if (entries.size() == before) {
        displayErrorBanner("Entry not found: " + name);
    } else {
        cout << " " << Color::BRIGHT_RED << "Confirm deletion of '" << name << "' ? (DELETE) : " << Color::RESET;
        string conf; getline(cin, conf);
        if (conf != "DELETE") { displayWarningBanner("Deletion cancelled."); waitForEnter(); return; }
        saveAllEntries(entries);
        displaySuccessBanner("Entry '" + name + "' deleted.");
        gStats.totalEntries--;
        logAudit("DELETION", name);
    }
    waitForEnter();
}

// ============================================================
//  LIST / BROWSE ENTRIES
// ============================================================

void listMode(int page = 1) {
    auto entries = loadAllEntries();
    if (entries.empty()) {
        displayInfoBanner("Memory is empty.");
        waitForEnter();
        return;
    }

    // Alphabetical sort
    sort(entries.begin(), entries.end(), [](const DataEntry& a, const DataEntry& b) {
        return toLower(a.name) < toLower(b.name);
    });

    int total   = (int)entries.size();
    int nbPages = (total + gConfig.maxResultsPerPage - 1) / gConfig.maxResultsPerPage;
    page = max(1, min(page, nbPages));

    displayEinsteinInterface("ENTRY LIST - Page " + to_string(page) + "/" + to_string(nbPages));

    int start = (page - 1) * gConfig.maxResultsPerPage;
    int end   = min(start + gConfig.maxResultsPerPage, total);

    vector<string> cols = {"#", "Name", "Category", "Priority", "Tags", "Accesses", "Modified"};
    vector<vector<string>> rows;
    for (int i = start; i < end; ++i) {
        const auto& e = entries[i];
        rows.push_back({
            to_string(i + 1),
            truncate(e.name, 20),
            truncate(e.category, 12),
            priorityToString(e.priority),
            truncate(e.tagsToString(), 15),
            to_string(e.accessCount),
            e.modificationDate.substr(0, 10)
        });
    }
    displayTable(cols, rows);

    cout << "\n " << Color::DIM
         << "[N] Next  [P] Previous  [V] View details  [Q] Back"
         << Color::RESET << "\n >> ";
    string nav; getline(cin, nav);
    nav = toUpper(trim(nav));

    if (nav == "N" && page < nbPages) listMode(page + 1);
    else if (nav == "P" && page > 1)  listMode(page - 1);
    else if (nav == "V") {
        cout << " Number to display: ";
        string n; getline(cin, n);
        int idx = -1;
        try { idx = stoi(n) - 1; } catch (...) {}
        if (idx >= 0 && idx < total) {
            SearchResult rr{entries[idx], 1.0, 0};
            displaySearchResult(rr);
            waitForEnter();
            listMode(page);
        }
    }
}

// ============================================================
//  EXPORT CSV / JSON
// ============================================================

void exportCSV(const string& fileName = "export_einstein.csv") {
    auto entries = loadAllEntries();
    ofstream f(fileName);
    if (!f.is_open()) {
        displayErrorBanner("Cannot create " + fileName);
        return;
    }

    f << "Name,Information,Category,Tags,Priority,CreationDate,ModificationDate,AccessCount\n";
    for (const auto& e : entries) {
        auto esc = [](const string& s) {
            return "\"" + regex_replace(s, regex("\""), "\"\"") + "\"";
        };
        f << esc(e.name)          << ","
          << esc(e.info)         << ","
          << esc(e.category)     << ","
          << esc(e.tagsToString()) << ","
          << esc(priorityToString(e.priority)) << ","
          << esc(e.creationDate) << ","
          << esc(e.modificationDate) << ","
          << e.accessCount       << "\n";
    }
    f.close();
    displaySuccessBanner("CSV export: " + fileName + " (" + to_string(entries.size()) + " entries)");
    logAudit("EXPORT_CSV", fileName);
}

void exportJSON(const string& fileName = "export_einstein.json") {
    auto entries = loadAllEntries();
    ofstream f(fileName);
    if (!f.is_open()) {
        displayErrorBanner("Cannot create " + fileName);
        return;
    }

    auto escapeJSON = [](const string& s) {
        string res;
        for (char c : s) {
            if      (c == '"')  res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else if (c == '\n') res += "\\n";
            else if (c == '\r') res += "\\r";
            else if (c == '\t') res += "\\t";
            else                res += c;
        }
        return res;
    };

    f << "[\n";
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        f << "  {\n"
          << "    \"name\": \""              << escapeJSON(e.name)          << "\",\n"
          << "    \"info\": \""              << escapeJSON(e.info)         << "\",\n"
          << "    \"category\": \""          << escapeJSON(e.category)     << "\",\n"
          << "    \"tags\": [";
        for (size_t t = 0; t < e.tags.size(); ++t)
            f << "\"" << escapeJSON(e.tags[t]) << "\""
              << (t + 1 < e.tags.size() ? "," : "");
        f << "],\n"
          << "    \"priority\": \""          << priorityToString(e.priority) << "\",\n"
          << "    \"creationDate\": \""      << e.creationDate              << "\",\n"
          << "    \"modificationDate\": \""  << e.modificationDate         << "\",\n"
          << "    \"accessCount\": "         << e.accessCount               << "\n"
          << "  }" << (i + 1 < entries.size() ? "," : "") << "\n";
    }
    f << "]\n";
    f.close();
    displaySuccessBanner("JSON export: " + fileName + " (" + to_string(entries.size()) + " entries)");
    logAudit("EXPORT_JSON", fileName);
}

void exportMenu() {
    displayEinsteinInterface("DATA EXPORT");
    cout << "\n [1] Export CSV\n [2] Export JSON\n [0] Back\n >> ";
    string c; getline(cin, c);
    if (c == "1") {
        cout << " File name (empty = export_einstein.csv) : ";
        string fn; getline(cin, fn);
        fn = trim(fn);
        if (fn.empty()) fn = "export_einstein.csv";
        exportCSV(fn);
    } else if (c == "2") {
        cout << " File name (empty = export_einstein.json) : ";
        string fn; getline(cin, fn);
        fn = trim(fn);
        if (fn.empty()) fn = "export_einstein.json";
        exportJSON(fn);
    }
    waitForEnter();
}

// ============================================================
//  IMPORT FROM TEXT FILE (format name|info|category|tags)
// ============================================================

void importFromFile(const string& path) {
    ifstream f(path);
    if (!f.is_open()) {
        displayErrorBanner("Cannot open " + path);
        return;
    }

    int success = 0, failures = 0;
    string line;
    while (getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        auto parts = split(line, '|');
        if (parts.size() < 2) { ++failures; continue; }

        DataEntry e;
        e.name      = trim(parts[0]);
        e.info      = trim(parts[1]);
        e.category  = (parts.size() > 2) ? trim(parts[2]) : "";
        if (parts.size() > 3) {
            auto tags = split(parts[3], ',');
            for (auto& t : tags) { t = trim(t); if (!t.empty()) e.tags.push_back(t); }
        }
        e.priority     = (parts.size() > 4) ? stringToPriority(trim(parts[4])) : Priority::LOW;
        e.creationDate = getDateTime();
        updateChecksum(e);
        saveEntry(e);
        ++success;
        displayProgress(success, success + failures);
    }
    cout << endl;
    f.close();
    displaySuccessBanner(to_string(success) + " entries imported, " + to_string(failures) + " errors.");
    logAudit("IMPORT", path + " | " + to_string(success) + " ok");
    gStats.totalEntries += success;
}

void importMenu() {
    displayEinsteinInterface("DATA IMPORT");
    cout << "\n Path to text file (format: name|info|category|tags|priority) :\n >> ";
    string path; getline(cin, path);
    importFromFile(trim(path));
    waitForEnter();
}

// ============================================================
//  BACKUP & RESTORE
// ============================================================

void performBackup() {
    ifstream src(MEMORY_FILE, ios::binary);
    if (!src.is_open()) return;
    string ts = getDate();
    string destName = BACKUP_PREFIX + ts + "_ia.txt";
    ofstream dst(destName, ios::binary);
    dst << src.rdbuf();
    src.close();
    dst.close();
    displaySuccessBanner("Backup created: " + destName);
    logAudit("BACKUP", destName);
}

void restoreBackup(const string& path) {
    ifstream src(path, ios::binary);
    if (!src.is_open()) {
        displayErrorBanner("Backup not found: " + path);
        return;
    }
    ofstream dst(MEMORY_FILE, ios::binary | ios::trunc);
    dst << src.rdbuf();
    src.close();
    dst.close();
    displaySuccessBanner("Restored from: " + path);
    logAudit("RESTORE", path);
}

void backupRestoreMenu() {
    displayEinsteinInterface("BACKUP & RESTORE");
    cout << "\n [1] Create a backup now\n"
         << " [2] Restore from a backup\n"
         << " [0] Back\n >> ";
    string c; getline(cin, c);
    if (c == "1") {
        performBackup();
    } else if (c == "2") {
        cout << " Backup file to restore: ";
        string ch; getline(cin, ch);
        cout << " " << Color::BRIGHT_RED
             << "WARNING: This will replace all current data. Confirm? (RESTORE) : "
             << Color::RESET;
        string conf; getline(cin, conf);
        if (conf == "RESTORE") restoreBackup(trim(ch));
        else displayWarningBanner("Restore cancelled.");
    }
    waitForEnter();
}

// ============================================================
//  INTEGRITY CHECK
// ============================================================

void checkIntegrity() {
    executeCyberScan("CHECKSUM VERIFICATION IN PROGRESS");
    auto entries = loadAllEntries();
    int ok = 0, corrupt = 0;
    vector<string> corruptNames;

    for (const auto& e : entries) {
        if (verifyChecksum(e)) ++ok;
        else { ++corrupt; corruptNames.push_back(e.name); }
    }

    displayEinsteinInterface("INTEGRITY REPORT");
    cout << "\n " << Color::BRIGHT_GREEN << "OK       : " << ok      << Color::RESET << endl;
    cout << " "   << Color::BRIGHT_RED   << "Corrupted: " << corrupt << Color::RESET << endl;

    if (!corruptNames.empty()) {
        cout << "\n Corrupted entries :\n";
        for (const auto& n : corruptNames)
            cout << "   - " << Color::BRIGHT_RED << n << Color::RESET << endl;
    } else {
        displaySuccessBanner("Integrity verified. No corruption detected.");
    }
    logAudit("INTEGRITY_CHECK", to_string(corrupt) + " corrupt");
    waitForEnter();
}

// ============================================================
//  STATISTICS
// ============================================================

void statisticsMode() {
    auto entries = loadAllEntries();
    gStats.totalEntries = (long long)entries.size();

    // Categories
    map<string, int> byCategory;
    map<string, int> byPriority;
    int totalTags = 0;
    int totalAccess = 0;

    for (const auto& e : entries) {
        byCategory[e.category.empty() ? "(none)" : e.category]++;
        byPriority[priorityToString(e.priority)]++;
        totalTags   += (int)e.tags.size();
        totalAccess += (int)e.accessCount;
    }

    displayEinsteinInterface("STATISTICS DASHBOARD");

    cout << "\n " << Color::BRIGHT_CYAN << "=== GLOBAL DATA ===" << Color::RESET << endl;
    cout << " Total entries    : " << Color::BRIGHT_GREEN << gStats.totalEntries    << Color::RESET << endl;
    cout << " Total searches   : " << Color::BRIGHT_GREEN << gStats.totalSearches   << Color::RESET << endl;
    cout << " Successful searches: " << Color::BRIGHT_GREEN << gStats.searchesFound   << Color::RESET << endl;
    cout << " Empty searches   : " << Color::BRIGHT_RED   << gStats.searchesNotFound << Color::RESET << endl;
    cout << " Total injections : " << Color::BRIGHT_GREEN << gStats.totalInjections << Color::RESET << endl;
    cout << " Total tags       : " << Color::BRIGHT_GREEN << totalTags   << Color::RESET << endl;
    cout << " Total accesses   : " << Color::BRIGHT_GREEN << totalAccess << Color::RESET << endl;

    if (!byCategory.empty()) {
        cout << "\n " << Color::BRIGHT_CYAN << "=== BY CATEGORY ===" << Color::RESET << endl;
        vector<pair<string, int>> catVec(byCategory.begin(), byCategory.end());
        sort(catVec.begin(), catVec.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
        int maxVal = catVec.empty() ? 1 : catVec[0].second;
        for (const auto& [cat, cnt] : catVec) {
            int bars = (maxVal > 0) ? (cnt * 30 / maxVal) : 0;
            cout << " " << left << setw(18) << truncate(cat, 16)
                 << " " << Color::BRIGHT_GREEN << string(bars, '|') << Color::RESET
                 << " " << cnt << endl;
        }
    }

    if (!byPriority.empty()) {
        cout << "\n " << Color::BRIGHT_CYAN << "=== BY PRIORITY ===" << Color::RESET << endl;
        for (const auto& [prio, cnt] : byPriority)
            cout << " " << left << setw(10) << prio << " : " << cnt << endl;
    }

    // Top 5 most accessed entries
    if (!entries.empty()) {
        auto sorted = entries;
        sort(sorted.begin(), sorted.end(), [](const DataEntry& a, const DataEntry& b) {
            return a.accessCount > b.accessCount;
        });
        cout << "\n " << Color::BRIGHT_CYAN << "=== TOP 5 ENTRIES (accesses) ===" << Color::RESET << endl;
        for (int i = 0; i < min(5, (int)sorted.size()); ++i) {
            cout << " " << (i + 1) << ". " << Color::BRIGHT_YELLOW
                 << truncate(sorted[i].name, 25)
                 << Color::RESET << " (" << sorted[i].accessCount << " accesses)" << endl;
        }
    }

    waitForEnter();
}

// ============================================================
//  UPDATE ENTRY MODE
// ============================================================

void updateMode() {
    if (gActiveProfile.isProtected) {
        displayErrorBanner("Protected mode active: modification forbidden.");
        waitForEnter(); return;
    }

    displayEinsteinInterface("UPDATE ENTRY");
    cout << "\n " << Color::CYAN << "Entry name to modify : " << Color::RESET;
    string name; getline(cin, name);
    name = trim(name);

    auto results = advancedSearch(name);
    if (results.empty()) {
        displayErrorBanner("Entry not found.");
        waitForEnter(); return;
    }

    DataEntry& found = results[0].entry;
    cout << "\n Found entry : " << Color::BRIGHT_CYAN << found.name << Color::RESET << endl;
    cout << " Current info : " << found.info << endl;
    cout << "\n What to modify?\n"
         << " [1] Information\n"
         << " [2] Category\n"
         << " [3] Tags\n"
         << " [4] Priority\n"
         << " [5] Everything\n"
         << " [0] Cancel\n >> ";
    string c; getline(cin, c);

    auto entries = loadAllEntries();
    for (auto& e : entries) {
        if (toLower(e.name) == toLower(found.name)) {
            if (c == "1" || c == "5") {
                cout << " New info : "; getline(cin, e.info);
            }
            if (c == "2" || c == "5") {
                cout << " New category : "; getline(cin, e.category);
                e.category = trim(e.category);
            }
            if (c == "3" || c == "5") {
                cout << " New tags (comma) : ";
                string ts; getline(cin, ts);
                e.tags.clear();
                for (auto& t : split(ts, ',')) {
                    t = trim(t);
                    if (!t.empty()) e.tags.push_back(t);
                }
            }
            if (c == "4" || c == "5") {
                cout << " Priority [1=LOW 2=MEDIUM 3=HIGH] : ";
                string p; getline(cin, p);
                if (p == "2") e.priority = Priority::MEDIUM;
                else if (p == "3") e.priority = Priority::HIGH;
                else e.priority = Priority::LOW;
            }
            e.modificationDate = getDateTime();
            updateChecksum(e);
            break;
        }
    }
    saveAllEntries(entries);
    displaySuccessBanner("Entry '" + found.name + "' updated.");
    logAudit("UPDATE", found.name);
    waitForEnter();
}

// ============================================================
//  BROWSE BY CATEGORIES
// ============================================================

void browseByCategories() {
    auto entries = loadAllEntries();
    set<string> cats;
    for (const auto& e : entries)
        cats.insert(e.category.empty() ? "(no category)" : e.category);

    if (cats.empty()) {
        displayInfoBanner("No categories available.");
        waitForEnter(); return;
    }

    displayEinsteinInterface("BROWSE BY CATEGORIES");
    cout << "\n";
    int i = 1;
    for (const string& c : cats)
        cout << " [" << i++ << "] " << c << endl;
    cout << " [0] Back\n >> ";

    string ch; getline(cin, ch);
    int idx = -1;
    try { idx = stoi(ch) - 1; } catch (...) {}

    if (idx < 0) return;
    auto catVec = vector<string>(cats.begin(), cats.end());
    if (idx >= (int)catVec.size()) return;

    string chosenCat = catVec[idx];
    if (chosenCat == "(no category)") chosenCat = "";

    auto results = advancedSearch("", chosenCat);
    displayEinsteinInterface("CATEGORY : " + catVec[idx]);
    cout << "\n " << results.size() << " entry(ies)\n";
    displayLine();
    for (const auto& r : results) {
        displaySearchResult(r);
        displayLine(60, '-');
    }
    waitForEnter();
}

// ============================================================
//  AUDIT LOG (display)
// ============================================================

void displayAuditLog(int nbLines = 20) {
    ifstream f(AUDIT_FILE);
    if (!f.is_open()) {
        displayInfoBanner("Audit log empty or nonexistent.");
        waitForEnter(); return;
    }

    vector<string> lines;
    string l;
    while (getline(f, l)) lines.push_back(l);
    f.close();

    displayEinsteinInterface("AUDIT LOG");
    cout << "\n " << Color::BRIGHT_CYAN
         << "Last " << min(nbLines, (int)lines.size()) << " events:"
         << Color::RESET << endl << endl;

    int start = max(0, (int)lines.size() - nbLines);
    for (int i = start; i < (int)lines.size(); ++i)
        cout << " " << Color::DIM << lines[i] << Color::RESET << endl;

    waitForEnter();
}

// ============================================================
//  DEBUG MODE
// ============================================================

void displayDebugInfo() {
    if (!gDebugMode) { displayErrorBanner("Debug mode disabled."); waitForEnter(); return; }
    displayEinsteinInterface("DEBUG INFO");
    auto entries = loadAllEntries();
    cout << "\n Memory file      : " << MEMORY_FILE << endl
         << " History file     : " << HISTORY_FILE << endl
         << " Audit file       : " << AUDIT_FILE << endl
         << " Profiles file    : " << PROFILES_FILE << endl
         << " Config file      : " << CONFIG_FILE << endl
         << " Entries loaded   : " << entries.size() << endl
         << " History entries  : " << gHistory.size() << endl
         << " Active profile   : " << gActiveProfile.name << endl
         << " Admin            : " << (gActiveProfile.isAdmin    ? "YES" : "NO") << endl
         << " Protected        : " << (gActiveProfile.isProtected ? "YES" : "NO") << endl
         << " Colors           : " << (gConfig.colorsEnabled     ? "YES" : "NO") << endl
         << " Animations       : " << (gConfig.animationsEnabled  ? "YES" : "NO") << endl
         << " Animation delay  : " << gConfig.animationDelayMs << " ms" << endl
         << " Version          : " << VERSION << endl
         << " Build            : " << BUILD_DATE << endl
         << " Date / Time      : " << getDateTime() << endl;
    waitForEnter();
}

// ============================================================
//  FULL TEXT SEARCH
// ============================================================

void fullTextSearch() {
    displayEinsteinInterface("FULL TEXT SEARCH");
    cout << "\n " << Color::CYAN << "Keyword to search in infos : " << Color::RESET;
    string keyword; getline(cin, keyword);
    keyword = trim(keyword);
    if (keyword.empty()) { waitForEnter(); return; }

    auto entries = loadAllEntries();
    vector<DataEntry> found;
    for (const auto& e : entries)
        if (caseInsensitiveSearch(e.info, keyword) || caseInsensitiveSearch(e.name, keyword))
            found.push_back(e);

    displayEinsteinInterface("FULL TEXT RESULTS");
    if (found.empty()) {
        displayErrorBanner("No results for: " + keyword);
    } else {
        cout << "\n " << Color::BRIGHT_GREEN << found.size() << " result(s)" << Color::RESET << endl;
        displayLine();
        for (const auto& e : found) {
            SearchResult r{e, 1.0, 0};
            displaySearchResult(r);
            displayLine(60, '-');
        }
    }
    logAudit("FULLTEXT", keyword);
    waitForEnter();
}

// ============================================================
//  RENAME ENTRY
// ============================================================

void renameMode() {
    if (gActiveProfile.isProtected || !gActiveProfile.isAdmin) {
        displayErrorBanner("Insufficient permissions.");
        waitForEnter(); return;
    }

    displayEinsteinInterface("RENAME ENTRY");
    cout << "\n " << Color::CYAN << "Old name : " << Color::RESET;
    string oldName; getline(cin, oldName);
    oldName = trim(oldName);

    cout << " " << Color::CYAN << "New name : " << Color::RESET;
    string newName; getline(cin, newName);
    newName = trim(newName);

    if (oldName.empty() || newName.empty()) {
        displayErrorBanner("Invalid names.");
        waitForEnter(); return;
    }

    auto entries = loadAllEntries();
    bool found = false;
    for (auto& e : entries) {
        if (toLower(e.name) == toLower(oldName)) {
            e.name              = newName;
            e.modificationDate = getDateTime();
            updateChecksum(e);
            found = true;
            break;
        }
    }

    if (found) {
        saveAllEntries(entries);
        displaySuccessBanner("'" + oldName + "' renamed to '" + newName + "'");
        logAudit("RENAME", oldName + " => " + newName);
    } else {
        displayErrorBanner("Entry '" + oldName + "' not found.");
    }
    waitForEnter();
}

// ============================================================
//  CLEAN DUPLICATES
// ============================================================

void cleanDuplicates() {
    if (!gActiveProfile.isAdmin) {
        displayErrorBanner("Admin access required.");
        waitForEnter(); return;
    }

    executeCyberScan("ANALYZING DUPLICATES");
    auto entries = loadAllEntries();
    map<string, int> seen;
    vector<DataEntry> unique;
    int removed = 0;

    for (const auto& e : entries) {
        string key = toLower(e.name);
        if (seen.count(key)) {
            ++removed;
        } else {
            seen[key] = 1;
            unique.push_back(e);
        }
    }

    if (removed == 0) {
        displaySuccessBanner("No duplicates detected.");
    } else {
        saveAllEntries(unique);
        displaySuccessBanner(to_string(removed) + " duplicate(s) removed.");
        logAudit("CLEAN_DUPLICATES", to_string(removed) + " removed");
        gStats.totalEntries -= removed;
    }
    waitForEnter();
}

// ============================================================
//  HELP
// ============================================================

void displayHelp() {
    displayEinsteinInterface("HELP & DOCUMENTATION");
    cout << "\n " << Color::BRIGHT_CYAN << "=== EINSTEIN MEMO TOOL V" << VERSION << " ===" << Color::RESET << endl;
    cout << R"(
 MAIN FEATURES :
 ─────────────────────────────────────────────────────
 [1] Fuzzy Search        : Finds entries even with
                           typos (Levenshtein + Dice)
 [2] Injection           : Adds or updates an entry with
                           category, tags, and priority
 [3] List                : Browse all entries in a table
 [4] Full Text           : Search inside the info content
 [5] Categories          : Browse by category
 [6] Update              : Modify an existing entry
 [7] Delete              : Remove an entry (ADMIN)
 [8] Rename              : Rename an entry (ADMIN)
 [9] History             : Displays recent searches
 [A] Statistics          : Complete dashboard
 [B] Export              : CSV or JSON
 [C] Import              : From text file (name|info|cat|tags)
 [D] Backup / Restore    : Save and restore data
 [E] Integrity           : Checksum verification
 [F] Audit Log           : System events
 [G] Configuration       : Colors, animations, debug...
 [H] Profiles            : Multi-user management (ADMIN)
 [I] Clean duplicates    : Deduplication (ADMIN)
 [J] Debug               : System info (debug mode on)
 [Q] Quit

 IMPORT FILE FORMAT :
 ─────────────────────────────────────────────────────
   name|information|category|tag1,tag2|PRIORITY
   Example : Einstein|Famous physicist|Science|physics,history|HIGH

 SEARCH ALGORITHMS :
 ─────────────────────────────────────────────────────
   - Levenshtein distance (typo tolerance)
   - Dice score (bigram similarity)
   - Combined score (60% Levenshtein + 40% Dice)
   - Substring bonus (partial match)
   - Configurable similarity threshold

 SECURITY :
 ─────────────────────────────────────────────────────
   - Double XOR encryption (MASTER_KEY_A + B)
   - djb2 checksum per entry
   - Profiles with hashed passwords
   - Timestamped audit log
   - Read-only mode (protected profile)
)";
    waitForEnter();
}

// ============================================================
//  MAIN MENU
// ============================================================

void displayMainMenu() {
    displayEinsteinInterface("WAITING FOR INSTRUCTIONS...");
    cout << "\n";
    cout << " " << Color::BRIGHT_BLUE  << "[1]" << Color::RESET << " Search             "
         << Color::BRIGHT_MAGENTA    << "[2]" << Color::RESET << " Inject             "
         << Color::BRIGHT_GREEN      << "[3]" << Color::RESET << " List entries\n";

    cout << " " << Color::BRIGHT_CYAN  << "[4]" << Color::RESET << " Full Text          "
         << Color::BRIGHT_YELLOW     << "[5]" << Color::RESET << " Categories         "
         << Color::BRIGHT_BLUE       << "[6]" << Color::RESET << " Update\n";

    cout << " " << Color::BRIGHT_RED   << "[7]" << Color::RESET << " Delete             "
         << Color::BRIGHT_MAGENTA    << "[8]" << Color::RESET << " Rename             "
         << Color::BRIGHT_GREEN      << "[9]" << Color::RESET << " History\n";

    cout << " " << Color::BRIGHT_CYAN  << "[A]" << Color::RESET << " Statistics         "
         << Color::BRIGHT_YELLOW     << "[B]" << Color::RESET << " Export             "
         << Color::BRIGHT_BLUE       << "[C]" << Color::RESET << " Import\n";

    cout << " " << Color::BRIGHT_MAGENTA << "[D]" << Color::RESET << " Backup/Restore     "
         << Color::BRIGHT_GREEN      << "[E]" << Color::RESET << " Integrity          "
         << Color::BRIGHT_CYAN       << "[F]" << Color::RESET << " Audit Log\n";

    cout << " " << Color::BRIGHT_YELLOW << "[G]" << Color::RESET << " Configuration      "
         << Color::BRIGHT_BLUE       << "[H]" << Color::RESET << " Profiles           "
         << Color::BRIGHT_MAGENTA    << "[I]" << Color::RESET << " Clean duplicates\n";

    cout << " " << Color::DIM         << "[J]" << Color::RESET << " Debug              "
         << Color::BRIGHT_GREEN      << "[?]" << Color::RESET << " Help               "
         << Color::BRIGHT_RED        << "[Q]" << Color::RESET << " Quit\n";

    cout << "\n >> ";
}

// ============================================================
//  MAIN ENTRY POINT
// ============================================================

int main() {
    // --- Load config ---
    loadConfig();

    // --- Startup screen ---
    executeCyberScan("INITIALIZING QUANTUM CORE V" + VERSION);
    executeCyberScan("VERIFYING DISK SECTORS");
    executeCyberScan("DECRYPTING COMPARTMENTS");

    // --- Profile login ---
    bool connected = false;
    for (int attempts = 0; attempts < 3 && !connected; ++attempts)
        connected = loginProfile();
    if (!connected) {
        displayErrorBanner("Too many failed attempts. Shutting down.");
        return 1;
    }

    // --- Load history ---
    loadHistory();

    // --- Automatic backup at startup ---
    if (gConfig.autoBackup) performBackup();

    // --- Main loop ---
    string choice;
    while (true) {
        displayMainMenu();
        getline(cin, choice);
        choice = toUpper(trim(choice));

        if      (choice == "1") searchMode();
        else if (choice == "2") injectionMode();
        else if (choice == "3") listMode();
        else if (choice == "4") fullTextSearch();
        else if (choice == "5") browseByCategories();
        else if (choice == "6") updateMode();
        else if (choice == "7") deletionMode();
        else if (choice == "8") renameMode();
        else if (choice == "9") displayHistory();
        else if (choice == "A") statisticsMode();
        else if (choice == "B") exportMenu();
        else if (choice == "C") importMenu();
        else if (choice == "D") backupRestoreMenu();
        else if (choice == "E") checkIntegrity();
        else if (choice == "F") displayAuditLog();
        else if (choice == "G") displayConfigMenu();
        else if (choice == "H") profileManagementMenu();
        else if (choice == "I") cleanDuplicates();
        else if (choice == "J") displayDebugInfo();
        else if (choice == "?" || choice == "H" ) displayHelp();
        else if (choice == "Q" || choice == "3" ) {
            executeCyberScan("DESTROYING TEMPORARY LOGS");
            logAudit("LOGOUT", gActiveProfile.name);
            saveConfig();
            if (gConfig.autoBackup) performBackup();
            clearScreen();
            cout << Color::DIM << "[-] Einstein Core disconnected. Data secured.\n" << Color::RESET;
            break;
        }
        else {
            displayWarningBanner("Unknown command. Press [?] for help.");
            pauseMs(600);
        }
    }

    return 0;
}

/*
 * ============================================================
 *   END OF FILE - EINSTEIN MEMO TOOL V12.0
 *   Compilation : g++ -std=c++17 -O2 -o einstein einstein_memo_tool_v12.cpp
 *   Linux/Mac   : ./einstein
 *   Windows     : einstein.exe
 * ============================================================
 */
