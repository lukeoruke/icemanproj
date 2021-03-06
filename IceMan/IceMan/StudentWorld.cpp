#include "StudentWorld.h"
#include "GameController.h"
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <future>
using namespace std;

const int TUNNEL_COL_START = 30;
const int TUNNEL_COL_END = 33;
const int TUNNEL_ROW = 4;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

void StudentWorld::createNewItem() {  //21
	int g = static_cast<int>(getLevel() * 25 + 300);
	if (rand() % g == 0) {  //ranges 1 to g; this should mean a 1/g change of running
		if ((rand() % g + 1) <= (g / 5)) {  //this should be 1/5, create sonar
			createSonar();
		}
		else { //rest is 4/5, create water
			createWater();
		}
	}
	return;
}

void StudentWorld::createNewProtester()
{
	if (m_ticksLastProtester == 0 && getProtestersLeft() < lvlProtestors())
	{
		m_ticksLastProtester = getProtesterTick();
		int probabilityOfHardcore = min(90, static_cast<int>(getLevel()) * 10 + 30);
		if (rand() % 100 <= probabilityOfHardcore) {
			createHProtester();
		}
		else
			createProtester();

	}
	if (m_ticksLastProtester > 0)
		m_ticksLastProtester--;
}

int StudentWorld::getSonarWaterTick() {
	return max(100, (300 - 10 * static_cast<int>(getLevel())));
}

int StudentWorld::getProtesterTick()
{
	return max(25, 200 - static_cast<int>(getLevel()));
}

//pg 19
int StudentWorld::move() {
	// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
	// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.

	setDisplayText();
	player->doSomething();

	if (player->getIsAlive()) {
		createNewItem();
		createNewProtester();

		if (player->getOilAmnt() < lvlOil()) {   //checks if player got the oil amnt
												 //then do something for all actors in a for loop

			for (size_t i = 0; i < actors.size(); i++)  //go thorough the actor vector and trigger its do something
			{
				if (actors.at(i) != nullptr)
					actors.at(i)->doSomething();

			}
			if (player->getIsAlive() == false)
			{
				decLives();
				m_ticksLastProtester = 0;
				return GWSTATUS_PLAYER_DIED;
			}

		}
		else {
			GameController::getInstance().playSound(SOUND_FINISHED_LEVEL);
			m_ticksLastProtester = 0;
			return GWSTATUS_FINISHED_LEVEL;
		}
		removeDead();
	}

	else {
		decLives();
		m_ticksLastProtester = 0;
		return GWSTATUS_PLAYER_DIED;
	}

	return GWSTATUS_CONTINUE_GAME;
}


// check if we have to use the respecitve Destructors aka  ~
void StudentWorld::cleanUp() {
	player.reset();
	for (int row = 0; row < MAX_WINDOW; row++) {
		for (int col = 0; col < MAX_WINDOW - 4; col++) {
			if (row < TUNNEL_COL_START || row > TUNNEL_COL_END || col < TUNNEL_ROW) {
				iceContainer[row][col].reset();
			}
		}
	}
	std::vector<unique_ptr<Actor>>::iterator it = actors.begin();
	for (; it != actors.end(); it++)
	{
		delete (*it).release();
	}
	actors.clear();
}

StudentWorld::~StudentWorld() {
	cleanUp();
}
//not used yet
StudentWorld* StudentWorld::getStudentWorld() {
	return this;
}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////GLOBAL FUNCTIONS(DISTANCE, RAND NUM GEN, REMOVE DEAD, EXC)//////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::removeDead() {
	if (actors.size() == 0)
		return;

	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	actors.erase(std::remove_if(it, actors.end(), [&](unique_ptr<Actor>& upAct)-> bool
		{return (upAct->getIsAlive() == false); }),
		actors.end());

	return;
}
//generates a random x value
int StudentWorld::generateRandX() {
	return (rand() % 60);
}
//generates a random y value
int StudentWorld::generateRandY() {
	return (rand() % 56);
}

