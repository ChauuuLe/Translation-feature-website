#include <iostream>
#include <fstream>
#include <regex>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

#define endl "\n"
const int N = 2e6;

vector<string> documents;
vector<string> paths;
map<pair<string, int>, int> path_map;
int node_count;
vector<int> adjacency_list[N];
ofstream fout1, fout2;

string formatLine(string &s) {
    bool start = false;
    string result = "";
    for (auto &c : s) {
        if (c != ' ') {
            start = true;
        }
        if (start) {
            result += c;
        }
    }
    return result;
}

void processFilePath(string s) {
    string current_path = "";
    int segment_count = 0;
    int last_node = -1;
    for (int i = 46; i < (int)s.size(); i++) {
        char c = s[i];
        current_path += c;
        if (c == '/' || i == (int)s.size() - 1) {
            segment_count++;
            pair<string, int> segment = {current_path, segment_count};
            if (!path_map.count(segment)) {
                path_map[segment] = ++node_count;
                paths.push_back(current_path);
            }
            if (last_node != -1) {
                adjacency_list[last_node].push_back(path_map[segment]);
            }
            last_node = path_map[segment];
        }
    }
}

void analyzeFile(const string& filePath, const vector<int>& lines_to_check) {
    cerr << "Processing file: " << filePath << endl;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Cannot open: " << filePath << endl;
        return;
    }

    string line;
    int line_number = 0;
    int check_index = 0;
    int relevant_line_count = 0;
    vector<pair<int, string>> relevant_lines;
    while (getline(file, line) && check_index < (int)lines_to_check.size()) {
        line_number++;
        if (line_number == lines_to_check[check_index]) {
            check_index++;
            if (line.size() >= 6 && line.substr(0, 6) == "import") {
                continue;
            } else {
                relevant_line_count++;
                if (relevant_line_count == 1) {
                    fout1 << filePath << endl;
                }
                fout1 << "Line: " << line_number << " " << line << endl;
            }
        }
    }
    if (relevant_line_count > 0) {
        processFilePath(filePath);
    }
}

int visited[N];
vector<int> subtree[N];
int result_count = 0;
bool banned[N];

void showFile(string &s) {
    string result = "";
    for (int i = (int)s.size() - 1; i >= 0; i--) {
        if (s[i] == '/') break;
        result += s[i];
    }
    reverse(result.begin(), result.end());
    fout2 << result << endl;
}

void depthFirstSearch(int u, string current_path, int depth) {
    visited[u] = 1;
    string temp_path = paths[u - 1];
    
    if (adjacency_list[u].empty()) {
        result_count++;
        subtree[u].push_back(u);
    }
    
    bool has_leaf = false;
    for (int &v : adjacency_list[u]) {
        if (visited[v]) continue;
        depthFirstSearch(v, temp_path, depth + 1);
        has_leaf |= adjacency_list[v].empty();
        for (int &leaf : subtree[v]) {
            subtree[u].push_back(leaf);
        }
    } 
    
    if (depth == 3 || (depth < 3 && has_leaf)) {
        fout2 << temp_path << " has files: " << endl;
        for (int &leaf : subtree[u]) {
            if (!banned[leaf]) {
                showFile(paths[leaf - 1]);
                banned[leaf] = 1;
            }
        }
    }
}

void showWordsNeedTranslation() {
    fout1.open("/cd-to-your-app/words_need_translation.csv", ios::out | ios::app);
    string filePath = "";
    vector<int> lines_to_check;
    for (string &line : documents) {
        if (isdigit(line[0])) {
            int line_number = stoi(line);
            lines_to_check.push_back(line_number);
        } else {
            sort(lines_to_check.begin(), lines_to_check.end());
            lines_to_check.erase(unique(lines_to_check.begin(), lines_to_check.end()), lines_to_check.end());
            analyzeFile(filePath, lines_to_check);
            lines_to_check.clear();
            filePath = line;
        }
    }
    if (!filePath.empty()) {
        analyzeFile(filePath, lines_to_check);
    }
}

void showFilesDirectory() {
    fout2.open("/cd-to-your-app/files_need_translation.csv", ios::out | ios::app);
    depthFirstSearch(1, "", 1);
}

int main() {
    freopen("/cd-to-your-app/output.txt", "r", stdin);
    ios::sync_with_stdio(false);
    cin.tie(0);
    string line;
    while (getline(cin, line)) {
        documents.push_back(line);
    }
    showWordsNeedTranslation();
    showFilesDirectory();
    cerr << "Result count: " << result_count << " Nodes processed: " << node_count << endl;
    return 0;
}
