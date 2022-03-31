#ifndef CardH
#define CardH

class Card
{
public:
	Card();
	Card(char symbol, char suit);
	Card(char symbol, char suit, int value);

	int GetValue() const;
	char GetSymbol() const;
	char GetSuit() const;

private:
	int value_;
	char symbol_;
	char suit_;
	bool face_up_ = false;	
};

#endif