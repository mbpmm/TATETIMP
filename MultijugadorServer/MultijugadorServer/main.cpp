#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include "TaTeTi.h"

#pragma comment (lib, "ws2_32.lib")

#define ADD_PLAYER '1'
#define MAKE_MOVES '2'
#define START_GAME '3'
#define EXIT '4'

using namespace std;

int matchID = 0;

struct message 
{
	byte cmd;
	char data[255];
};

int randomInt(int a, int b)
{
	return ((int)rand() / (int)RAND_MAX) * (b - a) + a;
}

//char* concat(const char* a, const char* b) 
//{
//	int lena = strlen(a);
//	int lenb = strlen(b);
//	char* con = malloc(lena + lenb + 1);
//	// copy & concat (including string termination)
//	memcpy(con, a, lena);
//	memcpy(con + lena, b, lenb + 1);
//	return con;
//}

int GetEmptyGame(vector<Game>& games, TaTeTi& tateti);

bool StartGame(vector<Game>& games, TaTeTi& tateti, vector<User>& users);

void DrawBoard(vector<User>& users, vector<Game>& games, TaTeTi& tateti, int id, message& msg);

void NextTurn(vector<User>& users, vector<Game>& games, TaTeTi& tateti, int id, message& msg);

int main() {
	TaTeTi tateti;

	vector<User> users;

	vector<Game> games;

	//iniciar winsock
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &data);
	if (wsOk!=0)
	{
		cerr << "No pudo iniciar winsock" << endl;
		return -1;
	}

	//crear listening socket
	SOCKET listening = socket(AF_INET, SOCK_DGRAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Invalid socket" << endl;
		return -1;
	}

	//bind el socket (atar un socket a la dupla ip:puerto)
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(8900); //puerto en el que vamos a escuchar
	hint.sin_addr.S_un.S_addr = ADDR_ANY; // es lo mismo que abajo
	//inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr); //ip de loopback

	int bindResult=bind(listening, (sockaddr*)&hint, sizeof(hint));

	if (bindResult==SOCKET_ERROR)
	{
		cerr << "No pudo hacer el bind" << endl;
		return -1;
	}

	//recibir data del socket y procesarla (proceso bloqueante)

	char buf[1024];
	message msgRcd;
	message msgToClient;
	//estructura con la data del cliente que nos esta enviando mensajes
	sockaddr_in client;
	int clientSize = sizeof(client);

	memset(buf, 0, sizeof(buf));
	memset(&msgRcd, 0, sizeof(msgRcd));
	memset(&msgToClient, 0, sizeof(msgToClient));
	string msgtest = ".";

	while (buf!="\n")
	{
		//ZeroMemory(buf, sizeof(buf)); //recorre el buffer y lo llena de ceros
		memset(buf, 0, sizeof(buf)); // es lo mismo que arriba pero ZeroMemory es solo de windows 
		memset(&msgRcd, 0, sizeof(msgRcd));
		memset(&msgToClient, 0, sizeof(msgToClient));
		
		//funcion bloqueante
		int bytesIn = recvfrom(listening, (char*)&msgRcd, sizeof(msgRcd), 0, (sockaddr*)&client, &clientSize);

		char ip[1024];
		unsigned short port = client.sin_port;

		inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));

		if (bytesIn == SOCKET_ERROR)
		{
			cerr << "Error al recibir data" << endl;
			return -1;
		}

		switch (msgRcd.cmd)
		{
		case ADD_PLAYER: {
			User u;
			u.client = client;
			u.listening = listening;
			users.push_back(u);
			cout << "Nuevo jugador: " << msgRcd.data << " (" << ip << ":" << port << ")" << endl;
			for (int i = 0; i < users.size(); i++)
			{
				if ((users[i].client.sin_addr.s_addr == client.sin_addr.s_addr) && (users[i].client.sin_port == client.sin_port))
				{
					users[i].alias = msgRcd.data;
					cout << endl;
				}
			}
			msgToClient = *((message*)"1Usuario registrado. Escriba 3 para jugar");
			sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
			break; }
		case MAKE_MOVES:
		{
			int index = 0;
			for (int i = 0; i < users.size(); i++)
			{
				if ((users[i].client.sin_addr.s_addr == client.sin_addr.s_addr) && (users[i].client.sin_port == client.sin_port))
				{
					if (users[i].playing && !users[i].playAgain)
					{
						index = i;
						break;
					}
				}
			}

			int playerInput = (int)msgRcd.data[0] - '0';
			int matchID = users[index].ID;

			if (tateti.MakeMove(games[matchID], users[index], playerInput))
			{
				DrawBoard(users, games, tateti, matchID, msgToClient);

				if (tateti.CheckAll(games[matchID]))
				{
					// hay un ganador
					string winnerCellType(1, tateti.CellToChar(users[index].cellType));


					string s = "1Ganaste! Para jugar de nuevo escribe 3, para salir escribe 4";
					msgToClient = *((message*)s.c_str());
					sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));

					users[index].playing = false;
					users[index].playAgain = false;

					if (users[index].p1) // gano p1
					{
						for (int i = 0; i < users.size(); i++)
						{
							if (users[i].ID == matchID && users[i].p2)
							{
								string s = "1Jugador " + winnerCellType + " gano. Para jugar de nuevo escribe 3, para salir escribe 4";

								msgToClient = *((message*)s.c_str());
								sendto(users[i].listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

								users[i].playing = false;
								users[i].playAgain = false;

								break;
							}
						}
					}
					else if (users[index].p2) // gano p2
					{
						for (int i = 0; i < users.size(); i++)
						{
							if (users[i].ID == matchID && users[i].p1)
							{
								string s = "1Jugador " + winnerCellType + " gano. Para jugar de nuevo escribe 3, para salir escribe 4";

								msgToClient = *((message*)s.c_str());
								sendto(users[i].listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

								users[i].playing = false;
								users[i].playAgain = false;

								break;
							}
						}
					}

					cout << "Partida " << matchID << " terminada" << endl;

					games[matchID].isInUse = false;

				}
				else if (games[matchID].turnsLeft == 0)
				{
					// empate

					int playersFound = 0;

					for (int i = 0; i < users.size(); i++)
					{
						if (playersFound < 2)
						{
							if (users[i].ID == matchID && (users[i].p1 || users[i].p2))
							{
								string s = "1Empate! Si queres jugar de nuevo escribe 3, si no escribe 4";

								msgToClient = *((message*)s.c_str());
								sendto(users[i].listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

								users[i].playing = false;
								users[i].playAgain = false;

								playersFound++;
							}
						}
						else
						{
							break;
						}
					}
					games[matchID].isInUse = false;
				}
				else
				{
					NextTurn(users, games, tateti, matchID, msgToClient);
				}
			}
			else
			{
				msgToClient = *((message*)"1Espacio invalido, elija otro espacio por favor: ");
				sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
			}
			break;
		}
		case START_GAME:
		{
			int index = 0;
			int usersOut = 0;
			for (int i = 0; i < users.size(); i++)
			{
				if ((users[i].client.sin_addr.s_addr == client.sin_addr.s_addr) && (users[i].client.sin_port == client.sin_port))
				{
					users[i].playAgain = true;

					if (!users[i].playing)
					{
						users[i].playing = false;
						index = i;
						break;
					}
				}
			}

			for (int i = 0; i < users.size(); i++)
			{
				if (users[i].playAgain)
				{
					usersOut++;
				}
			}

			if (usersOut % 2 == 0 && usersOut != 0)
			{
				if (StartGame(games, tateti, users))
				{
					int matchID = users[index].ID;
					User* users2 = new User[2];

					int playersFound = 0;

					for (int i = 0; i < users.size(); i++)
					{
						if (playersFound < 2)
						{
							if (users[i].ID == matchID && users[i].playing)
							{
								users2[playersFound] = users[i];
								playersFound++;
							}
						}
						else
						{
							break;
						}
					}

					char ip01[1024];
					unsigned short port01 = users2[0].client.sin_port;
					inet_ntop(AF_INET, &users2[0].client.sin_addr, ip01, sizeof(ip01));
					char ip02[1024];
					unsigned short port02 = users2[1].client.sin_port;
					inet_ntop(AF_INET, &users2[1].client.sin_addr, ip02, sizeof(ip02));

					cout << "Partida " << matchID << " creada, Jugadores : " << users2[0].alias << " y " << users2[1].alias << endl;

					string test1 = "Jugando contra " + users2[1].alias;
					string test2 = "Jugando contra " + users2[0].alias;

					if ((users2[0].client.sin_addr.s_addr == client.sin_addr.s_addr) && (users2[0].client.sin_port == client.sin_port))
					{
						msgToClient.cmd = '0';
						strcpy_s(msgToClient.data, test1.c_str());
						sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
					}
					if ((users2[1].client.sin_addr.s_addr == client.sin_addr.s_addr) && (users2[1].client.sin_port == client.sin_port))
					{
						msgToClient.cmd = '0';
						strcpy_s(msgToClient.data, test2.c_str());
						sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
					}

					delete[] users2;

					int randomValue = randomInt(0, 1);
					tateti.SetNextPlayer(games[matchID], (bool)randomValue);
					DrawBoard(users, games, tateti, matchID, msgToClient);
					NextTurn(users, games, tateti, matchID, msgToClient);
				}
				else
				{
					msgToClient = *((message*)"0No hay jugadores suficientes, espere");
					sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
				}
			}
			else
			{
				msgToClient = *((message*)"0No hay jugadores suficientes, espere");
				sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));

				cout << users[index].alias << "(" << ip << ":" << port << ") ha entrado a la sala de espera." << endl;
			}
			break;
		}
		case EXIT:
			msgToClient = *((message*)"4");
			sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
			break;
		default:
			cout << "Byte invalido " << msgRcd.cmd << endl;
			msgToClient = *((message*)"1Byte invalido");
			sendto(listening, (char*)&msgToClient, sizeof(msgToClient), 0, (sockaddr*)&client, sizeof(client));
			break;
		}
	}
	
	//destruir el socket
	closesocket(listening);

	//cleanup winsock
	WSACleanup();

	return 0;
}

