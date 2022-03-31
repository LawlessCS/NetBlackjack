#ifndef GameH
#define GameH

#include "Card.h"
#include "Deck.h"
#include "Hand.h"

#include <vector>
#include <string>

class Game {
public:
	Game(int max_players);
	void Initialise();
	std::string ShowHands(int current_player = -1);
	bool GameStep(int player_numpber, char choice);
	bool HasEnded();
	std::string End();

	void ClearScreen();

private:
	Deck CreateDeck();
	void SetUpPlayers();
	void InitialDeal();
	void CalculateWinners();
	std::string GetScores();

	const int MAX_PLAYERS_;

	Deck deck_;
	std::vector<Hand> players_;
	std::vector<int> winners_;

};

#endif