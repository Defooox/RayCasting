#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <Windows.h>

using namespace std;



int ScreenWidth = 120;		
int ScreenHeight = 40;		

int MapWidth = 16;			
int MapHeight = 16;		

float PlayerX = 14.7f;		
float PlayerY = 5.09f;		
float PlayerA = 0.0f;		//направление игрока	
float FOV = 3.14159f / 4.0f;	
float Depth = 16.0f;			//дальность обзора
float Speed = 4.0f;			

int main()
{

	wchar_t* screen = new wchar_t[ScreenWidth * ScreenHeight];// Массив для записи в буфер
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);// Буфер экрана
	SetConsoleActiveScreenBuffer(hConsole);// Настройка консоли
	DWORD dwBytesWritten = 0;// для дебага
	
	wstring map;
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1)
	{
		
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
		// Проверка нажатия клавиш и обновление координат игрока
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			PlayerA -= (Speed * 0.75f) * fElapsedTime;


		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			PlayerA += (Speed * 0.75f) * fElapsedTime;

		
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			PlayerX += sinf(PlayerA) * Speed * fElapsedTime;;
			PlayerY += cosf(PlayerA) * Speed * fElapsedTime;;
			if (map.c_str()[(int)PlayerX * MapWidth + (int)PlayerY] == '#') // Если столкнулись со стеной, но откатываем шаг
			{
				PlayerX -= sinf(PlayerA) * Speed * fElapsedTime;;
				PlayerY -= cosf(PlayerA) * Speed * fElapsedTime;;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			PlayerX -= sinf(PlayerA) * Speed * fElapsedTime;;
			PlayerY -= cosf(PlayerA) * Speed * fElapsedTime;;
			if (map.c_str()[(int)PlayerX * MapWidth + (int)PlayerY] == '#')// Если столкнулись со стеной, но откатываем шаг
			{
				PlayerX += sinf(PlayerA) * Speed * fElapsedTime;;
				PlayerY += cosf(PlayerA) * Speed * fElapsedTime;;
			}
		}

		for (int x = 0; x < ScreenWidth; x++) 
		{
			// Вычисление направления луча и установка шага и расстояния до стены
			float fRayAngle = (PlayerA - FOV / 2.0f) + ((float)x / (float)ScreenWidth) * FOV; // Направление луча


			float fStepSize = 0.1f;		  									
			float fDistanceToWall = 0.0f; 

			bool bHitWall = false;		
			bool bBoundary = false;		

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);


			// Цикл проверки столкновений с препятствиями
			while (!bHitWall && fDistanceToWall < Depth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(PlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(PlayerY + fEyeY * fDistanceToWall);

				// Проверка выхода за границы карты
				if (nTestX < 0 || nTestX >= MapWidth || nTestY < 0 || nTestY >= MapHeight)
				{
					bHitWall = true;			
					fDistanceToWall = Depth;
				}
				else
				{
					// Проверка столкновения с препятствием
					if (map.c_str()[nTestX * MapWidth + nTestY] == '#')
					{
						
						bHitWall = true;

						vector<pair<float, float>> p;

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								
								float vy = (float)nTestY + ty - PlayerY;
								float vx = (float)nTestX + tx - PlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

			
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			
			int nCeiling = (float)(ScreenHeight / 2.0) - ScreenHeight / ((float)fDistanceToWall); //Вычисляем координаты начала и конца стенки по формулам
			int nFloor = ScreenHeight - nCeiling;

	
			short nShade = ' ';
			if (fDistanceToWall <= Depth / 4.0f)			nShade = 0x2588;	// Если стенка близко, то рисуем 		
			else if (fDistanceToWall < Depth / 3.0f)		nShade = 0x2593;	// светлую полоску	
			else if (fDistanceToWall < Depth / 2.0f)		nShade = 0x2592;	// Для отдалённых участков 
			else if (fDistanceToWall < Depth)				nShade = 0x2591;	// рисуем более темную
			else											nShade = ' ';		

			if (bBoundary)		nShade = ' '; 

			for (int y = 0; y < ScreenHeight; y++)
			{
			
				if (y <= nCeiling)
					screen[y * ScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * ScreenWidth + x] = nShade;
				else 
				{
				
					float b = 1.0f - (((float)y - ScreenHeight / 2.0f) / ((float)ScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y * ScreenWidth + x] = nShade;
				} 
			}
		}

		
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", PlayerX, PlayerY, PlayerA, 1.0f / fElapsedTime);


		for (int nx = 0; nx < MapWidth; nx++)
			for (int ny = 0; ny < MapWidth; ny++)
			{
				screen[(ny + 1) * ScreenWidth + nx] = map[ny * MapWidth + nx];
			}
		screen[((int)PlayerX + 1) * ScreenWidth + (int)PlayerY] = 'P';


		screen[ScreenWidth * ScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten); // Запись содержимого массива screen в экран консоли
	}

	return 0;
}