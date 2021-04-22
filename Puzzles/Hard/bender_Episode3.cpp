#include <iostream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <algorithm>
#include <cmath>

using namespace std;

double cte(const double n) { return 0; }
double logn(const double n) { return log(log(n)); }
double n(const double n) { return log(n); }
double nlogn(const double n) { return log(n * log(n)); }
double n2(const double n) { return 2 * log(n); }
double n2logn(const double n) { return log(n * n * log(n)); }
double n3(const double n) { return 3 * log(n); }
double power2n(const double n) { return n * log(2); }

double leastError(const vector<double>& X, const vector<double>& Y, double(*function)(double)) {
    double logc = log(Y[Y.size() - 1]) - function(X[X.size() - 1]);

    double res = 0;
    for (int i = 0; i < X.size(); i++) {
        res += pow(log(Y[i]) - logc - function(X[i]), 2);
    }
    return res;
}

int main()
{
    int N;
    cin >> N; cin.ignore();

    vector<double> X, Y;
    for (int i = 0; i < N; i++) {
        int num;
        int t;
        cin >> num >> t; cin.ignore();
        X.push_back(num);
        Y.push_back(t);
    }

    vector<pair<string,double>> errors = {
        {"O(1)", leastError(X, Y, &cte)},
        {"O(log n)", leastError(X, Y, &logn)},
        {"O(n)", leastError(X, Y, &n)},
        {"O(n log n)", leastError(X, Y, &nlogn)},
        {"O(n^2)", leastError(X, Y, &n2)},
        {"O(n^2 log n)", leastError(X, Y, &n2logn)},
        {"O(n^3)", leastError(X, Y, &n3)},
        {"O(2^n)", leastError(X, Y, &power2n)}
    };

    double mini = std::numeric_limits<double>::max();
    string res = "";

    for (const pair<string, double> err : errors) {
        cerr << err.first << " " << err.second << endl;
        if (err.second < mini) {
            mini = err.second;
            res = err.first;
        }
    }
    cerr << endl;

    cout << res << endl;
}