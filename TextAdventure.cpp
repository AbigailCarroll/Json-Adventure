#include "TextAdventure.h"
#include "json.hpp"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
using json = nlohmann::json;



TextAdventure::TextAdventure() {
    map_ = "map1.json";
    ifstream fin("map1.json");
    fin >> j_; //load file into json object
    cRoom_ = findRoom(j_["player"]["initialroom"]);
    for (auto i : j_["objective"]["what"])
    {
        objectives_.push_back(i);
    }
}

TextAdventure::TextAdventure(string filepath)
{
    map_ = filepath;
    ifstream fin(map_);
    fin >> j_; //load file into json object
    cRoom_ = findRoom(j_["player"]["initialroom"]);
    for (auto i : j_["objective"]["what"])
    {
        objectives_.push_back(i);
    }
}

TextAdventure::~TextAdventure()
{
}

int TextAdventure::findRoom(string roomID)
{
    int i = 0;
    while (j_["rooms"][i]["id"] != roomID)
    {
        i++;
    }
    return i;
}

void TextAdventure::listRoom() const
{
    string desc = j_["rooms"][cRoom_]["desc"];
    cout << "> " << desc << endl;
    cout << checkRoom("enemies");
    cout << checkRoom("objects");
    
}

string TextAdventure::checkRoom(string object) const
{
    
    string output = "";
    for (auto i : j_[object])
    {
        if (i["initialroom"] == j_["rooms"][cRoom_]["id"])
        {
            output += "> ";
            output += "There is a ";
            output += i["id"];
            output += " in the room with you. ";
            output += i["desc"];
            output += "\n";
        }
    }
    
    return output;
}

string TextAdventure::isVowel(char c) //used for outputting the inventory, not perfect in situations such as Unicorn, Hourglass etc,
{//but a perfect solution would require an unnecessary amount of work.
    char vowels[5] = { 'a', 'e', 'i', 'o', 'u'};
    for (size_t i = 0; i < 5; i++)
    {
        if (c == vowels[i])
        {
            return "an ";
        }
    }
    return "a ";
}

bool TextAdventure::Kill(string enemy) 
{
    if (enemy.length() < 6)
    {
        cout << "> " << "What do you want to kill?" << endl;
        return false;
    }
    for (size_t i = 0; i < j_["enemies"].size(); i++)
    {
        if (enemy.find(j_["enemies"][i]["id"]) != string::npos && j_["enemies"][i]["initialroom"] == j_["rooms"][cRoom_]["id"]) //make sure enemy name and room are right
        {
            for (auto k : j_["enemies"][i]["killedby"]) //for each item that kills the enemy
            {
                bool found = false;
                for (size_t j = 0; j < inventory_.size(); j++) //search the inventory
                {
                    if (inventory_[j] == k)
                    {
                        found = true; //once it is found move on the the next necessary item
                    }
                }
                if (!found) //fail to find the right item
                {
                    string message = "The ";
                    message += j_["enemies"][i]["id"];
                    message += " killed you!";
                    GameOver(message);
                }
            }
            string enemy = j_["enemies"][i]["id"];
            
            cout << "> " << "You kill the " << enemy << endl;
            if (j_["objective"]["type"] == "kill")
            {
                checkObjectives(enemy);
            }
            j_["enemies"][i]["initialroom"] = j_["enemies"][i]["initialroom"].max_size();
            return true;
        }
        
    }
    cout << "> " << "There is no " << enemy << " in the room" << endl;
    return false;
}

bool TextAdventure::Move(string input)
{
    for (auto i : j_["rooms"][cRoom_]["exits"].items()) //iterates through all the exits of the current room
    {
        string s = i.key();
        if (input.find(s) != string::npos) //checks if the input contains the key of one of the exits
        {
            for(auto k : j_["enemies"]) //see if an enemy wants to kill the player
            {
                if (k["initialroom"] == j_["rooms"][cRoom_]["id"])
                {
                    if (Rand() < k["aggressiveness"])
                    {
                        string message = "The ";
                        message += k["id"];
                        message += " killed you as you tried to escape the room.";
                        GameOver(message);
                    }
                }
                if (checkDoors(j_["rooms"][cRoom_]["exits"][s]))
                {
                    return false;
                }
                prevRoom_ = cRoom_;
                cRoom_ = findRoom(j_["rooms"][cRoom_]["exits"][s]);
                string room = j_["rooms"][cRoom_]["id"];
                if (j_["objective"]["type"] == "room")
                {
                    checkObjectives(room);
                }
                listRoom();
                return true;
            }
            
        }
    }
    if (input.find("back") != string::npos)
    {
        string prev = j_["rooms"][prevRoom_]["id"];
        for (auto i : j_["rooms"][cRoom_]["exits"])
        {
            if (i == prev) //if the current room has an exit that leads to the previous room
            {
                int store = prevRoom_;
                prevRoom_ = cRoom_;
                cRoom_ = store;
                listRoom();
                return true;
            }
        }
    }
    cout << "> " << "You cannot go that way" << endl;
    return false;
}

