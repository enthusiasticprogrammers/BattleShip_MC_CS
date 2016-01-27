/* BattleShip Code Template
Feel free to modify the code however you wish however, try to use the variables to
handle all input and output from the user for now. When we actually get people to
test their program vs eachother, other libraries will be included so cin/cout will
not be necessary.

You do not have to use the functions defined at the bottom. Instead, if you wish
you can simply program only 2 functions: setup and decision. These should be the
only unique code between different people since most of the action occurs here.

The setup function should be responsible for placing the ships on your grid.
The decision function should be responsible for choosing your next move.

Note instead of sending a letter and a number. we will be sending 2 numbers instead.
This will allow us to easily change the size of the board to have more fun games.
In a test on my laptop, 2 computers could play 1 million games in 10 minutes 30 seconds.
That being said, I would recommend creating algorithms that can still work if we add
more ships into the game and create a larger field.

When gathering a response of hit/miss/destroyed we instead reply with a number
0, 1, or 2 instead of the letters. However, how you decide to handle the data is up
to you.
*/

#include <iostream>
#include <cstdlib>
#include <ctime>
const int SIZE = 10;
const int NUM_SHIPS = 5;
const int SHIP_SIZES[] = {5,4,3,3,2};

struct position
{
	short letter;
	short number;
};

//these variables below are needed for all input/output. Use them.
int letter_mine;
int number_mine;
int response_mine;
int letter_theirs;
int number_theirs;
int response_theirs;
int player_number;
//these variables above are needed for all input/output. Use them.

short mine[SIZE][SIZE];//0=not there, 1=ship is here, 2=hit, 3=destroyed
short theirs[SIZE][SIZE];//0=unknown, 1=miss, 2=hit

int main()
{
	srand(static_cast<unsigned int>(time(0)));//seed the random number gen
	
	//the main try-catch loop is needed to check for memory leaks in the final program
	try
	{
		while(true)
		{
			reset();
			setup();
			//get input number
			cin >> player_number;
			
			//You keep playing until the server tells you that you are player 0. this is the signal to exit
			if(player_number == 0)
			{
				break;
			}
			
			//If you are player number 2, you have to first say miss/hit then you can start calling positions
			if(player_number == 2)
			{
				//Get Enemy Position
				cin >> letter_theirs;
				cin >> number_theirs;
				
				//Process Response
				if(mine[letter_theirs-1][number_theirs-1]==1)
				{
					//hit
					mine[letter_theirs-1][number_theirs-1]=2;
					response_mine = 1;
				}
				else
				{
					//miss
					response_mine = 0;
				}
				
				//Send Response
				cout << response_mine << endl;
			}

			while(true)
			{
				//Choose a Position
				choice = decision();
				
				//Send Position
				letter_mine = choice.letter;
				number_mine = choice.number;			
				cout << letter_mine << endl;
				cout << number_mine << endl;
				
				//Wait for Response
				cin >> response_theirs;
				
				//Process Response
				if(response_theirs == 0)
				{
					//miss
					theirs[choice.letter-1][choice.number-1]=1;
				}
				else if(response_theirs == 1)
				{
					//hit
					theirs[choice.letter-1][choice.number-1]=2;
				}
				else if(response_theirs == 2)
				{
					//destroyed
					theirs[choice.letter-1][choice.number-1]=2;
					their_ships_remaining--;
				}
				else
				{
					//Error
					cout << "Error!" << endl;
				}
				
				if(their_ships_remaining == 0)
				{
					//I win
					cout << "Game Over! I Win!" << endl;
					break;
				}
				
				//...enemy turn occurs here...
				
				//Get Enemy Position
				cin >> letter_theirs;
				cin >> number_theirs;
				
				//Process Response
				if(mine[letter_theirs-1][number_theirs-1]==1)
				{
					mine[letter_theirs-1][number_theirs-1]=2;
					if(checkIfDestroyed())
					{
						//destroyed
						response_mine = 2;
					}
					else
					{
						//hit
						response_mine = 1;
					}
				}
				else
				{
					//miss
					response_mine = 0;
				}
				
				//Send Response
				cout << response_mine << endl;
				
				if(my_ships_remaining == 0)
				{
					//They win
					cout << "Game Over! I Lost!" << endl;
					response_mine = 0;
					cout << response_mine << endl;
					break;
				}
			}
		}
	}
	catch()
	{
		//Error
		return 1;
	}
	return 0;
}

void reset()
{
}

bool checkIfDestroyed()//true if a ship was destroyed, false otherwise
{
}

void setup()//responsible for placement of ships
{
}

position decision()//responsible for choosing your next move
{
}

//Notes:
//how you interpret the game board doesnt matter.
//http://www.hasbro.com/common/instruct/Battleship.PDF

//Rule Differences:
//no ship announcements.
//hit sink miss only