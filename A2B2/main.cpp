#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstdint>
#include <atomic>

using namespace std;

bool isPrime(uint64_t n)
{
    if (n < 2)
        return false;

    for (uint64_t i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}

uint64_t toNumber(string text)
{
    if (text.empty() || text.length() > 19)
        return 0;

    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
            return 0;
    }
    return stoull(text);
}

bool readConfig(string filename, uint64_t &x, uint64_t &y)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: cannot open " << filename << endl;
        return false;
    }

    x = 0;
    y = 0;

    string line;
    int lineNo = 0;
    while (getline(file, line))
    {
        lineNo++;

        string clean = "";
        for (int i = 0; i < line.length(); i++)
        {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r')
                clean += line[i];
        }

        if (clean.empty())
            continue;

        int eq = clean.find('=');
        if (eq == string::npos)
        {
            cout << "Error: line " << lineNo << " has no '='" << endl;
            return false;
        }

        string key = clean.substr(0, eq);
        string value = clean.substr(eq + 1);

        if (key == "x")
            x = toNumber(value);
        else if (key == "y")
            y = toNumber(value);
        else
        {
            cout << "Error: unknown setting '" << key << "' on line " << lineNo << endl;
            return false;
        }
    }
    
    file.close();
    if (x < 1 || x > 1000)
    {
        cout << "Error: x is missing or not a number between 1 and 1000" << endl;
        return false;
    }
    if (y < 2 || y > 1000000000000)
    {
        cout << "Error: y is missing or not a number between 2 and 1000000000000" << endl;
        return false;
    }

    return true;
}

string getTimeStamp()
{
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    int ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    char timeText[16];
    strftime(timeText, sizeof(timeText), "%H:%M:%S", localtime(&t));
    char result[32];
    snprintf(result, sizeof(result), "%s.%03d", timeText, ms);
    return string(result);
}

atomic<uint64_t> nextNumber;
vector< vector<uint64_t> > results;
void searchNumbers(uint64_t y, uint64_t slot)
{
    while (true)
    {
        uint64_t n = nextNumber++;

        if (n > y)
            return;

        if (isPrime(n))
            results[slot].push_back(n);
    }
}

int main()
{
    uint64_t x, y;

    if (!readConfig("../config.txt", x, y))
        return 1;

    cout << "No. of threads: " << x << ", searching 1 to " << y << endl;
    cout << "Start time: " << getTimeStamp() << endl;
    nextNumber = 2;
    results.resize(x);

    vector<thread> threads;
    for (uint64_t i = 0; i < x; i++)
        threads.push_back(thread(searchNumbers, y, i));
    for (int i = 0; i < threads.size(); i++)
        threads[i].join();
    vector<uint64_t> all;
    for (uint64_t i = 0; i < x; i++)
    {
        for (int j = 0; j < results[i].size(); j++)
            all.push_back(results[i][j]);
    }
    sort(all.begin(), all.end());

    for (int i = 0; i < all.size(); i++)
        cout << "found prime: " << all[i] << endl;

    cout << "End time: " << getTimeStamp() << endl;
    return 0;
}