void StudentWorld::overlap(const Actor& a) {

	int playerX = a.getX();
	int playerY = a.getY();

	for (int x = playerX; x < playerX + 4; x++) {
		for (int y = playerY; y < playerY + 4; y++) {
			if (iceContainer[x][y] != nullptr) {
				deleteIce(x, y);
				if (a.getID() == IID_PLAYER)
					GameController::getInstance().playSound(SOUND_DIG);
			}
		}
	}
}

bool StudentWorld::overlapAt(int x, int y) {


	for (int i = x; i < x + 4; i++) {
		for (int j = y; j < y + 4; j++) {
			if (iceContainer[i][j]) {
				return true;
			}
		}
	}
	return false;
}
//returns true is distance between actors is far enough
bool StudentWorld::distance(int x, int y) {
	if (actors.size() == 0) {   //temp
		return true;
	}
	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.end(); it++) {
		double d = (sqrt(pow(x - (*it)->getX(), 2) + pow(y - (*it)->getY(), 2)));
		if (d <= 6) {
			return false;
		}
	}
	return true;
}

double StudentWorld::radius(int x1, int y1, int x2, int y2) {
	double d = (sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
	return d;
}


bool StudentWorld::annoyNearbyPeople(const Actor& a, unsigned int hp)
{
	bool ans = false;
	if (radius(player->getX(), player->getY(), a.getX(), a.getY()) <= 3) {
		if (player->annoy(hp))
			ans = true;
	}
	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.end(); it++) {
		if (radius(a.getX(), a.getY(), (*it)->getX(), (*it)->getY()) <= 3) {
			if ((*it)->annoy(hp))
				ans = true;
		}
	}
	return ans;
}

