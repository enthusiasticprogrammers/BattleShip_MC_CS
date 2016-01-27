#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using namespace boost::interprocess;

//Global Variables
unsigned int priority = 2;
message_queue::size_type recvd_size;
boost::posix_time::ptime time_hack(boost::posix_time::second_clock::local_time()+boost::posix_time::seconds(10));
int player_number_1 = 1;
int player_number_2 = 2;
int letter_1;
int number_1;
int response_1;
int letter_2;
int number_2;
int response_2;

//game master variables
int games_played = 50;
int number_1_won = 0;
int number_2_won = 0;

//function prototypes
void send(message_queue& mq, int& num);
void receive(message_queue& mq, int& num);

int main(int argc, char *argv[])
{
	srand(static_cast<unsigned int>(time(0)));//seed the random number gen
	try
	{
		message_queue::remove("input_Bob");
		message_queue::remove("output_Bob");
		message_queue::remove("input_Sam");
		message_queue::remove("output_Sam");
		message_queue input_1(create_only, "input_Bob", 3, sizeof(int));
		message_queue output_1(create_only, "output_Bob", 3, sizeof(int));
		message_queue input_2(create_only, "input_Sam", 3, sizeof(int));
		message_queue output_2(create_only, "output_Sam", 3, sizeof(int));
		if(argc != 2)
		{
			player_number_1 = 0;
			player_number_2 = 0;
			send(input_1, player_number_1);
			send(input_2, player_number_2);
			return 1;
		}
		games_played = atoi(argv[1]);
		
		while(games_played > 0)
		{
			//send player numbers
			send(input_1, player_number_1);
			send(input_2, player_number_2);
			if(player_number_1 == 2)
			{
				receive(output_2, letter_2);
				send(input_1, letter_2);
				receive(output_2, number_2);
				send(input_1, number_2);
				receive(output_1, response_1);
				send(input_2, response_1);
			}
			while(true)
			{
				//Player 1 requests a position
				receive(output_1, letter_1);
				if(letter_1 == 0)
				{
					//Player 1 Lost. Player 2 Wins
					number_2_won++;
					//cout << "Player 2 Wins" << endl;
					break;
				}
				send(input_2, letter_1);
				receive(output_1, number_1);
				send(input_2, number_1);
				
				//Player 2 responds
				receive(output_2, response_2);
				send(input_1, response_2);
				
				//Player 2 requests a position
				receive(output_2, letter_2);
				if(letter_2 == 0)
				{
					//Player 2 Lost. Player 1 Wins
					number_1_won++;
					//cout << "Player 1 Wins" << endl;
					break;
				}
				send(input_1, letter_2);
				receive(output_2, number_2);
				send(input_1, number_2);
				
				//Player 1 responds
				receive(output_1, response_1);
				send(input_2, response_1);
			}
			games_played--;
			player_number_1 = rand()%2+1;
			player_number_2 = 3 - player_number_1;
		}
		player_number_1 = 0;
		player_number_2 = 0;
		send(input_1, player_number_1);
		send(input_2, player_number_2);
		cout << "Wins    Player 1: " << number_1_won << "    Player 2: " << number_2_won << endl;
	}
	catch(interprocess_exception &e)
	{
		message_queue::remove("input_Bob");
		message_queue::remove("output_Bob");
		message_queue::remove("input_Sam");
		message_queue::remove("output_Sam");
		cout << e.what() << endl;
		return 1;
	}
	return 0;
}

void send(message_queue& mq, int& num)
{
	while(!mq.timed_send(&num, sizeof(num), priority, time_hack));
}

void receive(message_queue& mq, int& num)
{
	while(!mq.timed_receive(&num, sizeof(num), recvd_size, priority, time_hack)){}
}