bool TextAdventure::checkDoors(string room) //return true for locked door
{
    for (int i = 0; i < j_["doors"].size(); i++)
    {
        if (room == j_["doors"][i]["rooms"][0] || j_["rooms"][cRoom_]["id"] == j_["doors"][i]["rooms"][0]) //could be written out as one long logical statememnt but i prefer to keep it shorter
        {
            if (room == j_["doors"][i]["rooms"][1] || j_["rooms"][cRoom_]["id"] == j_["doors"][i]["rooms"][1])
            {
                cout << "> The " << j_["doors"][i]["id"] << " is locked, blocking your path" << endl;
                return true;
            }
        }
    }
    return false;
}

bool TextAdventure::checkObjectives(string objective)
{
    for (size_t i = 0; i < objectives_.size(); i++)
    {
        if (objective == objectives_[i])
        {
            objectives_.erase(objectives_.begin() + i);
            break;
        }
    }
    if (objectives_.size() <= 0)
    {
        Win();
    }
    
    return false;
}

int TextAdventure::Rand()
{
    srand(time(nullptr));
    return 1 + (rand() % 100); 
}

bool TextAdventure::Save()
{
    ofstream saveFile("savefile.json");
    json save;
    save["map"] = map_;
    save["inventory"] = inventory_;
    save["playerroom"] = cRoom_;
    vector<string> enemiesKilled;
    for (auto i : j_["enemies"])
    {
        if (i["initialroom"] == i["initialroom"].max_size())
        {
            enemiesKilled.push_back(i["id"]);
        }
    }
    save["enemieskilled"] = enemiesKilled;

    saveFile << save;
    cout << "> Progress has been saved as \"savefile.json\"" << endl;
    cout << "> To load the save file type: \"load savefile.json\"" << endl;
    
    return true;
}

bool TextAdventure::Load(string filepath)
{
    json save;
    ifstream fin(filepath);
    fin >> save;
    if (save["map"] != map_)
    {
        cout << "savefile map does not match currently loaded mapfile" << endl;
        return false;
    }
    if (save["playerroom"] > j_["rooms"].size()-1)
    {
        cout << "Invalid room" << endl;
        return false;
    }
    for (auto i : save["enemieskilled"])
    {
        for (size_t k = 0; k < j_["enemies"].size(); k++)
        {
            if (i == j_["enemies"][k]["id"])
            {
                j_["enemies"][k]["initialroom"] = j_["enemies"][k]["initialroom"].max_size();
            }
        }
    }
    inventory_ = save["inventory"];
    for (auto i : save["inventory"])
    {
        for (size_t k = 0; k < j_["objects"].size(); k++)
        {
            if (i == j_["objects"][k]["id"])
            {
                j_["objects"][k].clear();
            }
        }
    }
    cRoom_ = save["playerroom"];
    cout << "> Save file loaded Successfully!" << endl << endl;
    listRoom();
    return true;
}

bool TextAdventure::Open(string input)
{
    for (int i = 0; i < j_["doors"].size(); i++)
    {
        if (input.find(j_["doors"][i]["id"]) != string::npos)
        {
            if (j_["rooms"][cRoom_]["id"] == j_["doors"][i]["rooms"][0] || j_["rooms"][cRoom_]["id"] == j_["doors"][i]["rooms"][1])
            {
                for (size_t j = 0; j < inventory_.size(); j++)
                {
                    if (inventory_[j] == j_["doors"][i]["key"])
                    {
                        cout << "> You turn the " << j_["doors"][i]["key"] << " in the keyhole, and the door opens." << endl;
                        j_["doors"][i].clear();
                        return true;
                    }
                }
                cout << "You try everything, but the door remains locked." << endl;
                return false;
            }
            cout << "You look around, but there is no " << input << "in this room." << endl;
            return false;
        }
        cout << "There is no such door." << endl;
        return false;
    }
    return false;
}