int GetEmptyGame(vector<Game>& games, TaTeTi& tateti)
{
	for (int i = 0; i < games.size(); i++)
	{
		if (!games[i].isInUse)
		{
			return games[i].ID;
		}
	}

	return -1;
}

bool StartGame(vector<Game>& games, TaTeTi& tateti, vector<User>& users)
{
	int player1Index = -1;
	int player2Index = -1;
	int currentID = -1;
	currentID = GetEmptyGame(games, tateti);

	if (currentID == -1)
	{
		games.push_back(Game());
		games[games.size() - 1].ID = matchID;
		games[games.size() - 1].isInUse = true;
		currentID = games[games.size() - 1].ID;
		matchID++;
		tateti.RestartCells(games[currentID]);
	}
	else
	{
		tateti.SetIsInUse(games[currentID], true);
		tateti.RestartCells(games[currentID]);
		tateti.SetTurns(games[currentID], 8);
		games[currentID].p1Joined = false;
		games[currentID].p2Joined = false;
	}

	for (int i = 0; i < users.size(); i++)
	{
		if (!users[i].playing && users[i].playAgain)
		{
			if (!games[currentID].p1Joined)
			{
				player1Index = i;
				users[i].ID = currentID;
				users[i].p1 = true;
				users[i].p2 = false;
				users[i].playing = true;
				users[i].playAgain = false;
				users[i].cellType = cross;
				games[currentID].p1Joined = true;
			}
			else if (!games[currentID].p2Joined)
			{
				player2Index = i;
				users[i].ID = currentID;
				users[i].p2 = true;
				users[i].p1 = false;
				users[i].playing = true;
				users[i].playAgain = false;
				users[i].cellType = dot;
				games[currentID].p2Joined = true;
				tateti.StartGame(games[currentID]);
				i = users.size();
			}
		}
	}

	if (!games[currentID].p1Joined || !games[currentID].p2Joined)
	{
		if (games[currentID].p1Joined)
		{
			users[player1Index].ID = -1;
			users[player1Index].p1 = false;
			users[player1Index].p2 = false;
			users[player1Index].playing = false;
			users[player1Index].playAgain = false;
			games[currentID].p1Joined = false;
		}
		if (games[currentID].p2Joined)
		{
			users[player2Index].ID = -1;
			users[player2Index].p1 = false;
			users[player2Index].p2 = false;
			users[player2Index].playing = false;
			users[player2Index].playAgain = false;
			games[currentID].p2Joined = false;
		}

		games[currentID].isInUse = false;

		games.pop_back();
		return false;
	}

	return true;
}

