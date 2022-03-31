Net Blackjack - Charlie Batten

This application demonstrates a possible implementation of Blackjack across the network using WinSock.
It is set up by default for 2 clients to connect to the server, but this can be changed to a different amount by changing the MAX_PLAYERS const in main.cpp in the server project

1. Open the server application
2. Open the client applications.

As stated above, two clients are expected, and if any more attempt to connect, they are refused.
The server application outputs uesful information on how the game is running, and the clients output information only useful to the player.
There is a constant bool defined in the client's main.cpp which controls the ouptut of debug strings to the console for more insight as to how the application runs.

The players are given the choice to hit or stand each turn, and the server waits for all players' inputs before dealing cards for the next turn.
Once a player has chosen to stand, they are kept up to date with the game's output, but are refused the option to make choices each turn until the game concludes.
When there are no active players remaining (i.e. they've bust or have chosen to stand) scores are calculated and displayed to the clients.
The game then ends and thanks the players for playing, before the application terminates.