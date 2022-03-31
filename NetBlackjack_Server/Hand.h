#ifndef HandH
#define HandH

#include "Card.h"

#include <vector>

class Hand
{
public:
	Hand();

	void AddCard(Card new_card);
	void End();
	const Card& GetCard(int index) const;
	int GetTotal() const;
	int GetSoftTotal() const;
	int GetScore() const;
	bool HasAce() const;
	bool HasBust() const;
	bool IsOver() const;
	int Count() const;

private:
	std::vector<Card> hand_;
	bool has_ace_;
	bool is_bust_;
	bool is_over_;
	int total_;
	int soft_total_;
};

#endif