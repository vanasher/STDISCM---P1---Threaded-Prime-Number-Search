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
#include <mutex>

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
    if (y < 1 || y > 1000000000000)
    {
        cout << "Error: y is missing or not a number between 1 and 1000000000000" << endl;
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
vector<uint64_t> all;
mutex listLock;
atomic<bool> go(false);
void searchNumbers(uint64_t y)
{   
    while (!go)
        this_thread::yield();

    while (true)
    {
        uint64_t n = nextNumber++;

        if (n > y)
            return;

        if (isPrime(n))
        {
            listLock.lock();
            all.push_back(n);
            listLock.unlock();
        }
    }
}

int main()
{   
    ios::sync_with_stdio(false);
    uint64_t x, y;

    if (!readConfig("../config.txt", x, y))
        return 1;

    
    nextNumber = 2;

    vector<thread> threads;
    for (uint64_t i = 0; i < x; i++)
        threads.push_back(thread(searchNumbers, y));
    string startTime = getTimeStamp();
    go = true;
    for (int i = 0; i < threads.size(); i++)
        threads[i].join();
    for (int i = 0; i < all.size(); i++)
        cout << "found prime: " << all[i] << "\n";
    string endTime = getTimeStamp();
    cout << "No. of threads: " << x << ", searched 1 to " << y << "\n";
    cout << "Start time: " << startTime << "\n";
    cout << "End time: " << endTime << "\n";
    return 0;
}