bool StudentWorld::protesterFoundGold(const Actor& a) {

	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.end(); it++) {
		if (radius(a.getX(), a.getY(), (*it)->getX(), (*it)->getY()) <= 3) {
			if ((*it)->getID() == IID_PROTESTER) {
				if (dynamic_cast<RegularProtester*>((*it).get())->getIsLeaving() == false) {
					dynamic_cast<RegularProtester*>((*it).get())->gainGold();
					increaseScore(25);
					return true;
				}
			}
			if ((*it)->getID() == IID_HARD_CORE_PROTESTER) {
				if (dynamic_cast<HardcoreProtester*>((*it).get())->getIsLeaving() == false) {
					increaseScore(50);
					dynamic_cast<HardcoreProtester*>((*it).get())->gainGold();
					return true;
				}
			}
		}
	}
	return false;
}
void StudentWorld::annoyIceman(unsigned int hp)
{
	player->annoy(hp);
}
bool StudentWorld::isRoomInFront(const Actor& a) {
	bool ans = false;
	int x = a.getX();
	int y = a.getY();
	switch (a.getDirection()) {
	case GraphObject::Direction::left:
		if (!overlapAt(x - 4, y))
			ans = true;
		break;
	case GraphObject::Direction::right:
		if (!overlapAt(x + 4, y))
			ans = true;
		break;
	case GraphObject::Direction::down:
		if (!overlapAt(x, y - 4))
			ans = true;
		break;
	case GraphObject::Direction::up:
		if (!overlapAt(x, y + 4))
			ans = true;
		break;
	default:
		return false;
	}
	return ans;
}
// x and y are the coord of whatever object is calling the fucntion
// r is the specific radius specifed by the object
bool StudentWorld::icemanNearby(const Actor& a, int x, int y, double r) {
	int playerX = player->getX();
	int playerY = player->getY();

	if (radius(playerX, playerY, x, y) <= r) {
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ICE//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createIce() {
	for (int row = 0; row < MAX_WINDOW; row++) {
		for (int col = 0; col < MAX_WINDOW - 4; col++) {
			if (row < TUNNEL_COL_START || row > TUNNEL_COL_END || col < TUNNEL_ROW) {
				iceContainer[row][col] = std::unique_ptr<Ice>(new Ice(row, col));
			}
		}
	}
}
//checks whether the pixel around it is a ice block
bool StudentWorld::iceInFront(const Actor& a) {
	int x = a.getX();
	int y = a.getY();
	switch (a.getDirection()) {
	case GraphObject::Direction::left:
		for (int i = y; i < y + 4; i++)
		{
			if (iceContainer[x - 1][i])
				return true;
		}
		break;
	case GraphObject::Direction::right:
		for (int i = y; i < y + 4; i++)
		{
			if (iceContainer[x + 4][i])
				return true;
		}
		break;
	case GraphObject::Direction::down:
		for (int i = x; i < x + 4; i++)
		{
			if (iceContainer[i][y - 1])
				return true;
		}
		break;
	case GraphObject::Direction::up:
		for (int i = x; i < x + 4; i++)
		{
			if (iceContainer[i][y + 4])
				return true;
		}
		break;
	default:
		return false;
	}
	return false;
}

//deletes ice block of a specified cooridnate
void StudentWorld::deleteIce(int x, int y) {
	iceContainer[x][y].reset();
	iceContainer[x][y] = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ICEMAN///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createPlayer() {
	player = std::unique_ptr<Iceman>(new Iceman(this));
}
void StudentWorld::incIcemanItem(char x) {
	switch (x) {
	case 'g':
		player->gainGold();
		break;
	case 'o':
		player->gainOilIceman();
		break;
	case 's':
		player->gainSonarIceman();
		break;
	case 'w':
		player->gainWaterIceman();
		break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////PROTESTER///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createProtester() {
	unique_ptr<RegularProtester> protester;
	protester = unique_ptr<RegularProtester>(new RegularProtester(this, 60, 60));
	actors.emplace_back(std::move(protester));
}
void StudentWorld::createHProtester() {
	unique_ptr<HardcoreProtester> protester;
	protester = unique_ptr<HardcoreProtester>(new HardcoreProtester(this, 60, 60));
	actors.emplace_back(std::move(protester));
}

int StudentWorld::lvlProtestors()
{
	return min(15, static_cast<int>(2 + static_cast<int>(getLevel()) * 1.5));
}

bool StudentWorld::icemanInSight(int x, int y) {
	if (player->getX() == x || player->getY() == y) {
		return true;
	}
	else {
		return false;
	}
}
double StudentWorld::protesterRadius(int x, int y) {
	double d = (sqrt(pow(player->getX() - x, 2) + pow(player->getY() - y, 2)));
	return d;
}
GraphObject::Direction StudentWorld::getIcemanDirection() {
	GraphObject::Direction x = player->getDirection();
	return x;
}
GraphObject::Direction StudentWorld::faceIceman(int x, int y) {
	if (player->getX() < x) {
		return GraphObject::left;
	}
	if (player->getX() > x) {
		return GraphObject::right;
	}
	if (player->getY() < y) {
		return GraphObject::down;
	}
	if (player->getY() > y) {
		return GraphObject::up;
	}
}
bool StudentWorld::isFacingIceman(const Actor& a)
{
	bool ans;
	int xActor = a.getX();
	int yActor = a.getY();
	int xPlayer = player->getX();
	int yPlayer = player->getY();
	switch (a.getDirection()) {
	case GraphObject::Direction::left:
		if ((xActor == xPlayer + 4) && ((yActor >= yPlayer - 3) && yActor <= yPlayer + 3))
			ans = true;
		break;
	case GraphObject::Direction::right:
		if ((xActor == xPlayer - 4) && ((yActor >= yPlayer - 3) && yActor <= yPlayer + 3))
			ans = true;
		break;
	case GraphObject::Direction::down:
		if ((yActor == yPlayer + 4) && ((xActor >= xPlayer - 3) && xActor <= xPlayer + 3))
			ans = true;
		break;
	case GraphObject::Direction::up:
		if ((yActor == yPlayer - 4) && ((xActor >= xPlayer - 3) && xActor <= xPlayer + 3))
			ans = true;
		break;
	default:
		return false;
	}
}
bool StudentWorld::canReachIceman(int x, int y) { //TODO:: cannot figure out the boulder issue
	int px = player->getX();
	int py = player->getY();
	if (px == x) {  //if the protester and iceman have the same X coord
		if (py > y) { //if the player is higher then the protester
			int startY = y;
			while (startY < py) {
				if (iceContainer[x][startY] || iceContainer[x + 1][startY] || iceContainer[x + 2][startY] || iceContainer[x + 3][startY])   // TODO:: account for boulders
				{
					return false;
				}
				startY++;
			}
			return true;
		}
		else {
			int startY = py;
			while (startY < y) {
				if (iceContainer[x][startY] || iceContainer[x + 1][startY] || iceContainer[x + 2][startY] || iceContainer[x + 3][startY]) { // TODO:: account for boulders
					return false;
				}
				startY++;
			}
			return true;
		}

	}
	//other half
	if (py == y) {//if the protester and iceman have the same Y coord
		if (px > x) {
			int startX = x;
			while (startX < px) {
				if (iceContainer[startX][y] || iceContainer[startX][y + 1] || iceContainer[startX][y + 2] || iceContainer[startX][y + 3]) {
					return false;
				}
				startX++;
			}
			return true;
		}
		else {
			int startX = px;
			while (startX < x) {
				if (iceContainer[startX][y] || iceContainer[startX][y + 1] || iceContainer[startX][y + 2] || iceContainer[startX][y + 3]) {
					return false;
				}
				startX++;
			}
			return true;
		}
	}
}
bool StudentWorld::canTurn(int x, int y, GraphObject::Direction r) {
	int e = player->getX();
	int q = player->getY();

	switch (r) {
	case GraphObject::up:
		if (x + 1 == 61) { //if the protester is to the very right
			if ((!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) {
				return true;
			}
			else {
				return false;
			}
		}
		if (x - 1 == -1) { // if the protester is to the very left
			if ((!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3])) {
				return true;
			}
			else {
				return false;
			}
		}
		//if there is not ice to the right or left of protester
		if (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3]) { //no ice to the right
			return true;
		}
		if ((!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) { //no ice to the left
			return true;
		}
		else {
			return false;
		}
		break;
	case GraphObject::down: //FIXEDS
		if (x + 1 == 61) { //if the protester is to the very right
			if (!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3]) {
				return true;
			}
			else {
				return false;
			}
		}
		if (x - 1 == -1) { // if the protester is to the very left
			if ((!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3])) {
				return true;
			}
			else {
				return false;
			}
		}
		//if there is no ice to the right or left of the protester return true
		if (!(y + 3 > 60)) { //starts counting from 57
			if (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3]) { // no ice to the right 
				return true;
			}
			if ((!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) { // no ice to the left
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
		break;
	case GraphObject::left:
		if (y + 1 == 61) { //if the protester is at max height of the game
			if ((!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1])) {   //checks below
				return true;
			}
			else {
				return false;
			}
		}
		if (y - 1 == -1) {  //if the protester is at min height of the game
			if ((!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4])) { //checks above
				return true;
			}
			else {
				return false;
			}
		}
		//if there is not ice to the top or bottom of protester TODO:: Check if these are right
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1]) { //no ice below him
			return true;
		}
		if (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4]) { //no ice above him
			return true;
		}
		else {
			return false;
		}
		break;
	case GraphObject::right:
		if (y + 1 == 61) { //if the protester is at max height of the game
			if ((!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1])) { //checks below
				return true;
			}
			else {
				return false;
			}
		}
		if (y - 1 == -1) { //if the protester is at min height of the game
			if (!iceContainer[x][y + 1] && !iceContainer[x + 3][y + 1]) { //checks above him
				return true;
			}
			else {
				return false;
			}
		}
		//if there is not ice to the top or bottom of protester
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1]) { //no ice below him
			return true;
		}
		if (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4]) {
			return true;
		}
		else {
			return false;
		}
		break;
	}

}
GraphObject::Direction StudentWorld::makeTurn(int x, int y, GraphObject::Direction r) {
	int choice = rand() % 2;
	switch (r) {
	case GraphObject::up:
		if (x + 1 == 61) { //checks the left
			if ((!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) {
				return GraphObject::left;
			}
		}
		if (x - 1 == -1) { // checkst the right
			if ((!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3])) {
				return GraphObject::right;
			}
		}
		if (!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3] && (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3])) {
			if (choice == 0) {
				return GraphObject::left;
			}
			if (choice == 1) {
				return GraphObject::right;
			}
		}
		if (!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3]) {
			return GraphObject::left;
		}
		if (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3]) {
			return GraphObject::right;
		}
		break;
	case GraphObject::down:
		if (x + 1 == 61) { //if the protester is to the very right
			if (!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3]) {
				return GraphObject::left;
			}
		}
		if (x - 1 == -1) { // if the protester is to the very left
			if ((!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3])) {
				return GraphObject::right;
			}
		}
		//if there is no ice to the right or left of the protester return 50/50
		if (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3] && (!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) {
			if (choice == 0) {
				return GraphObject::left;
			}
			if (choice == 1) {
				return GraphObject::right;
			}
		}
		//if there is no ice to the right
		if (!iceContainer[x + 4][y] && !iceContainer[x + 4][y + 3]) {
			return GraphObject::right;
		}
		//if there is no ice to the left
		if ((!iceContainer[x - 1][y] && !iceContainer[x - 1][y + 3])) {
			return GraphObject::left;
		}
		break;
	case GraphObject::left:
		if (y + 1 == 61) {
			if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1]) { //checks below
				return GraphObject::down;
			}
		}
		if (y - 1 == -1) {
			if ((!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4])) { //chekcs above
				return GraphObject::up;
			}
		}
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1] && (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4])) { // if both up/down are open,pick one
			if (choice == 0) {
				return GraphObject::up;
			}
			if (choice == 1) {
				return GraphObject::down;
			}
		}
		//if only above or below is open, pick accordingly
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1]) {
			GraphObject::down;
		}
		if ((!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4])) {
			return GraphObject::up;
		}
		break;
	case GraphObject::right:
		if (y + 1 == 61) {
			if ((!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1])) {
				return GraphObject::down;
			}
		}
		if (y - 1 == -1) {
			if (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4]) {
				return GraphObject::up;
			}
		}
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1] && (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4])) {
			if (choice == 0) {
				return GraphObject::up;
			}
			if (choice == 1) {
				return GraphObject::down;
			}
		}
		if (!iceContainer[x][y - 1] && !iceContainer[x + 3][y - 1]) {
			return GraphObject::down;
		}
		if (!iceContainer[x][y + 4] && !iceContainer[x + 3][y + 4]) {
			return GraphObject::up;
		}
		break;
	}
}
gridQueue::gridQueue(int x, int y) {
	m_x = x;
	m_y = y;
}
int gridQueue::getGridX() {
	return m_x;
}
int gridQueue::getGridY() {
	return m_y;
}
void StudentWorld::createGrid() {
	for (int row = 0; row <= 60; row++) {
		for (int col = 0; col <= 60; col++) {
			//if (row < TUNNEL_COL_START || row > TUNNEL_COL_END || col < TUNNEL_ROW) {
			if (iceContainer[row][col].get() != nullptr) { //if there is ice or boulder, it is unreachable
				grid[row][col] = 9999999;
			}
			else {
				grid[row][col] = 1000; //if there is open space
			}
			//}
		}
	}
	//testing one
	//for (int row = 0; row < 60; row++) {
	//	for (int col = 0; col < 60; col++) {
	//		if (iceContainer[row][col].get() == nullptr)
	//			std::cout << row << "," << col << endl;

	//	}
	//}
}
int StudentWorld::findPath(int proX, int proY) {
	createGrid();
	cout << player->getX() << endl;
	cout << player->getY() << endl;

	tree.push(gridQueue(60, 60));
	int distance = 0;

	while (!tree.empty()) {
		gridQueue guess = tree.front();
		tree.pop();
		int row = guess.getGridX();
		int col = guess.getGridY();
		if (row == proX && col == proY) {
			tree.empty();
			distance--;
			//cout << "DISTANCE IS " << distance << endl;
			return distance;
		}
		//if (row != proX &&  col != proY) {
		if (grid[row][col] == 1000) { //current point
			grid[row][col] = distance;
			distance++;
		}
		if (grid[row][col + 1] == 1000) {  //above the point
			grid[row][col + 1] = distance;
			tree.push(gridQueue(row, col + 1));
		}
		if (grid[row + 1][col] == 1000) {  //right the point
			grid[row + 1][col] = distance;
			tree.push(gridQueue(row + 1, col));
		}
		if (grid[row][col - 1] == 1000) {   //below the point
			grid[row][col - 1] = distance;
			tree.push(gridQueue(row, col - 1));
		}
		if (grid[row - 1][col] == 1000) {  //left of point
			grid[row - 1][col] = distance;
			tree.push(gridQueue(row - 1, col));
		}
		distance++;

		//}
	}
	//cout << "2nd distance display " << distance << endl;
	return distance;

}
GraphObject::Direction StudentWorld::pickPath(int proX, int proY, int distance) {
	//EX the distance is 6
	//while (distance != 0) {
	if (grid[proX][proY + 1] == distance - 1) { //up
		stepsToLeave.push_back(GraphObject::up);
		return GraphObject::up;
	}
	if (grid[proX + 1][proY] == distance - 1) { //right
		stepsToLeave.push_back(GraphObject::right);
		return GraphObject::right;
	}
	if (grid[proX][proY - 1] == distance - 1) {
		stepsToLeave.push_back(GraphObject::down);
		return GraphObject::down;
	}
	if (grid[proX - 1][proY] == distance - 1) {
		stepsToLeave.push_back(GraphObject::left);
		return GraphObject::left;
	}
	//}

}
std::vector<GraphObject::Direction> StudentWorld::leaveField(int proX, int proY) {
	//use threads to search multiple places at once
	//vector<int> options;
	//for (int r = 0; r < 4; r++) {
	//	auto ft = async(launch::async, [&] {return findPath(proX,proY); });
	//	int steps = ft.get();
	//	options.push_back(steps);
	//}

	int j = proX;
	int k = proY;
	if (stepsToLeave.empty()) {
		//findPath(proX, proY); //sets the distance from protester to exit			leaveField																			  //|
		int distance = StudentWorld::findPath(proX, proY);  // returns the shortes distance 
		while (distance != 0) {
			GraphObject::Direction q = pickPath(j, k, distance);
			if (q == GraphObject::up) {
				k++;
			}
			if (q == GraphObject::right) {
				j++;
			}
			if (q == GraphObject::down) {
				k--;
			}
			if (q == GraphObject::left) {
				j--;
			}
			distance--;
		}
	}

	return stepsToLeave;  // returns a vector that has the fastest path to return to exit
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////SQUIRT///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createSquirt(Iceman& man) {
	int x = man.getX();
	int y = man.getY();
	switch (man.getDirection()) {
	case GraphObject::Direction::left:
		x = x - 4;
		break;
	case GraphObject::Direction::right:
		x = x + 4;
		break;
	case GraphObject::Direction::down:
		y = y - 4;
		break;
	case GraphObject::Direction::up:
		y = y + 4;
		break;
	}
	actors.emplace_back(unique_ptr<Squirt>(new Squirt(this, x, y, man.getDirection())));
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////BOULDER//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int StudentWorld::lvlBoulder() {
	return min(static_cast<int>(getLevel()) / 2 + 2, 9);
}

