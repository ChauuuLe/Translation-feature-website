#include <iostream>
#include<bits/stdc++.h>
#include <fstream>
#include <regex>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<vector>
#include<map>
using namespace std;
#define endl "\n"
const int N = 2e6;


vector<string> doc;
vector<string> str;
map<pair<string,int> ,int> mp;
int n;
vector<int> adj[N];
string formatline (string &s) {
    bool ok = 0;
    string tmp = "";
    for (auto &c : s) {
        if (c!=' ') {
            ok = 1;
        }
        if (ok) {
            tmp += c;
        } 
    }
    return tmp;
}

void cal (string s) {
    //cerr << s << endl;
    string now = "";
    int cnt = 0;
    int last = -1;
    for (int i = 46; i < (int) s.size(); i++) {
        char c = s[i];
        if (c != '/') {
            now += c;
        }
        if (c == '/' || i==(int)s.size() - 1) {
            cnt++;
            pair<string,int> cc = {now, cnt};
            if (!mp.count (cc)) {
                mp[cc] = ++n;
                str.push_back(now);
            }
            if (last!=-1) {
                adj[last].push_back (mp[cc]);
            }
            last = mp[cc];
            now = "";
        }
    }
}
void troll(const string& filePath, const vector<int> dak) {
    cerr << filePath << endl;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Deo mo duoc: " << filePath << endl;
    }
    string line;
    int linenumber = 0;
    int id = 0;
    int cnt = 0;
    vector<pair<int, string>> lines;
    while (getline(file, line) && id < (int) dak.size()) {
        linenumber++;
        if (linenumber == dak[id]) {
            id++;
            //cout << linenumber << " " << id << endl;
            if ((int) line.size() >= 6 && line.substr(0, 6)=="import") {
                continue;
                
            }
            else {
                cnt++;
                lines.push_back({linenumber, line});
            }
        }
       //cout << linenumber << endl;
    }
    if (cnt) {
        cal (filePath);
    }
    //if (id < (int) dak.size()) cout << dak[id] << " " << id << endl;
}
ofstream fout;
int vis[N];
vector<int> S[N];
int res = 0;
bool ban[N];
void dfs (int u, string cac, int cnt) {
    vis[u] = 1;
    string tmp = cac + "/" + str[u - 1];
    
    if ((int)adj[u].size()==0) {
        res++;
        S[u].push_back(u);
    }
    
    bool ok = 0;
    for (int &v : adj[u]) {
        if (vis[v]) continue;
        dfs (v, tmp, cnt + 1);
        ok |= ((int)adj[v].size()==0);
        for (int &x : S[v]) {
            S[u].push_back(x);
        }
    } 
    
    if (cnt == 3) {
        fout << tmp << " " << "has files: " << endl;
        for (int &x : S[u]) {
            if (!ban[x]) {
                //res++;
                fout << str[x - 1] << endl;
                ban[x] = 1;
            }
        }
        //fout << "con cac" << endl;
    }
    else if (cnt < 3 && ok) {
        fout << tmp << " " << "has files: " << endl;
        for (int &x : S[u]) {
            if (!ban[x]) {
                //res++;
                fout << str[x - 1] << endl;
                ban[x] = 1;
            }
        }
       // fout << "cac " << endl;
    }
}
int main() {
    freopen("/home/chau/official-rakuna-app/output.txt", "r", stdin);
    fout.open("/home/chau/files_need_translation_js.csv", ios::out | ios::app); 
    ios::sync_with_stdio(false);
    cin.tie(0);
    string x;
    
    while (getline(cin, x)) {
        doc.push_back (x);
    }
    string filePath = "";
    vector<int> dak;
    for (string &x : doc) {
        if (x[0] >= '0' && x[0] <= '9') {
            int num = 0;
            for (char &c : x) {
                num = num * 10 + (c - '0');
            }
            dak.push_back (num);
        }
        else {
            sort (dak.begin(), dak.end());
            dak.resize(unique (dak.begin(), dak.end()) - dak.begin());
            troll (filePath, dak);
            dak.clear();
            filePath = x;
        }
    }
    if (filePath!="") {
        troll (filePath, dak);
    }
    dfs (1, "", 1);
    cerr << res << " " << n << endl;
    return 0;
}
