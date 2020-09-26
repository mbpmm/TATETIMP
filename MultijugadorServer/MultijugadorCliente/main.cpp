#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

#define NO_INPUT '0'
#define OTHER_OPTIONS '1'
#define GAMEPLAY '2'
#define EXIT '4'

using namespace std;

struct message
{
	byte cmd;
	char data[255];
};

void main()
{
	// inicializar winsock
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	if (WSAStartup(version, &data) == 0)
	{
		string ipToUse = "";
		int portToUse = 0;
		string alias = "";

		ipToUse = "127.0.0.1";
		portToUse = 8900;

		// indicar ip y puerto de destino
		sockaddr_in server;
		int serverSize = sizeof(server);
		server.sin_family = AF_INET; // AF_INET == IPV4
		server.sin_port = htons(portToUse); // puerto destino
		inet_pton(AF_INET, ipToUse.c_str(), &server.sin_addr); // direccion ip destino		

		// crear socket, UDP
		SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);
		if (out == INVALID_SOCKET)
		{
			cout << "Invalid socket: " << WSAGetLastError() << endl;

			return;
		}
		else
		{
			cout << "Bienvenido al server de TaTeTi!" << endl;
		}

		byte cmd = '0';
		// enviar data a traves del socket
		string msgtest;
		message msgRcd;
		message msg;
		//getline(cin, msgtest);

		char buf[1024];
		memset(buf, 0, sizeof(buf));
		memset(&msg, 0, sizeof(msg));
		memset(&msgRcd, 0, sizeof(msgRcd));
		cout << "Ingrese su alias:";
		getline(cin, msgtest);
		cout << endl;
		msg.cmd = '1';
		strcpy_s(msg.data, msgtest.c_str());

		sendto(out, (char*)&msg, sizeof(msg), 0, (sockaddr*)&server, sizeof(server));
		recvfrom(out, (char*)&msgRcd, sizeof(msgRcd), 0, (sockaddr*)&server, &serverSize);
		cout << msgRcd.data << endl;
		cout << endl;
		cmd = msgRcd.cmd;

		do
		{
			memset(buf, 0, sizeof(buf));
			memset(&msg, 0, sizeof(msg));
			memset(&msgRcd, 0, sizeof(msgRcd));

			switch (cmd)
			{
			case NO_INPUT:
			{
				//other player turn
				//no input
				//wait
			}
			break;
			case OTHER_OPTIONS:
			{
				cout << "Escribe el mensaje a mandar: ";
				getline(cin, msgtest);
				cout << endl;
				msg = *((message*)msgtest.c_str());
				sendto(out, (char*)&msg, sizeof(msg), 0, (sockaddr*)&server, sizeof(server));
			}
			break;
			case GAMEPLAY:
			{
				cout << "Escribe el mensaje a mandar: ";
				getline(cin, msgtest);
				cout << endl;
				strcpy_s(msg.data, msgtest.c_str());
				msg.cmd = '2';
				sendto(out, (char*)&msg, sizeof(msg), 0, (sockaddr*)&server, sizeof(server));
			}
			break;
			default:
				break;
			}


			int bytesIn = recvfrom(out, (char*)&msgRcd, sizeof(msgRcd), 0, (sockaddr*)&server, &serverSize);

			if (bytesIn == SOCKET_ERROR)
			{
				cerr << "Error al recibir data" << endl;
			}

			cout << msgRcd.data << endl;
			cout << endl;
			cmd = msgRcd.cmd;
		} while (cmd != EXIT);
		// cerrar el socket
		closesocket(out);
	}
	else
	{
		return;
	}

	fflush(stdin);

	// cerrar winsock
	WSACleanup();
}