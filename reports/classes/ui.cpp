#include "metro.h"

#include <iostream>

using namespace std;

namespace ui {

void clearScreen()
{
    cout << "\n";
}

void title(const string& text)
{
    string bar(text.size() + 8, '=');
    cout << "\n" << bar << "\n"
              << "    " + text << "\n"
              << bar << "\n";
}

void success(const string& text) { cout << "[OK] " + text << "\n"; }
void error(const string& text) { cout << "[!] " + text << "\n"; }
void warn(const string& text) { cout << "[*] " + text << "\n"; }
void info(const string& text) { cout << text << "\n"; }

string trim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool digitsToNumber(const string& text, long long& result)
{
    if (text.empty() || text.size() > 18)
        return false;
    size_t i = 0;
    bool negative = false;
    if (text[0] == '-') {
        negative = true;
        i = 1;
        if (text.size() == 1)
            return false;
    }
    long long value = 0;
    for (; i < text.size(); i++) {
        if (text[i] < '0' || text[i] > '9')
            return false;
        value = value * 10 + (text[i] - '0');
    }
    result = negative ? -value : value;
    return true;
}

bool parseInt(const string& text, int& out)
{
    long long value = 0;
    if (!digitsToNumber(trim(text), value) || value < -2147483647LL || value > 2147483647LL)
        return false;
    out = (int)value;
    return true;
}

string readLine(const string& prompt)
{
    cout << prompt;
    string line;
    if (!getline(cin, line))
        throw InputClosed();
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    return line;
}

int readInt(const string& prompt, int minValue, int maxValue)
{
    for (;;) {
        int value = 0;
        if (parseInt(readLine(prompt), value) && value >= minValue && value <= maxValue)
            return value;
        error("Please enter a whole number from " + to_string(minValue) + " to " +
              to_string(maxValue) + ".");
    }
}

string readText(const string& prompt, size_t maxLen)
{
    for (;;) {
        string s = trim(readLine(prompt));
        if (s.empty())
            error("This cannot be empty.");
        else if (s.size() > maxLen)
            error("Please use at most " + to_string(maxLen) + " characters.");
        else if (s.find('|') != string::npos)
            error("The '|' character is not allowed.");
        else
            return s;
    }
}

bool confirm(const string& question)
{
    for (;;) {
        string answer = trim(readLine(question + " (y/n): "));
        if (answer == "y" || answer == "Y")
            return true;
        if (answer == "n" || answer == "N")
            return false;
        error("Please type y or n.");
    }
}

Table::Table(Row headers) : headers_(headers) {}

void Table::addRow(Row row)
{
    row.resize(headers_.size());
    rows_.push_back(row);
}

static string tableCell(const string& text, size_t width)
{
    return " " + text + string(width - text.size(), ' ') + " |";
}

void Table::print() const
{
    Array<size_t> width(headers_.size());
    for (size_t i = 0; i < headers_.size(); i++)
        width[i] = headers_[i].size();
    for (const Row& row : rows_)
        for (size_t i = 0; i < row.size(); i++)
            if (row[i].size() > width[i])
                width[i] = row[i].size();

    string border = "+";
    for (size_t w : width)
        border += string(w + 2, '-') + "+";

    cout << border << "\n|";
    for (size_t i = 0; i < headers_.size(); i++)
        cout << tableCell(headers_[i], width[i]);
    cout << "\n" << border << "\n";
    for (const Row& row : rows_) {
        cout << "|";
        for (size_t i = 0; i < row.size(); i++)
            cout << tableCell(row[i], width[i]);
        cout << "\n";
    }
    cout << border << "\n";
}

}
