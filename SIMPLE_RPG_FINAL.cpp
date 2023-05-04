/*David Orriss, Brandon Viorato, and Jacob Clifton
C++ I Final Project; SIMPLE RPG
11:00 - 11:50 AM
Last edited on 6/7/2019*/

#include <iostream>//basic console output, such as printing the map.
#include <iomanip> //some visual formatting during battles.
#include <string> //monster and equipment names.
#include <fstream> //saving and loading player character
#include <ctime> //allows for random placement of player and monsters on the grid.
using namespace std;

void genMap(int&, int&), showMap();//basic map generation required to start the game, showMap prints the map on the console.
void gameplayLoop(int&, int&, int&);//handles the basic cycle of gameplay; in the finished product the battle function should be contained in here.
int BATTLE(int, int, int, int, int); //This function handles the turn-based battles. Player stats are put in, the function calls getMonster to generate a monster.
void getMonster(int, string&, char&, int&, int&, int&); //Generates a single monster based on the player's level at time of input, including its name, symbol, and stats.
void updatePlayerStats(int&, int&, int&, int&, int&, int&, bool); //goes Health, Intelligence, Strength, Defense, Level, Experience, and whether a battle had just been won.
char worldMap[22][22];//This is a global array accessed by three functions that stores the map.
					  //I tried to get this to work with a local array several times, but each time ended poorly. It was ultimately easier and more efficient to just use a global array.

int main()
{	
	int playerX, playerY, gameState = 1; //gamestate 1 means the game is running normally, 0 is a loss, 2 is a win.
	srand(time(NULL));//seeds random number generator with computer time to gurantee random placement; won't be needed in the final version
	genMap(playerX,playerY); //generates the initial map and gets where the player is.
	showMap(); //actually prints said map, with monsters and player location.
	while (gameState == 1)
	{
		gameplayLoop(playerX, playerY, gameState);
		if (gameState == 0)
		{
			system("cls");
			cout << "\n\n\n---==+GAME OVER+==---\n\n\n";
			break;
		}
		else if (gameState == 2)
		{
			system("cls");
			cout << "\n\n\n---==+ YOU WON +==---\n\n\n";
			break;
		}
	}
	system("pause");
	return(0);
}

