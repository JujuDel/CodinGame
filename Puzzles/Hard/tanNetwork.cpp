#include <algorithm>
#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

typedef long long int lli;
typedef long double ld;

unordered_map<string, lli> idxMap;
vector<string> names;
vector<double> latitudes;
vector<double> longitudes;

lli V;

ld distance(const lli A, const lli B) {
  ld x =
      (longitudes[B] - longitudes[A]) * cos((latitudes[A] + latitudes[B]) / 2.);
  ld y = latitudes[B] - latitudes[A];
  return sqrt(x * x + y * y);
}

lli minDistance(const vector<pair<ld, lli>> &dist, const vector<bool> &sptSet) {
  ld min = numeric_limits<ld>::max();
  lli min_index;

  for (lli v = 0; v < V; v++)
    if (!sptSet[v] && dist[v].first <= min)
      min = dist[v].first, min_index = v;

  return min_index;
}

vector<pair<ld, lli>> dijkstra(const vector<vector<ld>> &graph, const lli src,
                               const lli end) {
  vector<pair<ld, lli>> dist;

  vector<bool> sptSet;

  for (lli i = 0; i < V; i++)
    dist.push_back({numeric_limits<ld>::max(), -1}), sptSet.push_back(false);

  dist[src].first = 0;

  lli u = -1;
  for (int count = 0; u != end && count < V - 1; ++count) {
    u = minDistance(dist, sptSet);
    sptSet[u] = true;

    for (lli v = 0; v < V; v++)
      if (!sptSet[v] && graph[u][v] > 0 &&
          dist[u].first != numeric_limits<ld>::max() &&
          dist[u].first + graph[u][v] < dist[v].first) {
        dist[v].first = dist[u].first + graph[u][v];
        dist[v].second = u;
      }
  }
  return dist;
}

int main() {

  vector<vector<ld>> network;

  string startPoint;
  cin >> startPoint;
  cin.ignore();
  string endPoint;
  cin >> endPoint;
  cin.ignore();

  cin >> V;
  cin.ignore();
  for (lli i = 0; i < V; i++) {
    string stopName;
    getline(cin, stopName);

    stringstream ss(stopName);

    vector<string> split;
    while (ss.good()) {
      string substr;
      getline(ss, substr, ',');
      split.push_back(substr);
    }
    idxMap[split[0]] = i;
    names.push_back(split[1].substr(1, split[1].size() - 2));
    latitudes.push_back(stod(split[3]) * M_PI / 180.);
    longitudes.push_back(stod(split[4]) * M_PI / 180.);

    network.push_back({});
    for (lli j = 0; j < V; j++) {
      network[i].push_back(0.);
    }
  }

  lli M;
  cin >> M;
  cin.ignore();
  for (lli m = 0; m < M; m++) {
    string A, B, route;
    getline(cin, route);
    istringstream ss(route);
    ss >> A >> B;

    lli i = idxMap[A];
    lli j = idxMap[B];

    network[i][j] = distance(i, j);
  }

  vector<pair<ld, lli>> dists = dijkstra(network, idxMap[startPoint], idxMap[endPoint]);

  vector<string> result;
  lli i = idxMap[endPoint];
  while (i != -1) {
    result.push_back(names[i]);
    i = dists[i].second;
  }

  if (result.size() < 2 && !(result.size() == 1 && startPoint == endPoint))
    cout << "IMPOSSIBLE" << endl;
  else
    for (lli i = result.size() - 1; i >= 0; i--) {
      cout << result[i] << endl;
    }
}