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
const int MAX_NODES = 2e6;

vector<string> documents;
vector<string> paths;
map<pair<string, int>, int> path_map;
int node_count;
vector<int> adjacency_list[MAX_NODES];
vector<int> line_numbers_to_check[MAX_NODES];
ofstream output_file;
int visited[MAX_NODES];
bool banned[MAX_NODES];
int result_count = 0;

// Function to trim leading spaces from a string
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

// Function to process a file path and update the adjacency list
void processFilePath(string s) {
    string current_path = "";
    int segment_count = 0;
    int last_node = -1;
    for (int i = 46; i < (int)s.size(); i++) {
        char c = s[i];
        if (c != '/') {
            current_path += c;
        }
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
            current_path = "";
        }
    }
}

// Function to analyze a file and extract lines that need translation
void analyzeFile(const string& filePath, const vector<int>& lines_to_check) {
    cerr << "Processing file: " << filePath << endl;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filePath << endl;
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
                relevant_lines.push_back({line_number, line});
            }
        }
    }
    if (relevant_line_count > 0) {
        processFilePath(filePath);
    }
}

// Depth-First Search to traverse the directory structure
void depthFirstSearch(int node, string current_path, int depth) {
    visited[node] = 1;
    string full_path = current_path + "/" + paths[node - 1];
    
    if (adjacency_list[node].empty()) {
        result_count++;
        line_numbers_to_check[node].push_back(node);
    }
    
    bool has_leaf = false;
    for (int &neighbor : adjacency_list[node]) {
        if (visited[neighbor]) continue;
        depthFirstSearch(neighbor, full_path, depth + 1);
        has_leaf |= adjacency_list[neighbor].empty();
        for (int &leaf : line_numbers_to_check[neighbor]) {
            line_numbers_to_check[node].push_back(leaf);
        }
    } 
    
    if (depth == 3 || (depth < 3 && has_leaf)) {
        output_file << full_path << " has files: " << endl;
        for (int &leaf : line_numbers_to_check[node]) {
            if (!banned[leaf]) {
                output_file << paths[leaf - 1] << endl;
                banned[leaf] = 1;
            }
        }
    }
}

int main() {
    freopen("/home/chau/path-to-output-file-maded-by-scanner/output.txt", "r", stdin);
    output_file.open("/home/chau/files_need_translation_js.csv", ios::out | ios::app); 
    ios::sync_with_stdio(false);
    cin.tie(0);

    string line;
    while (getline(cin, line)) {
        documents.push_back(line);
    }
    
    string current_file_path = "";
    vector<int> lines_to_check;
    for (string &line : documents) {
        if (isdigit(line[0])) {
            int line_number = stoi(line);
            lines_to_check.push_back(line_number);
        } else {
            sort(lines_to_check.begin(), lines_to_check.end());
            lines_to_check.erase(unique(lines_to_check.begin(), lines_to_check.end()), lines_to_check.end());
            analyzeFile(current_file_path, lines_to_check);
            lines_to_check.clear();
            current_file_path = line;
        }
    }
    if (!current_file_path.empty()) {
        analyzeFile(current_file_path, lines_to_check);
    }
    depthFirstSearch(1, "", 1);
    cerr << "Result count: " << result_count << " Nodes processed: " << node_count << endl;
    return 0;
}