void DrawBoard(vector<User>& users, vector<Game>& games, TaTeTi& tateti, int id, message& msg)
{
	int playersFound = 0;

	for (int i = 0; i < users.size(); i++)
	{
		if (playersFound < 2)
		{
			if (users[i].ID == id && users[i].playing)
			{
				string s = "0" + tateti.GetCurrentGameBoard(games[id]);

				msg = *((message*)s.c_str());
				sendto(users[i].listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

				playersFound++;
			}
		}
		else
		{
			break;
		}
	}
}

void NextTurn(vector<User>& users, vector<Game>& games, TaTeTi& tateti, int id, message& msg)
{
	tateti.NextPlayer(games[id]);

	if (!games[id].nextPlayer)
	{
		string player1CellType = "";

		for (int i = 0; i < users.size(); i++)
		{
			if (users[i].ID == id && users[i].playing && users[i].p1)
			{
				string cellType(1, tateti.CellToChar(users[i].cellType));
				player1CellType = cellType;

				string s = "2Tu turno " + player1CellType + ". Para jugar seleccione un numero del 1 al 9.";
				msg = *((message*)s.c_str());
				sendto(users[i].listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

				break;
			}
		}

		for (int i = 0; i < users.size(); i++)
		{
			if (users[i].ID == id && users[i].playing && users[i].p2)
			{
				string s = "0Es turno de " + player1CellType;

				msg = *((message*)s.c_str());
				sendto(users[i].listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

				break;
			}
		}
	}
	else
	{
		string player2CellType = "";


		for (int i = 0; i < users.size(); i++)
		{
			if (users[i].ID == id && users[i].playing && users[i].p2)
			{
				string cellType(1, tateti.CellToChar(users[i].cellType));
				player2CellType = cellType;

				string s = "2Es su turno.Usted es " + player2CellType + ". Para jugar seleccione un numero del 1 al 9.";
				msg = *((message*)s.c_str());
				sendto(users[i].listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

				break;
			}
		}

		for (int i = 0; i < users.size(); i++)
		{
			if (users[i].ID == id && users[i].playing && users[i].p1)
			{
				string s = "0Es turno de " + player2CellType;

				msg = *((message*)s.c_str());
				sendto(users[i].listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&users[i].client, sizeof(users[i].client));

				break;
			}
		}
	}
}