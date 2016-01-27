#include <iostream>
#include <cstdlib>
#include <ctime>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using namespace boost::interprocess;

//constants
const int SIZE = 10;
const int NUM_SHIPS = 5;
const int SHIP_SIZES[] = {5,4,3,3,2};

struct position
{
	short letter;
	short number;
};

//Global Variables
boost::posix_time::ptime time_hack(boost::posix_time::second_clock::local_time()+boost::posix_time::seconds(10));
int letter_mine;
int number_mine;
int response_mine;
int letter_theirs;
int number_theirs;
int response_theirs;
int player_number;

short mine[SIZE][SIZE];//0=not there, 1=ship is here, 2=hit, 3=destroyed
short theirs[SIZE][SIZE];//0=unknown, 1=miss, 2=hit
position ship_locations[NUM_SHIPS][2];//front and end of ship
short my_ships_remaining;//number of ships still alive
short their_ships_remaining;//number of ships still left to sink
position choice;//the position selected as the next move

//function prototypes
void reset();
bool checkIfDestroyed();
void send(message_queue& mq, int& num, unsigned int pri);
void receive(message_queue& mq, int& num);
void setup();
position decision();

int main()
{
	srand(static_cast<unsigned int>(time(0)));//seed the random number gen
	//initialize message queues
	try
	{
		message_queue input(open_only, "input_Bob");
		message_queue output(open_only, "output_Bob");	
		while(true)
		{
			reset();
			setup();
			//get input number
			receive(input, player_number);
			
			if(player_number == 0)
			{
				break;
			}
			
			if(player_number == 2)
			{
				//Get Enemy Position
				receive(input, letter_theirs);
				receive(input, number_theirs);
				
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
				send(output, response_mine, 2);
			}

			while(true)
			{
				//Choice a Position
				choice = decision();
				
				//Send Position
				letter_mine = choice.letter;
				number_mine = choice.number;			
				send(output, letter_mine, 1);
				send(output, number_mine, 0);
				
				//Wait for Response
				receive(input, response_theirs);
				
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
					//cout << "Game Over! I Win!" << endl;
					break;
				}
				
				//...enemy turn...
				
				//Get Enemy Position
				receive(input, letter_theirs);
				receive(input, number_theirs);
				
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
				send(output, response_mine, 2);
				
				if(my_ships_remaining == 0)
				{
					//They win
					//cout << "Game Over! I Lost!" << endl;
					response_mine = 0;
					send(output, response_mine, 1);
					break;
				}
			}
		}
		message_queue::remove("input_Bob");
		message_queue::remove("output_Bob");
	}
	catch(interprocess_exception &e)
	{
		message_queue::remove("input_Bob");
		message_queue::remove("output_Bob");
		cout << e.what() << endl;
		return 1;
	}
	return 0;
}

void send(message_queue& mq, int& num, unsigned int pri)
{
	while(!mq.timed_send(&num, sizeof(num), pri, time_hack)){};
	//cout << "Sent: " << num << endl;
}

void receive(message_queue& mq, int& num)
{
	message_queue::size_type recvd_size;
	unsigned int priority;
	while(!mq.timed_receive(&num, sizeof(num), recvd_size, priority, time_hack)){}
	//cout << "Received: " << num << endl;
}

void reset()
{
	my_ships_remaining = NUM_SHIPS;
	their_ships_remaining = NUM_SHIPS;
	for(int i=0;i<SIZE;i++)
	{
		for(int j=0;j<SIZE;j++)
		{
			mine[i][j] = 0;
			theirs[i][j] = 0;
		}
	}
}

bool checkIfDestroyed()
{
	short hits_on_ship;
	for(int i=0;i<NUM_SHIPS;i++)
	{
		hits_on_ship = 0;
		for(int j=0;j<SHIP_SIZES[i];j++)
		{
			if(ship_locations[i][0].letter == ship_locations[i][1].letter)//vertical
			{
				if(mine[ship_locations[i][0].letter][ship_locations[i][0].number+j]==2)
				{
					hits_on_ship++;
				}
			}
			else//horizontal
			{
				if(mine[ship_locations[i][0].letter+j][ship_locations[i][0].number]==2)
				{
					hits_on_ship++;
				}
			}
			if(hits_on_ship == SHIP_SIZES[i])
			{
				for(j=j-1;j>=0;j--)
				{
					if(ship_locations[i][0].letter == ship_locations[i][1].letter)//vertical
					{
						mine[ship_locations[i][0].letter][ship_locations[i][0].number+j]=3;
					}
					else//horizontal
					{
						mine[ship_locations[i][0].letter+j][ship_locations[i][0].number]=3;
					}
				}
				my_ships_remaining--;
				return true;
			}
		}
	}
	return false;
}

void setup()//responsible for placement of ships
{
	//random method
	int rot, let, num;
	bool next_ship = false;
	int i=0;
	while(i<NUM_SHIPS)
	{
		rot = rand()%2;//0=horizontal, 1=vertical
		let = rand()%(SIZE-SHIP_SIZES[i]+1);
		num = rand()%(SIZE-SHIP_SIZES[i]+1);
		for(int j=0;j<SHIP_SIZES[i];j++)
		{
			next_ship=true;
			if(rot)//vertical
			{
				if(mine[let][num+j]==1)
				{
					next_ship=false;
					break;
				}
			}
			else//horizontal
			{
				if(mine[let+j][num]==1)
				{
					next_ship=false;
					break;
				}
			}
		}
		if(next_ship)
		{
			ship_locations[i][0].letter = let;
			ship_locations[i][0].number = num;
			for(int j=0;j<SHIP_SIZES[i];j++)
			{
				if(rot)
				{
					mine[let][num+j] = 1;
				}
				else
				{
					mine[let+j][num] = 1;
				}
			}
			if(rot)
			{
				ship_locations[i][1].letter = let;
				ship_locations[i][1].number = num+SHIP_SIZES[i];
			}
			else
			{
				ship_locations[i][1].letter = let+SHIP_SIZES[i];
				ship_locations[i][1].number = num;
			}
			i++;
		}
	}
}

position decision()
{
	//linear method
	position output;
	for(int i=0;i<SIZE;i++)
	{
		for(int j=0;j<SIZE;j++)
		{
			if(theirs[i][j]==0)
			{
				output.letter = i+1;
				output.number = j+1;
				return output;
			}
		}
	}
}

//Notes:
//how you interpret the game board doesnt matter.
//http://www.hasbro.com/common/instruct/Battleship.PDF

//Rule Differences:
//no ship announcements.
//hit sink miss only