bool TextAdventure::Look(string input)
{
    if (input.find("around") != string::npos || input.length() == 4)
    {
        listRoom();
        return true;
    }
    else
    {
        string output = "";
        if (input.find("objects") != string::npos)
        {
            output = checkRoom("objects");
            if (output == "")
            {
                output += "> There don't seem to be any objects in the room with you \n";
            }
        }
        if (input.find("enemies") != string::npos)
        {
            string s = checkRoom("enemies");
            output += s;
            if (s == "")
            {
                output += "> There don't seem to be any enemies in the room with you \n";
            }
        }
        if (output != "")
        {
            cout << output;
            return true;
        }
        cout << "> " << "I don't know what that means" << endl;
        return false;
    }
}

void TextAdventure::listItems()
{
    cout << "> " << "You look inside your pack and see: " << endl;
    for (size_t i = 0; i < inventory_.size(); i++)
    {
        char s = inventory_[i].at(0);
        cout << "> " << isVowel(s) << inventory_[i] << endl;
    }
}

bool TextAdventure::Take(string input)
{
    for (size_t i = 0; i < j_["objects"].size(); i++) //easier to use this loop as need to clear entry
    {
        if (!j_["objects"][i].empty())
        {
            if (input.find(j_["objects"][i]["id"]) != string::npos && findRoom(j_["objects"][i]["initialroom"]) == cRoom_)
            {
                string s = j_["objects"][i]["id"];
                cout << "> " << "You take the " << s << endl;
                if (j_["objective"]["type"] == "collect")
                {
                    checkObjectives(s);
                }
                inventory_.push_back(s);
                j_["objects"][i].clear();
                return true;
            }
        }

    }


    cout << "> " << "There is no" << input.substr(4) << " in the room" << endl;
    return false;
}

bool TextAdventure::parseInput(string input)
{
    if (input.substr(0,3) == "go ") 
    { 
        return Move(input);
    }
    else if (input.substr(0, 5) == "take ") 
    {
        return Take(input);
    }
    else if (input == "list items")
    {
        listItems();
        return true;
    }
    else if (input.substr(0, 4) == "look")
    {
        return Look(input);
    }
    else if (input.substr(0, 4) == "kill")
    {
        
        Kill(input);
        return true;
    }
    else if (input == "save")
    {
        Save();
        return true;
    }
    else if (input.substr(0,5) == "load ")
    {
        Load(input.substr(5));
        return true;
    }
    else if (input.substr(0, 5) == "open ")
    {
        return Open(input.substr(5));
    }
    cout << "> " << "I do not recognise that command" << endl;
    return false;
}

void TextAdventure::Win()
{
    

    cout << "__   __           __        ___       _ " << endl;
    cout << "\\ \\ / /__  _   _  \\ \\      / (_)_ __ | |" << endl;
    cout << " \\ V / _ \\| | | |  \\ \\ /\\ / /| | '_ \\| |" << endl;
    cout << "  | | (_) | |_| |   \\ V  V / | | | | |_|" << endl;
    cout << "  |_|\\___/ \\__,_|    \\_/\\_/  |_|_| |_(_)" << endl;
    exit(0);
}

void TextAdventure::GameOver(string message)
{
    cout << "  ____                         ___                 " << endl;
    cout << " / ___| __ _ _ __ ___   ___   / _ \\__   _____ _ __ " << endl;
    cout << "| |  _ / _` | '_ ` _ \\ / _ \\ | | | \\ \\ / / _ \\ '__|" << endl;
    cout << "| |_| | (_| | | | | | |  __/ | |_| |\\ V /  __/ |   " << endl;
    cout << " \\____|\\__,_|_| |_| |_|\\___|  \\___/  \\_/ \\___|_|   " << endl;
    cout << "> " << message << endl;
    exit(0);
}


//ADDITIONAL FEATURES

//players can save their progress which will be output to another json file - DONE
//doors can be locked and need a key to open - DONE
//limited natural language processing - DONE
// "go back" to return to previous room (assuming previous room is still accesible) - DONE


//doors

// Doors can be locked from both or one side, when a door is unlocked it needs to be unlocked for both sides
// save locked doors along with pair of rooms it connects, name of key that opens it.
// when player wants to move check whether the rooms are connected by a locked door.

