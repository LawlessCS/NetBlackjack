#include "Game.h"

#include <iostream>
#include <cassert>
#include <thread>

#include <winsock2.h>	
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

static SRWLOCK mutex;

// Number of clients that the server expects
const int MAX_PLAYERS = 2;
// Current number of players connected
int player_count;

// Array of chars to store fence values sent by clients
char socket_inputs[MAX_PLAYERS];
// Array of chars to store the choices made by clients
char player_choices[MAX_PLAYERS];
// Array of bools to halt threads until data is ready to sent
bool ready_to_send[MAX_PLAYERS];

// Array of bools logging whether the client's is still playing or nor
bool players_hand_over[MAX_PLAYERS];
// Bool to control the main game loop
bool game_running = false;

// Maximum amount of data to be sent/received in the buffer
const int BUFFER_SIZE = 1024;
// String to store the ouptut of the game to sent to clients
std::string game_output;

static void lock_mutex()
{
	AcquireSRWLockExclusive(&mutex);
}

static void unlock_mutex()
{
	ReleaseSRWLockExclusive(&mutex);
}

// Check to see if the clients have sent the specified fence value
bool CheckForReceivedChar(char desired_input)
{
	bool all_ready = true;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (socket_inputs[i] != desired_input)
		{
			all_ready = false;
		}
	}

	return all_ready;
}

// Clear the arrays storing data from clients
void ClearInputs()
{
	memset(socket_inputs, 0, sizeof(char) * MAX_PLAYERS);
	memset(player_choices, 0, sizeof(char) * MAX_PLAYERS);
}

// Set up sockets
int InitialiseWinsock()
{
	std::cout << "Initialising Winsock..\n";

	// Initialise Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	return result;
}

// Function to be run by each thread
// 1 instance per client
int ClientThread(SOCKET client_socket, int player_number)
{
	// Set up buffer for data transmission
	char buffer[BUFFER_SIZE];

	// Clear buffer to zeroes
	memset(buffer, 0, sizeof(char) * BUFFER_SIZE);

	// Send player number
	buffer[0] = player_number;

	// Send client their player number
	send(client_socket, buffer, 1, 0);
	std::cout << "Sent player " << player_number << " their player number\n";

	// Wait for ready message from client
	while (buffer[0] != 'r')
	{
		recv(client_socket, buffer, 1, 0);
		std::cout << "Received " << buffer[0] << " from player " << player_number << "\n";
	}

	// Store ready message from client in input array
	socket_inputs[player_number] = buffer[0];

	std::cout << "player " << player_number << " ready\n";

	std::cout << "Waiting for other players..\n";

	// Wait until the game has begun and all players have joined
	while (!game_running);

	// Send go signal to client
	buffer[0] = 'g';
	send(client_socket, buffer, 1, 0);
	std::cout << "Sent " << buffer[0] << " to player " << player_number << "\n";

	// Loop whilst game is in play
	while (game_running)
	{
		// Clear buffer
		memset(buffer, 0, BUFFER_SIZE);

		// Wait until 'go' signal is received
		while (buffer[0] != 'g')
		{
			recv(client_socket, buffer, 1, 0);
		}

		// Store go signal
		socket_inputs[player_number] = buffer[0];

		// Send game output to client when ready to send
		while (!ready_to_send[player_number]);
		send(client_socket, game_output.c_str(), game_output.length(), 0);
		std::cout << "Sent game output to player " << player_number << "\n";
		ready_to_send[player_number] = false;

		// Receive player's choice from client
		recv(client_socket, buffer, BUFFER_SIZE, 0);
		std::cout << "Recevied " << buffer[0] << " from player " << player_number << "\n";
		player_choices[player_number] = buffer[0];

		// Receive input ready signal from client
		recv(client_socket, buffer, 1, 0);
		socket_inputs[player_number] = buffer[0];
		std::cout << "Received input ready message from player " << player_number << "\n";

		// Wait until ready to send the status of the player's hand
		while (!ready_to_send[player_number]);
		buffer[0] = players_hand_over[player_number] ? 'n' : 'y';
		send(client_socket, buffer, 1, 0);
		std::cout << "Sent player " << player_number << " hand status\n";
		ready_to_send[player_number] = false;

		// Wait until the continue signal is sent by the client
		while (buffer[0] != 'j')
		{
			recv(client_socket, buffer, 1, 0);
		}

		// Store continue signal
		socket_inputs[player_number] = buffer[0];
		std::cout << "Received message received message\n";

		// When ready to sent the status of the game, send it.
		while (!ready_to_send[player_number]);
		buffer[0] = game_running ? 'l' : 'x';
		send(client_socket, buffer, 1, 0);
		std::cout << "Sent " << buffer[0] << " to player " << player_number << "\n";
		ready_to_send[player_number] = false;
	}

	buffer[0] = 'x';

	// Send final game output containing score display screen
	send(client_socket, game_output.c_str(), game_output.length(), 0);
	std::cout << "Sent close message to player " << player_number << "\n";
	game_running = false;

	// Close sockets and tidy up
	closesocket(client_socket);

	return EXIT_SUCCESS;
}

