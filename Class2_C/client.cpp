#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <Windows.h>
#include <map>
#include <conio.h>
#include "MessagePacket.h"
#include "PlayerData.h"

#include "SDL.h"

#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2.lib")


using namespace std;

#pragma comment(lib, "WS2_32.lib")

//		2			8		  4         4
//  Code	   SOCKET_ID      X         Y
//[][]    [][][][][][][][]  [][][][] [][][][]
//0       2                 10       14
char Data[18] = { 0, };

map<SOCKET, PlayerData*> PlayerList;

SOCKET MySocketID = 0L;

SDL_Window* MyWindow;
SDL_Renderer* MyRenderer;


void ProcessPacket(char* Packet)
{
	unsigned short Code = 0;
	memcpy(&Code, &Data[0], sizeof(Code));
	SOCKET FromID = 0;
	memcpy(&FromID, &Data[2], sizeof(FromID));

	Code = ntohs(Code);
	FromID = ntohll(FromID);
	PlayerData* NewPlayer = nullptr;
	switch ((MessagePacket)Code)
	{
		case MessagePacket::S2C_RegisterID:
//			cout << "MessagePacket::S2C_RegisterID " << endl;
			NewPlayer = new PlayerData();
			NewPlayer->MySocket = FromID;
			MySocketID = FromID;
			PlayerList[FromID] = NewPlayer;
		break;

		case MessagePacket::S2C_Spawn:
//			cout << "MessagePacket::S2C_Spawn " << endl;

			NewPlayer = new PlayerData();
			NewPlayer->MySocket = FromID;
			PlayerList[FromID] = NewPlayer;
		break;

		case MessagePacket::S2C_Destroy:
			PlayerList.erase(PlayerList.find(FromID));
		break;

		case MessagePacket::S2C_Move:
		{
			int X;
			int Y;
			memcpy(&X, &Data[10], sizeof(X));
			X = ntohl(X);
			memcpy(&Y, &Data[14], sizeof(Y));
			Y = ntohl(Y);

			//update PlayerList
			auto UpdatePlayer = PlayerList.find(FromID);
			UpdatePlayer->second->X = X;
			UpdatePlayer->second->Y = Y;
		}
		break;


		default:
			cout << "Not found code." << Code << endl;
		break;

	}

	//system("cls");
	//for (auto Player : PlayerList)
	//{
	//	COORD Cur;
	//	Cur.X = Player.second->X;
	//	Cur.Y = Player.second->Y;
	//	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
	//	cout << Player.second->MySocket << endl;
	////	cout << "Player ID : " << Player.second->MySocket << " : "
	//	//	<< Player.second->X << ", " << Player.second->Y << endl;
	//}


	SDL_SetRenderDrawColor(MyRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(MyRenderer);
	for (auto Player : PlayerList)
	{
		SDL_SetRenderDrawColor(MyRenderer, 0xff, 0x00, 0x00, 0x00);
		SDL_Rect MyRect = SDL_Rect{ Player.second->X, Player.second->Y,
			50, 50 };
		SDL_RenderFillRect(MyRenderer, &MyRect);
		//SDL_RenderCopy(MyRenderer, PlayerTexture, nullptr, &MyRect);
	}

	SDL_RenderPresent(MyRenderer);
}


unsigned WINAPI WorkThread(void* Arg)
{
	SOCKET ServerSocket;
	char Buffer[1024] = { 0, };

	ServerSocket = *(SOCKET*)Arg;

	while (true)
	{
		int RecvBytes = recv(ServerSocket, Data, sizeof(Data), 0);
		//cout << "RecvBytes : " << RecvBytes << endl;
		if (RecvBytes <= 0)
		{
			//서버랑 끊김
			break;
		}
		else
		{
			//패킷 처리(받은 자료 크기가 정확하다)
			ProcessPacket(Data);
		}
	}

	return 0;
}



int SDL_main(int agrc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	MyWindow = SDL_CreateWindow("Test", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	MyRenderer = SDL_CreateRenderer(MyWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = PF_INET;
	ServerSockAddr.sin_port = htons(4949);
	ServerSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE WorkHandle = (HANDLE)_beginthreadex(nullptr, 0, WorkThread, (void*)&ServerSocket, 0, nullptr);

	bool bIsRunning = true;

	while (bIsRunning)
	{
		//Input
		SDL_Event MyEvent;
		SDL_PollEvent(&MyEvent);
		if (MyEvent.type == SDL_QUIT)
		{
			bIsRunning = false;
		}
		else if (MyEvent.type == SDL_KEYDOWN)
		{

			map<SOCKET, PlayerData*>::iterator MyPlayer = PlayerList.find(MySocketID);
			switch (MyEvent.key.keysym.sym)
			{
			case SDLK_w:
				MyPlayer->second->Y--;
				break;

			case SDLK_s:
				MyPlayer->second->Y++;
				break;

			case SDLK_a:
				MyPlayer->second->X--;
				break;

			case SDLK_d:
				MyPlayer->second->X++;
				break;
			case SDLK_ESCAPE:
				bIsRunning = false;
				break;
			}

			unsigned short Code = htons((unsigned short)MessagePacket::C2S_Move);
			memcpy(&Data[0], &Code, sizeof(Code));
			SOCKET SendID = htonll(MyPlayer->second->MySocket);
			memcpy(&Data[2], &SendID, sizeof(SendID));
			int Temp = htonl(MyPlayer->second->X);
			memcpy(&Data[10], &Temp, sizeof(Temp));
			Temp = htonl(MyPlayer->second->Y);
			memcpy(&Data[14], &Temp, sizeof(Temp));

			int SentBytes = send(ServerSocket, Data, sizeof(Data), 0);
		}
	}

	closesocket(ServerSocket);

	SDL_DestroyRenderer(MyRenderer);
	SDL_DestroyWindow(MyWindow);
	SDL_Quit();

	WSACleanup();

	return 0;
}