void StudentWorld::createBoulder(int create) {
	for (int k = 0; k < create; k++) {
		int x = 0;
		int y = 0;
		do {
			do {
				x = generateRandX();
			} while (x > 26 && x < 34);

			do {
				y = generateRandY();
			} while (y < 20);


		} while (!distance(x, y));

		unique_ptr<Boulder> boulder;
		boulder = unique_ptr<Boulder>(new Boulder(this, x, y));
		overlap(*boulder);
		actors.emplace_back(std::move(boulder));
	}

}
// parses through actor vector and finds boulders
bool StudentWorld::boulderInFront(const Actor& a)
{
	bool ans = false;
	int xActor = a.getX();
	int yActor = a.getY();
	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.begin() + getBouldersLeft(); ++it)
	{

		int xBoulder = (*it)->getX();
		int yBoulder = (*it)->getY();
		switch (a.getDirection()) {
		case GraphObject::Direction::left:
			if ((xActor == xBoulder + 4) && ((yActor >= yBoulder - 3) && yActor <= yBoulder + 3))
				ans = true;
			break;
		case GraphObject::Direction::right:
			if ((xActor == xBoulder - 4) && ((yActor >= yBoulder - 3) && yActor <= yBoulder + 3))
				ans = true;
			break;
		case GraphObject::Direction::down:
			if ((yActor == yBoulder + 4) && ((xActor >= xBoulder - 3) && xActor <= xBoulder + 3))
				ans = true;
			break;
		case GraphObject::Direction::up:
			if ((yActor == yBoulder - 4) && ((xActor >= xBoulder - 3) && xActor <= xBoulder + 3))
				ans = true;
			break;
		default:
			return false;
		}
	}
	return ans;
}
int StudentWorld::getBouldersLeft() const
{
	return m_bouldersLeft;
}

