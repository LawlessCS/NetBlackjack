#include "Card.h"

Card::Card() : value_(0), symbol_('0'), suit_('0')
{
};

Card::Card(char symbol, char suit) : value_(symbol - 48), symbol_(symbol), suit_(suit)
{
};

Card::Card(char symbol, char suit, int value) : value_(value), symbol_(symbol), suit_(suit)
{
};

int Card::GetValue() const
{
	// Return value of card (1, 11)
	return value_;
}

char Card::GetSymbol() const
{
	// Return symbol of card (J, Q, K, A)
	return symbol_;
}

char Card::GetSuit() const
{
	// Return suit of card (C, H, S, D)
	return suit_;
}