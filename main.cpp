#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <ncurses.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

int nScreenWidth ;
int nScreenHeight;


float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159f / 4.0f;
float fDepth = 16.0f;

int main(){
	// So that we can use wide character array
	setlocale(LC_ALL,"");
	initscr(); // initialise the screen

	nodelay(stdscr,TRUE);
	keypad(stdscr, true); // initialise the keyboard: we can use arrows for directions

	noecho(); // user input is not displayed on the screen
	curs_set(0); // cursor symbol is not not displayed on the screen (Linux)

	// Our Game Map
	string map;
	map += "################";
	map += "#...#..........#";
	map += "#...#..........#";
	map += "#...#..........#";
	map += "#...#....#######";
	map += "#...#..........#";
	map += "##.............#";
	map += "####...........#";
	map += "#......#####..##";
	map += "#......#.......#";
	map += "#......#.......#";
	map += "#..............#";
	map += "#......#########";
	map += "#..............#";
	map += "#..............#";
	map += "################";
	
	// For Controlling Movement and turning mechanics so that it is independent of Frame Rate 
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();
	while(1)
	{
		getmaxyx(stdscr, nScreenHeight, nScreenWidth); // define dimensions of game window
		tp2 = chrono::system_clock::now();		
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Processing Input
		int KeyPressed = getch();
		switch(KeyPressed)
		{
			case ERR:
				break;
			case 'a':
				fPlayerA-= (0.4f)*15* fElapsedTime;
				break;
			case 'd':
				fPlayerA+= (0.4f)*15* fElapsedTime;
				break;
			case 'w':
				fPlayerX+= sinf(fPlayerA) * 15.0f * fElapsedTime;
				fPlayerY+= cosf(fPlayerA) * 15.0f * fElapsedTime;
				if(map[(int)fPlayerY*nMapWidth + (int)fPlayerX] == '#'){
					fPlayerX-= sinf(fPlayerA) * 15.0f * fElapsedTime;
					fPlayerY-= cosf(fPlayerA) * 15.0f * fElapsedTime;
				}
				break;
			case 's':
				fPlayerX-= sinf(fPlayerA) * 15.0f * fElapsedTime;
				fPlayerY-= cosf(fPlayerA) * 15.0f * fElapsedTime;
				if(map[(int)fPlayerY*nMapWidth + (int)fPlayerX] == '#'){
					fPlayerX+= sinf(fPlayerA) * 15.0f * fElapsedTime;
					fPlayerY+= cosf(fPlayerA) * 15.0f * fElapsedTime;
				}
				break;
			case 'q':
				fPlayerX-= cosf(fPlayerA) * 15.0f * fElapsedTime;
				fPlayerY+= sinf(fPlayerA) * 15.0f * fElapsedTime;
				if(map[(int)fPlayerY*nMapWidth + (int)fPlayerX] == '#'){
					fPlayerX+= sinf(fPlayerA) * 15.0f * fElapsedTime;
					fPlayerY-= cosf(fPlayerA) * 15.0f * fElapsedTime;
				}
				break;
			case 'e':
				fPlayerX+= cosf(fPlayerA) * 15.0f * fElapsedTime;
				fPlayerY-= sinf(fPlayerA) * 15.0f * fElapsedTime;
				if(map[(int)fPlayerY*nMapWidth + (int)fPlayerX] == '#'){
					fPlayerX-= cosf(fPlayerA) * 15.0f * fElapsedTime;
					fPlayerY+= sinf(fPlayerA) * 15.0f * fElapsedTime;
				}
				break;
			case 27:
				return 0;
			default:
				break;
		}
		for(int x=0 ; x <nScreenWidth; x++)
		{
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fStepSize = 0.01f;
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;
			bool bHitBoundary = false;

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while(!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;	

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if(nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test to see if the ray cell is a wall block
					if(	map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;
						vector<pair<float,float>> p;

						for(int tx = 0; tx <= 2; tx++){
							for(int ty = 0; ty <= 2; ty++){
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX+ tx - fPlayerX;
								float d = sqrt(vx*vx + vy*vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d,dot));
							}
							// Sort Pairs from closet 
							sort(p.begin(),p.end(), [](const pair<float, float> &left, const pair<float,float> &right){return left.first < right.first;});
							
							float fBound = 0.005;
							if(acos(p.at(0).second) < fBound) bHitBoundary = true;
							if(acos(p.at(1).second) < fBound) bHitBoundary = true;
							if(acos(p.at(2).second) < fBound) bHitBoundary = true;
						}
					}
				}
			}

			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// For Shading Walls so that further walls are Shaded lighter in comparison to nearer walls
			const char *shade;
			if (fDistanceToWall <= fDepth / 4.0f)
				shade = "\u2588";
			else if (fDistanceToWall < fDepth / 3.0f)
				shade = "\u2593";
			else if (fDistanceToWall < fDepth / 2.0f)
				shade = "\u2592";
			else if (fDistanceToWall < fDepth / 1.0f)
				shade = "\u2591";
			else
				shade = " ";

			if(bHitBoundary) shade = " ";
			for (int y = 0; y < nScreenHeight; y++){
				if(y < nCeiling)
					mvaddch(y, x,' ');
				else if (y >= nCeiling && y <= nFloor)
					mvprintw(y, x,shade);
				else{
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight/2.0f));
					char floorShade;
					if(b < 0.25) floorShade ='#';
					else if(b < 0.5) floorShade ='x';
					else if(b < 0.75) floorShade ='-';
					else if(b < 0.9) floorShade ='.';
					else floorShade = ' ';
					mvaddch(y, x,floorShade);
				}
			}
			for(int i=0; i<16; i++){
				for(int j=0; j<25; j++){
					mvprintw(i,j,"%c",map[i*16+j]);
				}
			}
			mvaddch((int)fPlayerY ,(int)fPlayerX,'P');
			mvaddch((int)fPlayerY + ceil(1.5*cosf(fPlayerA)),(int)fPlayerX + ceil(1.5*sinf(fPlayerA)),'@');
			mvprintw(0,0,"X=%f, Y=%f, A=%f, FPS=%f, WindowSize:%d,%d",fPlayerX,fPlayerY,fPlayerA,1.0f/fElapsedTime,nScreenWidth,nScreenHeight);
		}
		refresh();
	}
	curs_set(1); // cursor symbol is not not displayed on the screen (Linux)
	return 0;
}
