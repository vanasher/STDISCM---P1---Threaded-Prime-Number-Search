#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <cstdio>

using namespace std;

bool isPrime(int n)
{
    if (n < 2)
        return false;

    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}

// returns -1 if the text is not a valid number
int toNumber(string text)
{
    if (text.empty() || text.length() > 9)
        return -1;

    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
            return -1;
    }
    return stoi(text);
}

bool readConfig(string filename, int &x, int &y)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: cannot open " << filename << endl;
        return false;
    }

    x = -1;
    y = -1;

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

    if (x < 1 )
    {
        cout << "Error: x is missing or not a number greater than 1" << endl;
        return false;
    }

    if (y < 1 || y > 100000000)
    {
        cout << "Error: y is missing or not a number between 1 and 100000000" << endl;
        return false;
    }

    if (x > y)
    {
        cout << "Error: x cannot be more than y" << endl;
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

void searchRange(int start, int end)
{
    for (int n = start; n <= end; n++)
    {
        if (isPrime(n))
        {
            cout << "[" << getTimeStamp() << "] Thread " << this_thread::get_id()
                 << " found prime: " << n << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    int x, y;

    if (!readConfig("../config.txt", x, y))
        return 1;

    cout << "No of threads: " << x << ", searching 1 to " << y << endl;
    cout << "Start time: " << getTimeStamp() << endl;

    vector<thread> threads;
    int size = y / x;

    for (int i = 0; i < x; i++)
    {
        int start = i * size + 1;
        int end = (i + 1) * size;
        // if not divisible, the last thread gets the remaining
        if (i == x - 1)
            end = y;

        threads.push_back(thread(searchRange, start, end));
    }

    for (int i = 0; i < threads.size(); i++)
        threads[i].join();

    cout << "Done." << endl;
    return 0;
}