int RunServer(const char* port)
{
	// Assure winsock is ready
	if (InitialiseWinsock() != 0)
	{
		std::cout << "Unable to initialise Winsock\n";
		return EXIT_FAILURE;
	}

	//// Set up information for connecting clients
	//const addrinfo hints =
	//{
	//	.ai_flags = AI_PASSIVE,
	//	.ai_family = AF_INET,
	//	.ai_socktype = SOCK_STREAM,
	//	.ai_protocol = IPPROTO_TCP
	//};

	// Modified version of above code
	addrinfo hints_;
	hints_.ai_flags = AI_PASSIVE;
	hints_.ai_family = AF_INET;
	hints_.ai_socktype = SOCK_STREAM;
	hints_.ai_protocol = IPPROTO_TCP;
	memset(&hints_, 0, sizeof(hints_));

	// Resolve local address and port
	addrinfo* info;
	int result = getaddrinfo(nullptr, port, &hints_, &info);

	if (result != 0)
	{
		std::cout << "getaddrinfo() failed with error: " << result << "\n";
		WSACleanup();

		return EXIT_FAILURE;
	}

	// Create server socket
	SOCKET listen_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

	if (listen_socket == INVALID_SOCKET)
	{
		std::cout << "Unable to create server socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(info);
		WSACleanup();

		return EXIT_FAILURE;
	}

	// Set up the TCP listening socket
	result = bind(listen_socket, info->ai_addr, (int)info->ai_addrlen);

	if (result == SOCKET_ERROR)
	{
		std::cout << "bind() failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(info);
		closesocket(listen_socket);
		WSACleanup();

		return EXIT_FAILURE;
	}

	// Clear no longer needed addrinfo struct
	freeaddrinfo(info);

	// Create bool to track whether max players is reached
	bool hit_max = false;

	// Create array to store client threads
	std::thread client_threads[MAX_PLAYERS];

	// Loop until max players are connected
	while (!hit_max)
	{
		// Listen for connection
		int result = listen(listen_socket, 1);

		// Return 1 if error
		if (result == SOCKET_ERROR)
		{
			std::cout << "listen() failed with error: " << WSAGetLastError() << "\n";
			closesocket(listen_socket);
			WSACleanup();

			return EXIT_FAILURE;
		}

		// Accept client socket
		std::cout << "Waiting for client connection..  ";
		SOCKET client_socket = accept(listen_socket, nullptr, nullptr);

		if (client_socket == INVALID_SOCKET)
		{
			std::cout << "FAILED!\n";
			std::cout << "accept() failed with error: " << WSAGetLastError() << "\n";
			closesocket(listen_socket);
			closesocket(client_socket);
			WSACleanup();

			return EXIT_FAILURE;
		}

		std::cout << "Client connected!\n";

		// Increase player count
		lock_mutex();
		player_count++;
		unlock_mutex();

		// Open thread for client
		client_threads[player_count - 1] = std::thread(ClientThread, client_socket, player_count - 1);

		// Check if hit max players
		lock_mutex();
		hit_max = (player_count == MAX_PLAYERS);
		unlock_mutex();
	}

	// Wait for ready message from all clients
	while (!CheckForReceivedChar('r'));

	// Show beginning game message to console
	std::cout << "\n\nALL PLAYERS READY. BEGINNING GAME!!\n\n";

	// Create game instance with max players
	Game game(MAX_PLAYERS);
	game.Initialise();

	// Mark game as in progress
	lock_mutex();
	game_running = true;
	unlock_mutex();

	// Loop while game running
	while (game_running)
	{
		// Clear all inputs to make sure nothing left in memory
		ClearInputs();

		// Wait until 'go' signal is received
		while (!CheckForReceivedChar('g'));

		// Get game output to send to clients
		game_output = game.ShowHands();
		std::cout << game_output;

		// Mark game output data as ready to be sent
		memset(ready_to_send, true, sizeof(bool) * MAX_PLAYERS);

		// Wait until all players have made their choice (hit or stand)
		while (!CheckForReceivedChar('i'));

		// Apply hit or stand to each player in turn
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			players_hand_over[i] = game.GameStep(i, player_choices[i]);
			std::cout << "Player " << i << " chose " << player_choices[i] << "\n";
		}
		
		// Mark player hand data as ready to send
		memset(ready_to_send, true, sizeof(bool) * MAX_PLAYERS);
		

		// Test to see if the game is over
		std::cout << "Checking game ended\n";
		if (game.HasEnded())
		{
			// End game because everyone's hand is over
			game_output = game.End();
			game_running = false;
			std::cout << "Game has ended\n";
			// Mark game over output as ready to send
			memset(ready_to_send, true, sizeof(bool) * MAX_PLAYERS);
		}
		else
		{
			// Game continues
			std::cout << "Game has not ended\n";
		}

		// Wait until 'continue' signal received from clients
		std::cout << "Ready to send\n";
		while (!CheckForReceivedChar('j'));

		// Mark final data as ready to send
		memset(ready_to_send, true, sizeof(bool) * MAX_PLAYERS);
	}

	// Make sure all threads have finished executing
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (client_threads[i].joinable())
		{
			client_threads[i].join();
		}
	}

	// Close sockets and tidy up
	closesocket(listen_socket);
	WSACleanup();

	return EXIT_SUCCESS;
}

int main()
{
	std::cout << "Net Blackjack - SERVER\n======================\n\n";

	// Ready mutex for controlling data
	InitializeSRWLock(&mutex);

	// Run server on port 17901 - arbitrary value
	int result = RunServer("17901");

	if (result != 0)
	{
		// Log that server closed unexpectedly
		std::cout << "Server closed unexpectedly\n";
	}

	// Server is cloesd
	std::cout << "Server closed with code: " << result;

	// Terminate application
	std::cout << "\n\nPress 'enter' to close..";
	(void)getchar();
	return 0;
}