#include "Hand.h"

Hand::Hand() : total_(0), has_ace_(false), is_bust_(false), is_over_(false), soft_total_(0)
{

}

void Hand::AddCard(Card new_card) {
	// Add new card to hand
	hand_.push_back(new_card);

	// Increate total by value of card
	total_ += new_card.GetValue();
	// Increase soft total by value of card
	soft_total_ += new_card.GetValue();

	// If card is ace, remove 10 from soft total (ace has value or 1 or 11)
	if (new_card.GetSymbol() == 'A')
	{
		has_ace_ = true;
		soft_total_ -= 10;
	}

	// If total is over 21 and no ace present player busts
	if (total_ > 21 && has_ace_ == false)
	{
		is_bust_ = true;
	}
	// If players has ace and soft total is over 21, player busts
	else if (soft_total_ > 21)
	{
		is_bust_ = true;
	}
}

const Card& Hand::GetCard(int index) const
{
	// Return card at index in hand
	return hand_.at(index);
}

bool Hand::HasAce() const
{
	// Check if player has ace
	return has_ace_;
}

bool Hand::HasBust() const
{
	// Check if player is bust
	return is_bust_;
}

bool Hand::IsOver() const
{
	// Check if player is out
	return is_over_;
}

void Hand::End()
{
	// Mark player's hand as over
	is_over_ = true;
}

int Hand::GetTotal() const
{
	// Return hard total value 
	return total_;
}

int Hand::GetSoftTotal() const
{
	// Return soft total
	return soft_total_;
}

int Hand::GetScore() const
{
	// Return player's score - 0 if bust else total
	if (is_bust_)
	{
		return 0;
	}

	if (total_ <= 22)
	{
		return total_;
	}

	return soft_total_;
}

int Hand::Count() const
{
	// Return number of cards in hand
	return (int)hand_.size();
}