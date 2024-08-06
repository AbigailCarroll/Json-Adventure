#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include "TextAdventure.h"
using namespace std;
using json = nlohmann::json;


int main(int argc, char* argv[])
{
    TextAdventure j("map2.json");
    string input, lower;
    j.listRoom();
    while (true)
    {
        //cin >> input;
        getline(cin, input);
        for (char i : input)
        {
            lower += (char)tolower(i);
        }
        j.parseInput(lower);
        lower = "";
    }
}

