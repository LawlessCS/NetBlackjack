#ifndef DeckH
#define DeckH

#include "Card.h"
#include "Hand.h"

#include <vector>

class Deck
{
public:
	Deck();

	void InitialiseDeck();
	void Shuffle();
	void Deal(Hand& player);
	void Rebuild();

	char ConvertSuit(int suit);
	int GetRemainingCards() const;

private:
	const int DECK_SIZE = 52;

	std::vector<Card> deck_;
};

#endif