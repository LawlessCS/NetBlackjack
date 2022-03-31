#include "Deck.h"

#include <algorithm>
#include <random>
#include <chrono>

Deck::Deck()
{
	InitialiseDeck();
}

void Deck::InitialiseDeck()
{
	// Set deck vector to correct size
	deck_.reserve(DECK_SIZE);

	// Populate deck
	for (int suit_int = 0; suit_int < 4; suit_int++)
	{
		// Add number cards
		for (int value = 2; value < 10; value++) {
			deck_.emplace_back(value+48, ConvertSuit(suit_int));
		}

		// Add picture cards and ace
		deck_.emplace_back('T', ConvertSuit(suit_int), 10);
		deck_.emplace_back('J', ConvertSuit(suit_int), 10);
		deck_.emplace_back('Q', ConvertSuit(suit_int), 10);
		deck_.emplace_back('K', ConvertSuit(suit_int), 10);
		deck_.emplace_back('A', ConvertSuit(suit_int), 11);
	}
}

void Deck::Shuffle() 
{
	// Set new random seed for unique shuffle
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	// Shuffle cards
	std::shuffle(deck_.begin(), deck_.end(), std::default_random_engine(seed));
}

void Deck::Deal(Hand& player)
{
	// Save top card
	player.AddCard(deck_.front());

	// Remove top card from deck
	deck_.erase(deck_.begin());
}

void Deck::Rebuild()
{
	InitialiseDeck();
}

char Deck::ConvertSuit(int suit)
{
	// Char encoding for clubs spades hearts diamonds characters
	switch (suit)
	{
	case 0:
		return '\u0006';
	case 1:
		return '\u0003';
	case 2:
		return '\u0005';
	case 3:
		return '\u0004';
	}
	
	return '0';
}

int Deck::GetRemainingCards() const
{
	// Return number of cards remaining in deck 
	return (int)deck_.size();
}
