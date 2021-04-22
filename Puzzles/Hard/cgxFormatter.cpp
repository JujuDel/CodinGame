#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// DISCLAIMER: Not optim at all, but it works
//   Good luck to understand it ;)

void clean(string &str) {
  str.erase(remove(str.begin(), str.end(), ' '), str.end());
  str.erase(remove(str.begin(), str.end(), '\t'), str.end());
  str.erase(remove(str.begin(), str.end(), '\n'), str.end());
}

void select(string &str) {
  size_t found = str.find('\'');
  if (found != string::npos) {
    size_t found2 = str.find('\'', found + 1);
    if (found2 == found + 1)
      str = "";
    else {
      str = str.substr(found, found2 - found + 1);
    }
  } else {
    clean(str);
  }
}

int main() {
  int N;
  cin >> N;
  cin.ignore();
  string CGX = "";
  for (int i = 0; i < N; i++) {
    string CGXLine;
    getline(cin, CGXLine);
    CGX += (CGX.size() > 0 ? "\n" : "") + CGXLine;
  }
  cerr << CGX << endl;

  int indent = 0;
  int i = 0;

  while (i < CGX.size()) {
    size_t foundOpen = CGX.find('(', i);
    size_t foundClose = CGX.find(')', i);
    size_t foundComma = CGX.find(';', i);
    size_t foundEqual = CGX.find('=', i);
    size_t foundQuote = CGX.find('\'', i);

    size_t mini = min(foundQuote, min(min(foundEqual, foundOpen),
                                      min(foundClose, foundComma)));

    bool begin = true;

    // Something present
    if (mini != string::npos) {
      // '\'' first
      if (mini == foundQuote) {
        size_t found2 = CGX.find('\'', mini + 1);
        cout << string(4 * indent, ' ') << CGX.substr(mini, found2 - mini + 1);
        i = found2 + 1;

        begin = false;

        foundOpen = CGX.find('(', i);
        foundClose = CGX.find(')', i);
        foundComma = CGX.find(';', i);
        foundEqual = CGX.find('=', i);
        mini = min(min(foundEqual, foundOpen), min(foundClose, foundComma));
        if (mini != foundEqual && mini != foundComma) {
          cout << endl;
          continue;
        }
      }

      if (mini == string::npos) {
        cout << endl;
      }
      // '(' first
      else if (mini == foundOpen) {
        string substr = CGX.substr(i, foundOpen - i);
        select(substr);
        if (substr.size() > 0)
          cout << string(4 * indent, ' ') + substr << endl;
        cout << string(4 * indent, ' ') + "(" << endl;
        indent++;
        i = foundOpen + 1;
      }
      // ')' first
      else if (mini == foundClose) {
        string substr = CGX.substr(i, foundClose - i);
        select(substr);
        if (substr.size() > 0)
          cout << string(4 * indent, ' ') << substr << endl;
        indent--;
        cout << string(4 * indent, ' ') + ")";
        i = foundClose + 1;
        if (i < CGX.size() && CGX[i] == ';') {
          cout << ";" << endl;
          i++;
        } else
          cout << endl;
      }
      // ';' first
      else if (mini == foundComma) {
        string substr = CGX.substr(i, foundComma - i);
        select(substr);
        cout << (begin ? string(4 * indent, ' ') : "") << substr << ";" << endl;
        i = foundComma + 1;
      }
      // '=' first
      else {
        string substr = CGX.substr(i, foundEqual - i);
        select(substr);
        cout << substr << "=";
        i = foundEqual + 1;

        size_t foundQuote = CGX.find('\'', i);
        size_t mini2 =
            min(min(foundQuote, foundComma), min(foundOpen, foundClose));
        if (mini2 != string::npos && mini2 == foundQuote) {
          size_t found2 = CGX.find('\'', foundQuote + 1);
          string substr = CGX.substr(i, found2 - i + 1);
          select(substr);
          cout << substr;
          i = found2 + 1;
          foundComma = CGX.find(';', i);
          mini2 = min(foundComma, min(CGX.find(')', i), CGX.find('(', i)));
        }

        if (mini2 != string::npos) {
          string substr = CGX.substr(i, mini2 - i);
          select(substr);
          cout << substr;
          if (mini2 == foundComma) {
            cout << ";";
            mini2++;
          }
          i = mini2;
          if (true || mini2 != foundClose) {
            cout << endl;
          }
        }
      }
    }
    // Nothing's there
    else {
      select(CGX);
      cout << CGX << endl;
      break;
    }
  }
}