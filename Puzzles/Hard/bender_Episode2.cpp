#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


void printRooms(const std::unordered_map<int, std::unordered_set<int>> &map_room,
                const std::unordered_map<int, int> &map_money)
{
    for (const auto& elem : map_money) {
        std::cerr << "Room: " << elem.first << std::endl;
        std::cerr << "   Money: " << elem.second << std::endl;
        std::cerr << "   Nexts:";
        for (const auto& r : map_room.at(elem.first)) {
            std::cerr <<  " " << r;
        }
        std::cerr << std::endl;
    }
}

long long int memoization(const std::unordered_map<int, std::unordered_set<int>> &map_room,
                          const std::unordered_map<int, int> &map_money,
                          const int k, const int N, long long int mem[])
{
    if (k >= N)
        return 0;
    
    if (mem[k] != -1)
        return mem[k];
    
    long long int money = std::numeric_limits<long long int>::min();
    for (const int& r : map_room.at(k)) {
        if (r == -1) {
            money = std::max(money, static_cast<long long int>(0));
        }
        else {
            money = std::max(money, memoization(map_room, map_money, r, N, mem));
        }
    }

    money += map_money.at(k);
    mem[k] = money;
    return money;
}


long long int solve(const std::unordered_map<int, std::unordered_set<int>> &map_room,
                    const std::unordered_map<int, int> &map_money,
                    const int N)
{
    long long int mem[N];
    for (int i = 0; i < N; ++i) {
        mem[i] = -1;
    }

    return memoization(map_room, map_money, 0, N, mem);
}


int main()
{
    std::unordered_map<int, std::unordered_set<int>> map_room;
    std::unordered_map<int, int> map_money;

    int N;
    std::cin >> N; std::cin.ignore();

    std::string delimiter = " ";
    for (int i = 0; i < N; i++) {
        std::string room;
        getline(std::cin, room);
        
        size_t pos = room.find(delimiter);
        int roomID = std::stoi(room.substr(0, pos));
        room.erase(0, pos + delimiter.length());

        pos = room.find(delimiter);
        int money = std::stoi(room.substr(0, pos));
        room.erase(0, pos + delimiter.length());

        pos = room.find(delimiter);
        std::string nextRoomA = room.substr(0, pos);
        int nextRoomAInt = nextRoomA == "E" ? -1 : std::stoi(nextRoomA);
        room.erase(0, pos + delimiter.length());

        pos = room.find(delimiter);
        std::string nextRoomB = room.substr(0, pos);
        int nextRoomBInt = nextRoomB == "E" ? -1 : std::stoi(nextRoomB);

        if (map_money.find(roomID) == map_money.end()) {
            map_money[roomID] = money;
        }
        
        if (map_room.find(roomID) == map_room.end()) {
            if (nextRoomA != nextRoomB) {
                map_room[roomID] = {nextRoomAInt, nextRoomBInt};
            }
            else {
                map_room[roomID] = {nextRoomAInt};
            }
        }
        else {
            for (const int& r : {nextRoomAInt, nextRoomBInt}) {
                if (map_room[roomID].find(r) == map_room[roomID].end()) {
                    map_room[roomID].insert(r);
                }
            }
        }
    }

    //printRooms(map_room, map_money);

    std::cout << solve(map_room, map_money, N) << std::endl;

    return 0;
}