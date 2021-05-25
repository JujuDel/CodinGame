#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

using namespace std;

constexpr double _eps{ 15. };
constexpr double _radius{ 600. };

template<typename T>
struct Coord {
    T x;
    T y;

    template<typename T1>
    bool operator==(const Coord<T1>& other) {
        return x == other.x && y == other.y;
    }
    template<typename T1>
    bool operator!=(const Coord<T1>& other) {
        return !(*this==other);
    }
    template<typename T1>
    void operator-=(const Coord<T1>& other) {
        x -= other.x;
        y -= other.y;
    }
};

struct hash_coord { 
    template<typename T>
	size_t operator()(const Coord<T>& c) const
	{
        auto h1 = hash<T>{}(c.x);
        auto h2 = hash<T>{}(c.y);
		return h1 ^ h2; 
	} 
};

struct equals_func{
    template<typename T1, typename T2>
    bool operator()( const Coord<T1>& lhs, const Coord<T2>& rhs ) const {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y);
    }
};

class Pod {
public:
    Pod(const string& f_name) : name{ f_name }, doBoost{ true }
    {}

    void readInput() {
        cin >> x >> y >> vx >> vy >> angle >> nextId; cin.ignore();
    }

    template<typename T>
    void computeThrust(const Coord<T>& checkPoint) {
        thrust = 100;

        cerr << name << endl;
        cerr << "Angle " << angle << endl;

        double quot{ 1000. };

        Coord<double> pos{ x / quot, y / quot };

        distance = pow(x - checkPoint.x, 2) + pow(y - checkPoint.y, 2);

        double xnew = cos(M_PI * angle / 180.) + pos.x;
        double ynew = sin(M_PI * angle / 180.) + pos.y;

        double xcheck = checkPoint.x / quot;
        double ycheck = checkPoint.y / quot;

        cerr << "x " << xnew << endl;
        cerr << "y " << ynew << endl;
        cerr << "x " << xcheck << endl;
        cerr << "y " << ycheck << endl;

        cerr << (xnew * xcheck + ynew * ycheck) << endl;
        cerr << (sqrt(pow(xnew, 2) + pow(ynew, 2)) * sqrt(pow(xcheck, 2) + pow(ycheck, 2))) << endl;
        cerr << (xnew * xcheck + ynew * ycheck) / (sqrt(pow(xnew, 2) + pow(ynew, 2)) * sqrt(pow(xcheck, 2) + pow(ycheck, 2))) << endl;

        angle = acos( (xnew * xcheck + ynew * ycheck) / (sqrt(pow(xnew, 2) + pow(ynew, 2)) * sqrt(pow(xcheck, 2) + pow(ycheck, 2))) );

        cerr << "Angle " << angle << endl;
        angle = 180. * abs(angle) / M_PI;

        cerr << "Angle " << angle << endl;
        cerr << "Distance " << distance << endl;

        if (angle > _eps) {
            thrust *= max(0., 1 - angle / 90.) * min(1., distance / (2 * _radius));
        }
    }

    void command(const Coord<int>& checkPoint){
        // There were 6 lines of code here ;)
    }

    int x;
    int y;
    int vx;
    int vy;
    int angle;
    int thrust;
    int distance;
    int nextId;

    string name;

    bool doBoost;
};

int main()
{
    std::cerr << "You have enoug info to make your own BOT now \U0001F609" << std::endl;
    std::cerr << "http://www.someecards.com/usercards/nsviewcard/MjAxMi04ZWYxNjNmNTI5ODZiMjk0/" << std::endl;
}