void genMap(int& playerX, int& playerY)//playerX is the column the player is located in, playerY is the row.
{
	//generate the first line of the map; just mountains ('^')
	//this adds borders to the map the player can't move over later.
	const int ROWS = 22, COLUMNS = 22;
	int col; // creates the col variable for use later.
	for (int curRow = 0; curRow < 22; curRow++)
	{
		if (curRow == 0 || curRow == 21) //checks if we're on the edge of the map.
		{
			for (col = 0; col < 22; col++) //if so, set everything in that row to a space.
				worldMap[curRow][col] = '^';
		}
		else
		{
			for (col = 0; col < 22; col++)
			{
				if (col == 0 || col == 21)
					worldMap[curRow][col] = '^';//mountain, cannot be crossed
				else
					worldMap[curRow][col] = '/';//ground, kinda looks like tall grass right?
			}
		}
	}
	int placeX = 1, placeY = 1; //determines where special characters are placed on the map. X is Rows, Y is Columns
	for (int placements = 0; placements < 19; placements++)//runs 18 times, not counting unsuccessful placement.
	{
		char readCharacter;//basic variable for reading what's on the randomly pulled spot
		placeX = 1 + rand() % 20;//pull a random row from 1 to 20.
		placeY = 1 + rand() % 20;//same as above for columns.
		readCharacter = worldMap[placeX][placeY];
		if (readCharacter == '^' || readCharacter == 'S') //Checks to make sure the spot isn't a mountain or a monster is already there.
			placements--; //if the space is occupied or a border, decrement placements and try again, as the placement wasn't successful.
		else
			worldMap[placeX][placeY] = 'S';//if not, place an encounter over that space.
	}
	bool playerPlaced = false; //checks to make sure the player has been placed successfully.
	while (!playerPlaced) //runs until the player is placed successfully on the map
	{
		char readCharacter;//basic variable for reading what's on the randomly pulled spot
		placeX = 1 + rand() % 20;//pull a random row from 1 to 20.
		placeY = 1 + rand() % 20;
		readCharacter = worldMap[placeX][placeY];
		if (readCharacter != '^' && readCharacter != 'S') //checks for if something isn't already placed there
		{
			playerX = placeX; playerY = placeY; //store player's location for use in movement later on
			worldMap[placeX][placeY] = '@'; //if not, place the player and finish map generation
			playerPlaced = true; //once the player has successfully been placed on the map, map generation ends.
		}
			
	}
}
void showMap()
{ //this is a very basic function that prints out the map and should be reused whenever the map needs to be redrawn
	//such as the case in the player moving or leaving a battle.
	for (int curRow = 0; curRow < 22; curRow++)
	{
		for (int curCol = 0; curCol < 22; curCol++)
		{
			cout << worldMap[curRow][curCol];
		}
		cout << endl;
	}
}
void gameplayLoop(int& playerX, int& playerY, int& gameState)
{
	//this function is the basic gameplay loop. it should only be interrupted by battles, leveling, and other gameplay events.
	int xCheck, yCheck, HP, INT, STR, DEF, LVL, EXP, maxHP; //establishes basic values for player.
	/*Player Stats (for Reference)
	xCheck - the column that is checked for vacancy or not. When the loop completes, this will become the player's column location.
	yCheck - the same idea as above, but for rows instead of columns.
	HP - The player's current amount of health. It's initialized at its maximum (at first 200) and goes down whenever
	the player is attacked.
	INT - Intelligence, determines the amount of damage done by Magic. Ignores the monster's Defense.
	STR - Strength, determines the damage done by an Attack. The monster's defense reduces it.
	DEF - Defense, decreases the amount of damage the player takes,
	LVL - Level, determines what monsters the player encounters.
	EXP - Experience, keeps track of how many monsters the player has defeated that level.
	*/
	updatePlayerStats(HP, INT, STR, DEF, LVL, EXP, false); //reminder; the boolean value at the end checks if a battle was won.
	maxHP = HP; //A special variable to determine how much the player heals between fights, detailed later.
	char playerInput = 'z'; //invalid input to ensure input by user is required.
	gameState = (EXP == 3 && LVL == 4) ? 2 : 1;// checks if the player has hit max level and EXP from the amount of monsters spawned
											   // if so, the player is a winner.
	while (playerInput != 'N' && playerInput != 'E' && playerInput != 'S' && playerInput != 'W')
	{
		cout << "Travel North, East, South, or West?: ";
		cin >> playerInput;
	}
	xCheck = playerX; yCheck = playerY; //variables for checking if the slot the player moves into is occupied or not
	if (playerInput == 'N')//checking direction player wants to move in; the slot above.
		xCheck = playerX-1;
	else if (playerInput == 'S')//the slot below
		xCheck = playerX+1;
	else if (playerInput == 'E')//the slot to the right
		yCheck = playerY+1;
	else if (playerInput == 'W')//the slot to the left
		yCheck = playerY-1;
	//DO NOT TOUCH THE NAVIGATION ABOVE! I somehow got the directions swapped during writing and it's weird, but just go with it.
	if (worldMap[xCheck][yCheck] == '^') //checks to make sure a mountain isn't in the way
	{
		cout << "A mountain blocks your path; moving through is impossible.\n";
		system("pause");
		gameState = 1;
	}
	else if (worldMap[xCheck][yCheck] == 'S') //this triggers a battle
	{
		int battleWon = BATTLE(HP, INT, STR, DEF, LVL);
		if (battleWon == 1) //very basic script to heal the player for 60% of their health + 10 HP
		{
			++EXP; //Increments EXP by one before it's put into the function.
			int toHeal = HP * 0.6 + 10;
			HP = ((HP + toHeal) > maxHP) ? maxHP: HP += toHeal; //checks to see if the player would be healed beyond max health; if so, just fully heal them.
			updatePlayerStats(HP, INT, STR, DEF, LVL, EXP, true); //go into the function with a battle won
		}
		else
			gameState = 0;//ends the game on a loss, as the player is dead.
		worldMap[playerX][playerY] = '/'; //replaces the position the player was previously in with an empty tile
		worldMap[xCheck][yCheck] = '@'; //replaces the next tile with the player's location
		playerX = xCheck; playerY = yCheck;//update the player's location on the map.
		system("pause");
	}
	else
	{
		worldMap[playerX][playerY] = '/'; //replaces the position the player was previously in with an empty tile
		worldMap[xCheck][yCheck] = '@'; //replaces the next tile with the player's location
		playerX = xCheck; playerY = yCheck;//update the player's location on the map.
	}
	system("cls");
	showMap();
}
void updatePlayerStats(int& HP, int& INT, int& STR, int& DEF, int& LVL, int& EXP, bool battleWon)
{
	//this function writes the player's stats to a file and reads them from that file. All presented variables have appeared before.
	//the battleWon boolean simply is used to state whether the player had just won a battle or not before entering this function.
	ifstream inputFile; //creates an input file stream object
	ofstream outputFile; //and an output file stream object
	static bool hasRunOnce = false; //checks if the loop has run once before
	int pStats[6]; //creates a temporary array for storing and managing player stats.
	if (hasRunOnce == false)
	{
		HP = 200; INT = 5; STR = 15; DEF = 5; LVL = 0; EXP = 0; //if the loop hasn't run before in the program, assign default values to all the player's stats
		pStats[0] = HP; pStats[1] = INT; pStats[2] = STR; pStats[3] = DEF; pStats[4] = LVL; pStats[5] = EXP;//get the values of the variables assigned to appropriate spots in the array
		outputFile.open("pstats.txt"); //pStats.txt is what stores the stats of the character
		for (int i = 0; i <= 5; i++) //writes the character's stats to the file for the first run of the function
			outputFile << pStats[i] << endl;
		outputFile.close();
		hasRunOnce = true; //signifies the loop has run once
	}
	else
	{
		inputFile.open("pstats.txt");
		for (int i = 0; i <= 5; i++) //read the stats from the file and assign them into pStats
			inputFile >> pStats[i];
		inputFile.close();
		HP = pStats[0]; INT = pStats[1]; STR = pStats[2]; DEF = pStats[3]; LVL = pStats[4]; EXP = pStats[5];
		if (pStats[5] == 4)//checks to make sure that the player's experience is equal to 4. if so, the player levels up.
		{
			outputFile.open("pstats.txt"); //opens the output file before increasing all of the player's stats.
			LVL += 1;
			cout << "You Leveled Up to Level " << LVL << endl;
			HP += (25 + rand()%25); //player gains anywhere between 25 and 50 more health
			INT += (1 + rand()% 5); //player gains anywehre between 1 and 6 Intelligence
			STR += (5 + rand() % 5); //player gains anywhere between 5 to 10 more Strength
			DEF += (1 + rand() % 3); //player gains anywhere between 1 and 3 Defense
			EXP = 0;
			cout << "HP is now " << HP << ", INT is now " << INT << ", STR is now " << STR << ", DEF is now " << DEF << "." << endl;
			pStats[0] = HP; pStats[1] = INT; pStats[2] = STR; pStats[3] = DEF; pStats[4] = LVL; pStats[5] = EXP; //puts updated values into array
			for (int i = 0; i <= 5; i++) //writes the character's stats to the file
				outputFile << pStats[i] << endl;
			outputFile.close();//closes the file.
			system("pause");
		}
		else if (battleWon)
		{
			inputFile.open("pstats.txt"); //opens the file to read
			pStats[0] = HP; pStats[1] = INT; pStats[2] = STR; pStats[3] = DEF; pStats[4] = LVL; pStats[5] = EXP; //writes values to the x array defined earlier
			for (int i = 0; i <= 5; i++) //writes the character's stats to the file
				inputFile >> pStats[i];
			pStats[5] += 1; //increments the sixth value in the array by 1
			EXP = pStats[5]; //stores that value in EXP.
			inputFile.close(); //closes the input connection.

			outputFile.open("pstats.txt");
			for (int i = 0; i <= 5; i++) //writes the character's stats to the file
				outputFile << pStats[i] << endl;
			outputFile.close();//closes the file.
		}
		else //just the usual stat updating between turns
		{
			for (int i = 0; i <= 5; i++) //writes the character's stats to the file
				outputFile << pStats[i] << endl;
			outputFile.close();//closes the file.
		}
	}
}
int BATTLE(int HP, int MP, int STR, int DEF, int playerLevel)
{
	int MHP, MSTR, MDEF, mxp = 1; //These are the monster's stats, which will be determined by the getMonster function later.
	string mName; char mSymb;
	/*Stat definitions
	MHP, MSTR, MDEF - Monster HP, Strength, and Defense. These all behave the same way for the monsters as they do for the player.
	mName - the Monster's name. When a W is pulled from getMonster, the name becomes Big Worm, for example.
	mSymb - the Monster's representative symbol in combat. The player's symbol is an @, for example.*/
	getMonster(playerLevel, mName, mSymb, MHP, MSTR, MDEF);
	cout << setw(7) << '@' << setw(7) << mSymb << endl;
	char choice; //using an integer made the compiler freak out in a way so hilarious I wish we could keep it
	int damageDone; //damageDone keeps numbers consistent between Strength and actual hit damage.
	//bool dead = false;
	cout << "A WILD " << mName << " APPEARED!\n" << "ITS STATS ARE: " << MHP << " HP | " << MSTR << " STR | " << MDEF << " DEF\n";
	while (true)
	{
		cout << "WHAT WILL YOU DO?\n" << "1. ATTACK\n" << "2. SPELL\n";
		cin >> choice;
		switch (choice)
		{
		case '1': //a single attack deals anywhere between 20% more to 20% less of your STR
			damageDone = (STR * 1.2 - (rand() % STR * 0.4)) - (MDEF * 0.1);
			damageDone = (damageDone - MDEF * 0.5); //calculate amount of health saved by defense
			damageDone = (damageDone < 0) ? 0: damageDone; //in case you somehow deal less than zero damage, default to zero.
			MHP = MHP - damageDone;
			cout << "YOUR ATTACK DID " << damageDone << " DAMAGE!\n" << "THE MONSTER HAS " << MHP << " REMAINING HEALTH.\n";
			if (MHP > 0)//fix for posthumous attacking.
			{
				damageDone = (MSTR * 1.2 - (rand() % MSTR * 0.4));
				damageDone = (damageDone - DEF * 0.5); //calculate amount of health saved by defense
				damageDone = (damageDone < 0) ? 0: damageDone; //see above, but for monster
				HP = HP - damageDone;
				cout << "THE " << mName << "'S ATTACK DID " << damageDone << " DAMAGE!\n" << "YOU HAVE " << HP << " REMAINING HEALTH.\n";
			}
			break;
		case '2': //Magic ignores defense so this is a simple calculation
			damageDone = (MP * 2.5);
			MHP = MHP - damageDone;
			cout << "YOUR ATTACK DID " << damageDone << " DAMAGE!\n" << "THE MONSTER HAS " << MHP << " REMAINING HEALTH.\n";
			if (MHP >= 0)//fix for posthumous attacking.
			{
				damageDone = (MSTR * 1.2 - (rand() % MSTR * 0.4));
				damageDone = (damageDone - DEF * 0.5); //calculate amount of health saved by defense
				damageDone = (damageDone < 0) ? 0 : damageDone; //see above, but for monster
				HP = HP - damageDone;
				cout << "THE " << mName << "'S ATTACK DID " << damageDone << " DAMAGE!\n" << "YOU HAVE " << HP << " REMAINING HEALTH.\n";
			}
			break;
		default:
			cout << "That's not a recognized input. Anyway...\n";
			break;
		}
		if (MHP <= 0) //Detects if the monster has been defeated.
		{
			cout << "THE MONSTER HAS BEEN DEFEATED!" << " YOU GAINED " << mxp << " XP.\n";
			return mxp; //returns the exp gained from defeeating the monster.
		}
		if (HP < 0) //Detects if the character has been defeated.
		{
			cout << "YOU HAVE BEEN DEFEATED\n";
			return -1;//Sentinel value signifying the player has been defeated.
		}
	}
}
void getMonster(int levelInput, string& mName, char& mSymb, int& maxHealth, int& attack, int& defense)
//This function generates a single monster for an encounter, based on the level the player puts in.
//maxHealth, attack, and defense correspond to MHP, MSTR, and MDEF respectively, and the values given to them
//here are assigned to those variables back in BATTLE.
{	  //player lvl, monster num
	const char monsters[5][4] = {{'w', 'i', '*', 'z'}, //level 1 monsters; worm, cutout, fly, snake
								 {'!', 'o', '2', 'W'}, //level 2 monsters; bat-fly, slime, cobra, big worm
								 {'1', 'O', '^', '~'}, //level 3 monsters; sapling, evil egg, seagull, parasite
								 {'Y', '&', '#', ':'}, //level 4 monsters; tree, peanut, monster, stack of eyes
								 {'Z', '0', 'L', '%'}};//level 5 monsters; huge snake, good egg, laminated cutout, strange thing
	mSymb = monsters[levelInput][(rand() % 3)]; // pulls a random number between 0 and 3; this will be the monster's symbol.
	switch (mSymb)
	{
		//level 1 monsters start here
	case('w'): {mName = "Worm"; maxHealth = 100; attack = 20; defense = 0; break; }
	case('i'): {mName = "Cardboard Cutout"; maxHealth = 150; attack = 10; defense = 0; break; }
	case('*'): {mName = "Mosquito"; maxHealth = 50; attack = 15; defense = 0; break; }
	case('z'): {mName = "Snake"; maxHealth = 100; attack = 25; defense = 0; break; }
			   //Level 2 Monsters start here. Defense is now being used.
	case('!'): {mName = "Bat Fly"; maxHealth = 100; attack = 30; defense = 0; break; }
	case('o'): {mName = "Slime"; maxHealth = 150; attack = 20; defense = 10; break; }
	case('2'): {mName = "Cobra"; maxHealth = 100; attack = 25; defense = 5; break; }
	case('W'): {mName = "Big Worm"; maxHealth = 200; attack = 15; defense = 5; break; }
			   //Level 3 Monsters begin here.
	case('1'): {mName = "Sapling"; maxHealth = 250; attack = 15; defense = 15; break; }
	case('O'): {mName = "Bad Egg"; maxHealth = 175; attack = 35; defense = 10; break; }
	case('^'): {mName = "Seagull"; maxHealth = 150; attack = 30; defense = 0; break; }
	case('~'): {mName = "Parasite"; maxHealth = 100; attack = 50; defense = 0; break; }
			   //Level 4 Monsters begin here
	case('Y'): {mName = "Tree"; maxHealth = 300; attack = 20; defense = 15; break; }
	case('&'): {mName = "Peanut"; maxHealth = 50; attack = 30; defense = 75; break; }
	case('#'): {mName = "Monster"; maxHealth = 225; attack = 40; defense = 20; break; }
	case(':'): {mName = "Stack of Eyes"; maxHealth = 150; attack = 50; defense = 20; break; }
			   //Level 5 Monsters begin here
	case('Z'): {mName = "Huge Snake"; maxHealth = 250; attack = 35; defense = 15; break; }
	case('0'): {mName = "Good Egg"; maxHealth = 300; attack = 40; defense = 10; break; }
	case('L'): {mName = "Laminated Cutout"; maxHealth = 450; attack = 20; defense = 0; break; }
	case('%'): {mName = "Strange Thing"; maxHealth = 400; attack = 25; defense = 20; break; }
	}

}