void StudentWorld::decBouldersLeft()
{
	m_bouldersLeft--;
}

void StudentWorld::incBouldersLeft()
{
	m_bouldersLeft++;
}
//we used this function
bool StudentWorld::boulderInTheWay(const Actor& a, int where)
{
	bool ans = false;
	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.begin() + getBouldersLeft(); ++it)
	{
		int xActor = a.getX();
		int yActor = a.getY();
		int xBoulder = (*it)->getX();
		int yBoulder = (*it)->getY();
		switch (a.getDirection()) {
		case GraphObject::Direction::left:
			xActor = xActor - where;
			break;
		case GraphObject::Direction::right:
			xActor = xActor + where;
			break;
		case GraphObject::Direction::down:
			yActor = yActor - where;
			break;
		case GraphObject::Direction::up:
			yActor = yActor + where;
			break;
		default:
			return false;
		}
		if (radius(xActor, yActor, xBoulder, yBoulder) <= 3)
			ans = true;
	}

	return ans;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////GOLD/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int StudentWorld::lvlGold() {
	return max(5 - static_cast<int>(getLevel()) / 2, 2);
}
void StudentWorld::createGold(int num)
{
	for (int k = 0; k < num; k++) {
		int x = 0;
		int y = 0;
		do {
			do {
				x = generateRandX();
			} while (x > 26 && x < 34);

			y = generateRandY();

		} while (!distance(x, y));
		unique_ptr<Gold> gold;
		gold = unique_ptr<Gold>(new Gold(this, x, y, false, true, false)); //false to make gold invisible when created
		actors.emplace_back(std::move(gold));
	}
}

