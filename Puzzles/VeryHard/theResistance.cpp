#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>

using namespace std;

vector<string> morse_dictionnary{
    ".-", "-...", "-.-.", "-..", ".",
    "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---",
    ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--",
    "--.."
};

struct Node {
    int nbFinish;
    Node* left;
    Node* right;

    Node() :
        nbFinish{ 0 },
        left{ nullptr },
        right{ nullptr }
    {}
};

class Trie {
public:
    Trie() {}

    void addWord(const string& word) {
        Node* node = &root;
        for (const char& c : word) {
            if (c == '.') {
                if (!node->left) {
                    node->left = new Node();
                }
                node = node->left;
            }
            else {
                if (!node->right) {
                    node->right = new Node();
                }
                node = node->right;
            }
        }
        node->nbFinish++;
    }

    const Node* getRoot() const {
        return &root;
    }

private:
    Node root;
};

long long int memoization(const string& message, const Node* const root, const size_t idx, long long int *mem) {
    if (idx == message.size()) {
        return 1;
    }
    if (idx > message.size()) {
        return 0;
    }

    if (mem[idx] != -1) {
        return mem[idx];
    }

    const Node* node = root;
    long long int res = 0;
    size_t local_idx = idx;
    while (node) {
        if (node->nbFinish > 0) {
            res += node->nbFinish * memoization(message, root, local_idx, mem);
        }

        if (message[local_idx] == '.') {
            node = node->left;
        }
        else {
            node = node->right;
        }
        local_idx++;
    }
    mem[idx] = res;
    return res;
}

int main()
{
    string L;
    cin >> L; cin.ignore();

    int N;
    cin >> N; cin.ignore();

    Trie dictionnary;
    for (int i = 0; i < N; i++) {
        string W;
        cin >> W; cin.ignore();

        string word = "";
        for (const char& c : W) {
            word += morse_dictionnary[int(c)-int('A')];
        }
        dictionnary.addWord(word);
    }

    long long int mem[L.size()];
    for (int i = 0; i < L.size(); ++i) {
        mem[i] = -1;
    }

    cout << memoization(L, dictionnary.getRoot(), 0, mem) << endl;
}