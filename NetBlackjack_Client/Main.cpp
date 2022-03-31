#include <iostream>
#include <cassert>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

const bool DEBUG_OUTPUT = FALSE;

void ClearScreen(int player_num = -1)
{
	// Clear screen
	system("CLS");
	std::cout << "\n\n";

	// Draw title header
	if (player_num != -1)
	{
		std::cout << "   BLACKJACK - Player " << player_num + 1 << "\n   =====================\n\n";
	}
}

int SetUpSocket(const char* port, SOCKET& connect_socket)
{
	// Initialise Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	assert(result == 0);

	//// Store information for creating socket
	//const addrinfo hints = {
	//	.ai_family = AF_INET,
	//	.ai_socktype = SOCK_STREAM,
	//	.ai_protocol = IPPROTO_TCP
	//};

	// Modified version of above code
	addrinfo hints_;
	hints_.ai_family = AF_INET;
	hints_.ai_socktype = SOCK_STREAM;
	hints_.ai_protocol - IPPROTO_TCP;
	memset(&hints_, 0, sizeof(hints_));

	// Resolve local server address and port
	addrinfo* info;
	result = getaddrinfo("localhost", "17901", &hints_, &info);

	if (result != 0)
	{
		std::cout << "getaddrinfo() failed with error: " << result << "\n";
		WSACleanup();

		return EXIT_FAILURE;
	}

	// Initialise socket
	connect_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

	if (connect_socket == SOCKET_ERROR)
	{
		std::cout << "Creating socket failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(info);
		WSACleanup();

		return EXIT_FAILURE;
	}

	// Test connection to server
	std::cout << "Attempting to connect to the server..  ";
	result = connect(connect_socket, info->ai_addr, (int)info->ai_addrlen);

	if (result == SOCKET_ERROR)
	{
		std::cout << "FAILED!\n";
		std::cout << "Unable to connect to server: " << WSAGetLastError() << "\n";
		freeaddrinfo(info);
		closesocket(connect_socket);
		WSACleanup();

		return EXIT_FAILURE;
	}

	int player_num = 0;

	std::cout << "Connection successful!\n";

	return EXIT_SUCCESS;
}

void PrintBuffer(char* buffer, int length = -1)
{
	// If length specified print 'length' characters
	if (length != -1)
	{
		for (int i = 0; i < length; i++)
		{
			std::cout << buffer[i];
		}
	}
	// If no length specified, print until null terminator
	else
	{
		int index = 0;

		while (buffer[index] != 0)
		{
			std::cout << buffer[index];
			index++;
		}
	}
}

void MainLoop(SOCKET& connect_socket)
{
	// Set up variables
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	bool player_playing = true;
	bool game_running = false;
	int player_number = 0;

	bool in_play = true;

	// Get player number
	std::cout << "Waiting for data\n";
	memset(buffer, 0, BUFFER_SIZE);
	recv(connect_socket, buffer, 1, 0);
	std::cout << "Recevied player number " << buffer[0] << " from server\n";
	player_number = buffer[0];

	// Send back ready signal
	buffer[0] = 'r';
	send(connect_socket, buffer, 1, 0);
	std::cout << "Sent ready message.\n";

	// Wait for other player slots to be filled
	std::cout << "Waiting for other players to join..\n";

	// Wait until 'go' signal received from server
	while (buffer[0] != 'g')
	{
		recv(connect_socket, buffer, 1, 0);
	}

	// All players connected so begin game
	std::cout << "All players connected!";
	game_running = true;

	// Client side game loop
	while (game_running)
	{
		// Clear buffer
		memset(buffer, 0, BUFFER_SIZE);

		// Send go signal to server
		buffer[0] = 'g';
		send(connect_socket, buffer, 1, 0);

		// Receive game output data into buffer
		recv(connect_socket, buffer, BUFFER_SIZE, 0);

		char choice = 'C';

		// If player is still in play, then offer hit/stand choice
		if (player_playing)
		{
			// Set choice to neutral value
			// Loop while not valid choice
			while (choice != 'h' && choice != 's')
			{
				// Clear the screen and draw the game output
				ClearScreen(player_number);
				PrintBuffer(buffer);
				// Ssk player for input
				std::cout << "Player " << player_number + 1 << ": [H]it or [S]tand: ";
				// Store player input in 'choice'
				choice = getchar();
			}
		}
		// if the player has stood or is bust, don't offer the choice of H/S
		else
		{
			ClearScreen(player_number);
			PrintBuffer(buffer);

			// Output that the player can not make choice
			std::cout << "Player " << player_number << ": You have unable to make a choice - please wait.\n";

			choice = 's';
		}

		// Send choice to server
		send(connect_socket, &choice, 1, 0);
		if (DEBUG_OUTPUT)
			std::cout << "Sent option " << choice << " to the server\n";

		// Send input ready signal to server
		buffer[0] = 'i';
		send(connect_socket, buffer, 1, 0);
		if (DEBUG_OUTPUT)
			std::cout << "Sent input ready message to the server\n";
		std::cout << "\n\nWaiting for other players..\n";

		// Receive status as to whether the player is still playing or not
		recv(connect_socket, buffer, 1, 0);
		if (DEBUG_OUTPUT)
			std::cout << "Received hand status: " << buffer[0] << "\n";
		
		// Parse received hand status
		if (buffer[0] == 'y')
		{
			player_playing = true;
		}
		else if (buffer[0] == 'n')
		{
			player_playing = false;
		}

		// Send continue signal to server
		buffer[0] = 'j';
		send(connect_socket, buffer, 1, 0);
		if (DEBUG_OUTPUT)
			std::cout << "Sent message received message\n";
		

		// Wait for received game status signal
		recv(connect_socket, buffer, 1, 0);

		// If received 'x' then game has ended
		if (buffer[0] == 'x')
		{
			if (DEBUG_OUTPUT)
				std::cout << "Game ends here\n";
			game_running = false;
		}
		// If received 'l' then game loops again
		if (buffer[0] == 'l')
		{
			if (DEBUG_OUTPUT)
				std::cout << "Game loops again\n";
		}
	}

	// Clear buffer
	memset(buffer, 0, BUFFER_SIZE);

	// Receive score display
	recv(connect_socket, buffer, BUFFER_SIZE, 0);

	// Clear screen
	ClearScreen(player_number);

	// Print score display
	PrintBuffer(buffer);
}

int main()
{
	std::cout << "NETWORK TEST - CLIENT\n=====================\n\n";

	// Cretae and set up network connection
	SOCKET connect_socket;
	int result = SetUpSocket("17901", connect_socket);
	std::cout << "CreateSocket() returned with code: " << result << "\n\n";

	if (result == 0)
	{
		// Run main game
		MainLoop(connect_socket);
		std::cout << "\n\n\nThank you for playing!";
	}

	// Clean up and close
	closesocket(connect_socket);
	WSACleanup();

	std::cout << "\n\nPress 'enter' to close..";

	while (getchar() != '\n');
	(void)getchar();
	return 0;
}