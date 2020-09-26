#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

enum Cell
{
	emptyCell, dot, cross
};

struct User
{
	sockaddr_in client;
	SOCKET listening;
	string alias;
	bool playing = false;
	bool p1 = false;
	bool p2 = false;
	bool playAgain = false;
	int ID;
	Cell cellType;
};

struct Game
{
	int ID;
	Cell cellArray[9];
	bool isInUse = false;
	bool nextPlayer = true;
	bool p1Joined = false;
	bool p2Joined = false;
	int turnsLeft = 8;
	string currentGameBoard;
};


class TaTeTi
{
private:

public:
	TaTeTi();

	bool CheckVertical(Game& g);
	bool CheckHorizontal(Game& g);
	bool CheckCrosses(Game& g);
	bool CheckAll(Game& g);
	bool CheckInput(Game& g, int input);
	char CellToChar(Cell input);
	bool MakeMove(Game& g, User player, int cell);
	void SetNextPlayer(Game& g, bool state);
	void SetIsInUse(Game& g, bool isInUse);
	void SetTurns(Game& g, int turns);
	void StartGame(Game& g);
	void NextPlayer(Game& g);
	void RestartCells(Game& g);
	string GetCurrentGameBoard(Game& g);
};