#include <string>
#include <vector>
using namespace std;
#include "json.hpp"
using json = nlohmann::json;

class TextAdventure {

public:

	TextAdventure(); //uses premade json file

	TextAdventure(string filePath); //takes filepath to jsonfile to load

	~TextAdventure(); //destructor

	bool parseInput(string input);

	void listRoom() const; //lists the room pointed to by cRoom_


private:

	int Rand(); //returns random number between 1 and 100 (both inclusive)

	string isVowel(char c);

	bool Kill(string input); //tries to kill the enemy given in input

	int findRoom(string roomID); //takes a room id(string) and returns the index it is stored at(int)

	string checkRoom(string object) const; //checks rooms for objects or enemies

	void GameOver(string message);

	bool Move(string input);

	void Win();

	bool checkObjectives(string objective);

	bool Save(); //saves the current state of the game in a json file that can be loaded like others

	bool Load(string filepath); //validates and loads json saved file

	bool checkDoors(string room); //takes input of room id trying to be moved to and checks if there is a locked door between the two rooms
	
	bool Open(string input);

	bool Look(string input);

	void listItems();

	bool Take(string input);

	json j_;
	int cRoom_;
	int prevRoom_;
	string map_;
	vector<string> inventory_;
	vector<string> objectives_;


};