int StudentWorld::getGoldLeft() const
{
	return m_goldleft;
}

void StudentWorld::decGoldLeft()
{
	m_goldleft--;
}

void StudentWorld::incGoldLeft() {
	m_goldleft++;
}
void StudentWorld::placeGold(int x, int y) {
	unique_ptr<Gold> gold;
	gold = unique_ptr<Gold>(new Gold(this, x, y, true, false, true)); //true to make gold visible
	actors.emplace_back(std::move(gold));
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////OIL//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int StudentWorld::lvlOil() {
	return min(2 + static_cast<int>(getLevel()), 21);
}
void StudentWorld::createOil(int num)
{
	for (int k = 0; k < num; k++) {
		int x = 0;
		int y = 0;
		do {
			do {
				x = generateRandX();
			} while (x > 26 && x < 34);

			y = generateRandY();

		} while (!distance(x, y));
		unique_ptr<Oil> oil;
		oil = unique_ptr<Oil>(new Oil(this, x, y));
		actors.emplace_back(std::move(oil));
	}
}

int StudentWorld::getOilLeft() const
{
	return m_oilleft;
}

void StudentWorld::decOilLeft()
{
	m_oilleft--;
}

void StudentWorld::incOilLeft() {
	m_oilleft++;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////SONAR////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createSonar()
{
	unique_ptr<Sonar> sonar;
	sonar = unique_ptr<Sonar>(new Sonar(this, 0, 60, true, getSonarWaterTick()));
	actors.emplace_back(std::move(sonar));

}
void StudentWorld::incSonarLeft() {
	m_sonarleft++;
}
void StudentWorld::decSonarLeft() {
	m_sonarleft--;
}
int StudentWorld::getSonarLeft() const {
	return m_sonarleft;
}
void StudentWorld::useSonar() {
	GameController::getInstance().playSound(SOUND_SONAR);
	int playerX = player->getX();
	int playerY = player->getY();
	std::vector<unique_ptr<Actor>> ::iterator it = actors.begin();
	for (; it != actors.end(); it++) {
		if ((radius(playerX, playerY, (*it)->getX(), (*it)->getY()) < 12)) {
			(*it)->setVisible(true);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////WATER POOL///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void StudentWorld::createWater()
{

	//Each new Water Goodie must be added to a random ice - less spot in the oil field.
	//Water may only be added to a location if the entire 4x4 grid at that location is free
	//of Ice.
	int x = 0;
	int y = 0;
	do {

		x = generateRandX();
		do {
			y = generateRandY();
		} while (y < 20);

	} while (overlapAt(x, y));

	unique_ptr<Water> water;
	water = unique_ptr<Water>(new Water(this, x, y, true, getSonarWaterTick()));
	actors.emplace_back(std::move(water));

}
void StudentWorld::decWaterLeft() {
	m_waterleft--;
}
void StudentWorld::incWaterLeft() {
	m_waterleft++;
}

void StudentWorld::incProtestersLeft()
{
	m_protestersleft++;
}

void StudentWorld::decProtestersLeft()
{
	m_protestersleft--;
}

int StudentWorld::getWaterLeft() const {
	return m_waterleft;
}
int StudentWorld::getProtestersLeft() const
{
	return m_protestersleft;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////DISPLAY//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//pg 22
string StudentWorld::formatStats(unsigned int level, unsigned int lives, int health, int squirts, int gold, int barrelsLeft, int sonar, int score)
{
	string s_level = "Lvl: " + SpacePadNumber(level, 2) + "  ";
	string s_lives = "Lives: " + to_string(lives) + "  ";
	string s_health = "Hlth: " + SpacePadNumber(health, 3) + "%  ";
	string s_squirt = "Wtr: " + SpacePadNumber(squirts, 2) + "  ";
	string s_gold = "Gld: " + SpacePadNumber(gold, 2) + "  ";
	string s_barrel = "Oil Left: " + SpacePadNumber(barrelsLeft, 2) + "  ";
	string s_sonar = "Sonar: " + SpacePadNumber(sonar, 2) + "  ";
	string s_score = "Scr: " + ZeroPadNumber(score);

	string display = s_level + s_lives + s_health + s_squirt + s_gold + s_barrel + s_sonar + s_score;
	return display;
}

void StudentWorld::setDisplayText() {
	unsigned int level = getLevel();
	unsigned int lives = getLives();
	int health = (*player).getHP() * 10;
	int squirts = (*player).getWaterAmnt();
	int gold = (*player).getGoldAmnt();
	int barrelsLeft = lvlOil() - (*player).getOilAmnt();
	int sonar = (*player).getSonarAmnt();
	int score = getScore();
	// Next, create a string from your statistics, of the form:  // Lvl: 52 Lives: 3 Hlth: 80% Wtr: 20 Gld: 3 Oil Left: 2 Sonar: 1 Scr: 321000 

	string s = formatStats(level, lives, health, squirts, gold, barrelsLeft, sonar, score);
	// Finally, update the display text at the top of the screen with your newly created stats 
	setGameStatText(s); // calls our provided GameWorld::setGameStatText
}

string StudentWorld::ZeroPadNumber(int num)
{
	// stringstream conversion int to string
	stringstream s;

	s << num;
	string ans;
	s >> ans;

	// Append zeros
	int length = ans.length();
	for (int i = 0; i < 6 - length; i++)
		ans = "0" + ans;
	return ans;
}

string StudentWorld::SpacePadNumber(int num, int pad)
{
	// stringstream conversion int to string
	stringstream s;

	s << num;
	string ans;
	s >> ans;

	// Append spaces
	int length = ans.length();
	for (int i = 0; i < pad - length; i++)
		ans = " " + ans;
	return ans;
}
