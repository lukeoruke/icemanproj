#include "StudentWorld.h"
#include <string>
using namespace std;
#include <memory>

const int TUNNEL_COL_START = 30;
const int TUNNEL_COL_END = 33;
const int TUNNEL_ROW = 4;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

void StudentWorld::createIce() {
	//ask mr parker why he did -4????
	for (int row = 0; row < MAX_ROW; row++) {
		for (int col = 0; col < MAX_COL; col++) {
			if (row < TUNNEL_COL_START || row > TUNNEL_COL_END || col < TUNNEL_ROW) {
				iceContainer[row][col] = std::unique_ptr<Ice>(new Ice(row, col));
			}
		}
	}

}

void StudentWorld::createPlayer() {
	player = new Iceman();
}