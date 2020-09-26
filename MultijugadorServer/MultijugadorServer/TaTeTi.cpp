#include "TaTeTi.h"

#include "iostream"
#include <vector>
#include <cmath>

using namespace std;

TaTeTi::TaTeTi()
{

}

bool TaTeTi::CheckVertical(Game& g)
{
	if (g.cellArray[6] != emptyCell && g.cellArray[6] == g.cellArray[3] && g.cellArray[3] == g.cellArray[0])
		return true;
	else
		if (g.cellArray[7] != emptyCell && g.cellArray[7] == g.cellArray[4] && g.cellArray[4] == g.cellArray[1])
			return true;
		else
			if (g.cellArray[8] != emptyCell && g.cellArray[8] == g.cellArray[5] && g.cellArray[5] == g.cellArray[2])
				return true;
			else
				return false;
}

bool TaTeTi::CheckHorizontal(Game& g)
{
	if (g.cellArray[6] != emptyCell && g.cellArray[6] == g.cellArray[7] && g.cellArray[7] == g.cellArray[8])
		return true;
	else
		if (g.cellArray[3] != emptyCell && g.cellArray[3] == g.cellArray[4] && g.cellArray[4] == g.cellArray[5])
			return true;
		else
			if (g.cellArray[0] != emptyCell && g.cellArray[0] == g.cellArray[1] && g.cellArray[1] == g.cellArray[2])
				return true;
			else
				return false;
}

bool TaTeTi::CheckCrosses(Game& g)
{
	if (g.cellArray[6] != emptyCell && g.cellArray[6] == g.cellArray[4] && g.cellArray[4] == g.cellArray[2])
		return true;
	else
		if (g.cellArray[8] != emptyCell && g.cellArray[8] == g.cellArray[4] && g.cellArray[4] == g.cellArray[0])
			return true;
		else
			return false;
}

bool TaTeTi::CheckAll(Game& g)
{
	return CheckHorizontal(g) || CheckVertical(g) || CheckCrosses(g);
}

bool TaTeTi::CheckInput(Game& g, int input)
{
	if (input > 0 && input <= 9 && g.cellArray[input - 1] == emptyCell)
		return true;
	else
		return false;
}

char TaTeTi::CellToChar(Cell input)
{
	char output;
	switch (input)
	{
	case emptyCell:
		output = '.';
		break;
	case dot:
		output = 'O';
		break;
	case cross:
		output = 'X';
		break;
	}
	return output;
}

bool TaTeTi::MakeMove(Game& g, User player, int cell)
{
	if (CheckInput(g, cell))
	{
		g.cellArray[cell - 1] = player.cellType;
		g.turnsLeft--;

		return true;
	}
	else
	{
		return false;
	}
}

void TaTeTi::SetNextPlayer(Game& g, bool state)
{
	g.nextPlayer = state;
}

void TaTeTi::SetIsInUse(Game& match, bool isInUse)
{
	match.isInUse = isInUse;
}

void TaTeTi::SetTurns(Game& g, int turns)
{
	g.turnsLeft = turns;
}

void TaTeTi::StartGame(Game& g)
{
	for (int f = 0; f < 9; f++)
	{
		g.cellArray[f] = emptyCell;
	}

	g.isInUse = true;
}

void TaTeTi::NextPlayer(Game& g)
{
	g.nextPlayer = !g.nextPlayer;
}

void TaTeTi::RestartCells(Game& g)
{
	for (int f = 0; f < 9; f++)
	{
		g.cellArray[f] = emptyCell;
	}

}

string TaTeTi::GetCurrentGameBoard(Game& g)
{
	g.currentGameBoard = "";
	g.currentGameBoard += '\n';
	for (int i2 = 2; i2 >= 0; i2--)
	{
		for (int i3 = 0; i3 < 3; i3++)
		{
			g.currentGameBoard += CellToChar(g.cellArray[i2 * 3 + i3]);
			g.currentGameBoard += ' ';
		}
		g.currentGameBoard += '\n';
	}

	return g.currentGameBoard;

}

