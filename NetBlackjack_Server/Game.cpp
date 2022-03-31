#include "Game.h"

#include <iostream>
#include <sstream>

Game::Game(int max_players) : MAX_PLAYERS_(max_players)
{
	// Create space in vector for all players
	players_.reserve(MAX_PLAYERS_);
}

void Game::Initialise()
{
	// Deck is created implicitly in constructor
	// Shuffle deck
	deck_.Shuffle();
	// Create hands for players
	SetUpPlayers();
	// Give players 2 initial cards
	InitialDeal();
}

std::string Game::End()
{
	// Calculate player(s) with highest scores - stored locally
	CalculateWinners(); 
	// Return score output string 
	return GetScores();
}

void Game::ClearScreen()
{
	// Clear console
	system("cls");
}

Deck Game::CreateDeck()
{
	// Create and shuffle deck
	Deck deck;
	deck.Shuffle();

	return deck;
}

void Game::SetUpPlayers()
{
	// Number of players playing
	int player_count = MAX_PLAYERS_;

	// Create hand for each player
	for (int i = 0; i < player_count; i++)
	{
		// Add newly created player to vector
		players_.emplace_back();
	}
}

void Game::InitialDeal()
{
	// Deal each player 2 cards
	for (Hand& player : players_)
	{
		for (int i = 0; i < 2; i++)
		{
			// If no cards left in deck
			if (deck_.GetRemainingCards() == 0) {
				// Create new deck and shuffle
				deck_.Rebuild();
				deck_.Shuffle();
			}

			// Otherwise deal top card to player
			deck_.Deal(player);
		}
	}
}

std::string Game::ShowHands(int current_player)
{
	// Create stringstream to store output string
	std::stringstream ss;
	ss << "\n";

	// Show all players' cards
	for (size_t i = 0; i < players_.size(); i++)
	{
		const Hand& hand = players_.at(i);

		ss << "   ";

		// Output player number
		ss << "Player " << i + 1 << ":  ";

		// Show each card
		for (int j = 0; j < hand.Count(); j++)
		{
			// Output card's symbol and suit
			const Card& card = hand.GetCard(j);
			ss << card.GetSymbol() << card.GetSuit() << " ";
		}

		// Show totals
		ss << "\tTotal: ";

		// Show hard total if not bust
		if (hand.GetTotal() <= 21)
		{
			ss << hand.GetTotal();

			if (hand.HasAce())
			{
				ss << '/';
			}
		}

		// Show soft total if hand contains ace
		if (hand.HasAce())
		{
			ss << hand.GetSoftTotal();
		}

		// Show BLACKJACK if hand totals 21 with two cards
		if (hand.GetTotal() == 21 && hand.Count() == 2) {
			ss << "\tBLACKJACK";
		}

		// Show BUSTED if total is over 21
		else if (hand.HasBust()) {
			ss << "\tBUSTED!";
		}

		// Show STAND if the hand has ended
		else if (hand.IsOver()) {
			ss << "\tSTAND";
		}

		ss << '\n';
	}

	ss << "\n\n";

	return ss.str();
}

bool Game::GameStep(int player_number, char choice)
{
	// Get reference to hand for given player
	Hand& hand = players_.at(player_number);

	// Hit
	if (choice == 'h')
	{		// Deal another card and loop
		if (deck_.GetRemainingCards() == 0)
		{
			// Rebuild deck if no cards left
			deck_.Rebuild();
		}

		deck_.Deal(hand);
	}
	else if (choice == 's')
	{
		// Go to next player
		hand.End();
	}
	else
	{
		// Should never run - function recevied non 'h' or 's' input
		std::cout << "ERROR IN GameStep Function";
	}

	// Return false if player is still in game
	return (hand.IsOver() || hand.HasBust());
}

bool Game::HasEnded()
{
	// Check to see if the game will continue 

	bool all_over = true;

	for (Hand& player : players_)
	{
		if (!player.IsOver() && !player.HasBust())
		{
			all_over = false;
		}
	}

	return all_over;
}

void Game::CalculateWinners()
{
	// Calculate player(s) who have the highest score

	int max_score = 0;

	for (size_t i = 0; i < players_.size(); i++)
	{
		int score = players_.at(i).GetScore();

		max_score = (score > max_score) ? score : max_score;
	}

	if (max_score != 0)
	{
		for (size_t i = 0; i < players_.size(); i++)
		{
			if (players_.at(i).GetScore() == max_score)
			{
				// Add player number to winners array
				winners_.push_back(i);
			}
		}
	}
}

std::string Game::GetScores()
{
	// Create stringstream to output score display
	std::stringstream ss;

	for (size_t i = 0; i < players_.size(); i++)
	{
		const Hand& hand = players_.at(i);

		ss << "   ";

		// Ouptut player's number
		ss << "Player " << i + 1 << ": ";

		int score = hand.GetScore();

		// If player has bust, output "BUST" + their score, else output just their score
		if (score == 0)
		{
			ss << "BUST (" << hand.GetTotal() << ")";
		}
		else
		{
			ss << score;
		}

		// S p a c i n g 
		ss << "\t\t";

		// Ouptut cards in player's hand
		for (int i = 0; i < hand.Count(); i++)
		{
			ss << hand.GetCard(i).GetSymbol() << hand.GetCard(i).GetSuit() << " ";
		}
			
		ss << "\n";
	}

	// If nobody won (everyone bust)
	if (winners_.size() == 0)
	{
		ss << "\n   Unfortunatley, everyone bust :(";
	}
	else
	{
		// Show winning players by number if multiple won
		ss << "\n   Congratulations player";

		if (winners_.size() != 1)
		{
			ss << "s ";

			for (size_t i = 0; i < winners_.size() - 1; i++)
			{
				// Output winning player's number
				ss << winners_.at(i) + 1 << ", ";
			}

			ss << winners_.at(winners_.size() - 1) + 1;
		}
		// If 1 player won
		else
		{
			ss << ' ' << winners_.at(0) + 1;
		}

		ss << " - you won!";
	}

	// Return created string
	return ss.str();
}