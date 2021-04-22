#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int L;
    int C;
    int N;
    cin >> L >> C >> N; cin.ignore();

    queue<int> waiting;
    vector<int> numbers;
    for (int i = 0; i < N; i++) {
        int Pi;
        cin >> Pi; cin.ignore();
        numbers.push_back(Pi);
        waiting.push(i);
    }

    unordered_map<int, pair<long long int, int>> umap;

    long long int res = 0;
    int c = 0;
    while (c < C) {
        if (umap.find(waiting.front()) == umap.end())
        {
            umap[waiting.front()] = { res, c };

            long long int count = 0;
            int n = 0;
            while (count + numbers[waiting.front()] <= L && n < N) {
                waiting.push(waiting.front());
                count += numbers[waiting.front()];
                waiting.pop();
                n++;
            }
            res += count;
            c++;
        }
        else {
            pair<long long int, int> p = umap[waiting.front()];
            cerr << "Found it" << endl;
            cerr << p.first << "  " << p.second << endl;
            cerr << static_cast<long long int>((C - c) / (c - p.second)) << endl;
            res += (res - p.first) * static_cast<long long int>((C - c) / (c - p.second));
            c = C - (C - c) % (c - p.second);
            break;
        }
    }
    
    while (c < C) {
        long long int count = 0;
        int n = 0;
        while (count + numbers[waiting.front()] <= L && n < N) {
            waiting.push(waiting.front());
            count += numbers[waiting.front()];
            waiting.pop();
            n++;
        }
        res += count;
        c++;
    }

    cout << res << endl;
}
