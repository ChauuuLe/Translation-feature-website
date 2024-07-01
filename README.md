

## i18next Scanner Configuration with C++ Integration

This configuration is designed to be used with i18next-scanner for extracting translation keys from JavaScript and JSX files in a React project. The configuration will scan specified files, extract keys, and save them into JSON files in the appropriate locale directories. Additionally, a C++ program is provided to further analyze and filter lines that require translation, providing more control over the process.

## Table of Contents

- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Options](#options)
- [Custom Transform](#custom-transform)
- [C++ Integration](#c-integration)
- [Contribution](#contribution)

## Installation

First, install the required npm packages:

```sh
npm install i18next-scanner fs path
```

## Configuration

Create a configuration file (e.g., `i18next-scanner.config.js`) and copy the following content into it:

```javascript
const fs = require('fs');
const path = require('path');
const { Parser } = require('i18next-scanner');

module.exports = {
  input: [
    'app/javascript/**/*.{js,jsx}', // Include all JS and JSX files in app/javascript and subdirectories
  ],
  options: {
    debug: true,
    removeUnusedKeys: true,
    sort: true,
    lngs: ['en', 'de'], // Add other languages as needed
    resource: {
      loadPath: 'public/locales/{{lng}}/{{ns}}.json',
      savePath: 'public/locales/{{lng}}/{{ns}}.json',
      jsonIndent: 2,
    },
    ns: [
      'translation',
    ],
    defaultLng: 'en',
    defaultNs: 'translation',
    interpolation: {
      prefix: '{{',
      suffix: '}}',
    },
    trans: {
      component: 'Trans',
      i18nKey: 'i18nKey',
      extensions: ['.js', '.jsx'],
      fallbackKey: (ns, value) => value, // Use value as key
    },
    func: {
      list: ['i18next.t', 'i18n.t'],
      extensions: ['.js', '.jsx'],
    },
    keySeparator: false, // Use content as keys
    namespaceSeparator: false, // Namespace separator
    defaultValue: (lng, ns, key) => (lng === 'en' ? key : ''),
    skipDefaultValues: (lng) => lng !== 'en',
  },
  transform: function customTransform(file, enc, done) {
    const parser = this.parser;
    const content = file.contents.toString(enc);
    const lines = content.split('\n');

    // Open the output file for appending
    const outputFilePath = path.resolve(__dirname, 'linesNeedToBeConsider.txt');
    const outputStream = fs.createWriteStream(outputFilePath, { flags: 'a' });

    // Log the file path and number of lines
    outputStream.write(`${file.path}\n`);
    //outputStream.write(`Number of lines: ${lines.length}\n`);

    // Use the built-in parser for functions and Trans components
    parser.parseFuncFromString(content, { list: ['i18next.t', 'i18n.t'] });
    parser.parseTransFromString(content);

    // Use custom regex to extract string literals in JS/JSX files
    const regex = /(['"])((?:\\.|[^\\])*?)\1/g;
    const importRegex = /^\s*import\s+.*\s+from\s+['"].*['"];?$/;
    const classRegex = /\.([a-zA-Z0-9_-]+)\s*{/;
    const jsxContentRegex = /<[^>]+>([^<]+)<\/[^>]+>/g;

    let match;
    while ((match = regex.exec(content)) !== null) {
      const key = match[2]; // Extract the content inside quotes
      const precedingContent = content.substring(0, match.index).trim();

      // Skip unwanted content
      if (
        key.includes('@mui') || 
        key.includes('/') || 
        importRegex.test(precedingContent) || 
        classRegex.test(precedingContent) || 
        precedingContent.match(/className\s*=/) || 
        precedingContent.match(/class\s*=/) || 
        precedingContent.match(/styles\s*=\s*\{[^}]*\}/)
      ) {
        continue;
      }

      parser.set(key, key);

      // Find the line number
      const lineNumber = content.substring(0, match.index).split('\n').length;

      outputStream.write(`${lineNumber}\n`);
      //outputStream.write(`Content: ${key}\n`);
    }

    // Use custom regex to capture inner text content within JSX tags
    while ((match = jsxContentRegex.exec(content)) !== null) {
      const key = match[1].trim();
      if (key && !key.includes('@mui') && !key.includes('/') && key.length > 1) {
        const lineNumber = content.substring(0, match.index).split('\n').length;
        parser.set(key, key);

        outputStream.write(`${lineNumber}\n`);
        //outputStream.write(`Content: ${key}\n`);
      }
    }

    outputStream.end(done);
  },
};

```

## Usage

The code help to find words that needs to be translated, show each the files and lines that contains that words. 
The given code only show Files and lines but you can config in code if you want to see the words.
To run i18next-scanner with the provided configuration, use the following command:

```sh
npx i18next-scanner --config i18next-scanner.config.js
```

This command will scan the specified files and extract translation keys into the JSON files located in the `public/locales` directory.

## Options

The `options` object in the configuration contains various settings to customize the scanning and extraction process:

- **debug**: Enable debug mode.
- **removeUnusedKeys**: Remove keys that are not found in the source files.
- **sort**: Sort keys alphabetically.
- **lngs**: List of languages to support (e.g., `['en', 'de']`).
- **resource**: Configuration for loading and saving translation files.
  - **loadPath**: Path to load translation files.
  - **savePath**: Path to save translation files.
  - **jsonIndent**: Indentation level for JSON files.
- **ns**: List of namespaces to use.
- **defaultLng**: Default language.
- **defaultNs**: Default namespace.
- **interpolation**: Configuration for interpolation in translation strings.
- **trans**: Configuration for `Trans` components.
  - **component**: Name of the `Trans` component.
  - **i18nKey**: Key for the `i18nKey` attribute.
  - **extensions**: File extensions to scan.
  - **fallbackKey**: Function to use the value as the key.
- **func**: Configuration for function calls.
  - **list**: List of function names to scan.
  - **extensions**: File extensions to scan.
- **keySeparator**: Use content as keys.
- **namespaceSeparator**: Namespace separator.
- **defaultValue**: Default value for keys.
- **skipDefaultValues**: Skip default values for non-default languages.

## Custom Transform

The `transform` function is a custom transformation that processes each file and extracts translation keys using custom logic. It performs the following steps:

1. **Parse Functions and Trans Components**: Uses the built-in parser to extract keys from function calls (`i18next.t`, `i18n.t`) and `Trans` components.
2. **Custom Regex Extraction**: Uses custom regular expressions to extract string literals and inner text content within JSX tags.
3. **Filter Unwanted Content**: Skips content that matches certain patterns (e.g., imports, class names, styles).
4. **Write to Output File**: Logs file paths and line numbers to an output file (`output.txt`).

## C++ Integration

To further analyze and filter lines that require translation, you can use the following C++ program. This program reads the `linesNeedToBeConsider.txt` generated by i18next-scanner and processes the file paths and line numbers to provide more control and filtering. Also it can provided the each Folder contains which files that contain the words.

### C++ Code

```cpp
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
    for (int i = 0; i < (int)s.size(); i++) {
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

```

### Compiling and Running the C++ Program

1. Save the

 C++ code to a file, e.g., `analyze_translations.cpp`.
2. Compile the C++ program using a C++ compiler like `g++`:

    ```sh
    g++ -o analyze_translations analyze_translations.cpp
    ```

3. Run the compiled program:

    ```sh
    ./analyze_translations
    ```

### Input and Output

- The program reads from an input file (`linesNeedToBeConsider.txt`) generated by i18next-scanner.
- The program writes the analysis results to an output file (`words_need_translation_js.csv`), listing files and lines and words that need translation.
- The program writes the Folder, each Folder show which .js files that contains the words.

## Contribution

If you encounter any issues or have suggestions for improvements, feel free to open an issue or submit a pull request on the project's GitHub repository.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
