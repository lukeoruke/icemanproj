#include "Actor.h"
#include "StudentWorld.h"
#include "GameController.h"
// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp
const int ICEMAN_START_X = 30;
const int ICEMAN_START_Y = 60;


//  GRAPH OBJ
//      |
//   ________
//   |      |
//  ICE	   ACTOR	
//           |
// _______________________________________________________________
//         |	                 |                               |
// BOULDER,SQUIRT(maybe)       PERSON                         GOODIES
//                               |                               |
//                     ______________________           ____________________
//                    |                      |
//                  ICEMAN               Protester
//                                       |       |      
//                                   Reg. Prot.   HC. PROT.
//                                                                            

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ACTOR////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////  pg 24
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, Direction dir, double size, int depth)
	:GraphObject(imageID, startX, startY, dir, size, depth)
{
	m_isAlive = true;
	m_world = world;
	setVisible(true);
}
Actor::~Actor() {}
//void Actor :: doSomething() {}

StudentWorld* Actor::getWorld() const {
	return  m_world;
}

bool Actor::setDead() {
	return m_isAlive = false;
}
bool Actor::getIsAlive() {
	return m_isAlive;
}

void Actor::moveInDirection() {

	Direction d = this->getDirection();
	switch (d) {

	case GraphObject::Direction::left:
		moveTo(getX() - 1, getY());
		break;
	case GraphObject::Direction::right:
		moveTo(getX() + 1, getY());
		break;
	case GraphObject::Direction::down:
		moveTo(getX(), getY() - 1);
		break;
	case GraphObject::Direction::up:
		moveTo(getX(), getY() + 1);
		break;
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ICE//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
Ice::Ice(int row, int col) :GraphObject(IID_ICE, row, col, right, 0.25, 3) {
	setVisible(true);
}
Ice::~Ice() {};
//void Ice::doSomething(){};

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////SQUIRT///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
Squirt::Squirt(StudentWorld* world, int row, int col, GraphObject::Direction direction)
	: Actor(world, IID_WATER_SPURT, row, col, direction, 1.0, 1)
{
	setVisible(true);
	m_travel_distance = 4;
}
void Squirt::doSomething() {
	if (m_travel_distance > 0 && getWorld()->annoyNearbyPeople(*this, 2)) {
		m_travel_distance = 0;
	}
	else if (m_travel_distance > 0 && !getWorld()->iceInFront(*this) && !getWorld()->boulderInTheWay(*this, 1)) {
		moveInDirection();
		m_travel_distance--;

	}
	else {
		setDead();
	}
	return;
}
bool Squirt::annoy(unsigned int amt)
{
	return false;
}
Squirt::~Squirt() {
	setDead();
}


///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////ICEMAN///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////     pg 27
Iceman::Iceman(StudentWorld* world)
	:Person(world, IID_PLAYER, ICEMAN_START_X, ICEMAN_START_Y, right, 10)
{
	m_water_amnt = 5;
	m_sonar_amnt = 1;
	m_gold_amnt = 0;
	m_oil_amnt = 0;
}
Iceman::~Iceman() {}

void Iceman::doSomething() {
	//check if the iceman is alive
	if (getHP() <= 0) {
		GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
		setDead();
		return;
	}

	getWorld()->overlap(*this);

	//pg30
	int ch;
	if (getWorld()->getKey(ch) == true) {
		switch (ch)
		{
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		case KEY_PRESS_SPACE:
			if (m_water_amnt > 0) {
				GameController::getInstance().playSound(SOUND_PLAYER_SQUIRT);
				m_water_amnt--;
				if (!(getWorld()->iceInFront(*this)) && !(getWorld()->boulderInTheWay(*this, 4))) {  //if there is ice in front, don't fire the water. same logic but for boulder
					if (getWorld()->isRoomInFront(*this))
					{
						getWorld()->createSquirt(*this);
					}
				}

			}
			break;
		case KEY_PRESS_LEFT:  //x-1
			if (getDirection() != left)
				setDirection(left);
			else if (getX() > 0 && !(getWorld()->boulderInTheWay(*this, 1)))
				moveTo(getX() - 1, getY());
			break;
		case KEY_PRESS_RIGHT: //x+1
			if (getDirection() != right)
				setDirection(right);
			else if (getX() < MAX_WINDOW - 4 && !(getWorld()->boulderInTheWay(*this, 1)))
				moveTo(getX() + 1, getY());
			break;
		case KEY_PRESS_DOWN:  //y-1
			if (getDirection() != down)
				setDirection(down);
			else if (getY() > 0 && !(getWorld()->boulderInTheWay(*this, 1)))
				moveTo(getX(), getY() - 1);
			break;;
		case KEY_PRESS_UP: //y+1
			if (getDirection() != up)
				setDirection(up);
			else if (getY() < MAX_WINDOW - 4 && !(getWorld()->boulderInTheWay(*this, 1)))
				moveTo(getX(), getY() + 1);
			break;
		case KEY_PRESS_TAB:
			if (m_gold_amnt > 0) {
				m_gold_amnt--;
				getWorld()->placeGold(this->getX(), this->getY());
				// create gold object
			}
			break;
		case 'Z':
		case 'z':
			if (m_sonar_amnt > 0) {
				m_sonar_amnt--;
				getWorld()->useSonar();
			}

		}
	}



}

void Iceman::gainGold() {
	m_gold_amnt++;
	getWorld()->increaseScore(10);
}
void Iceman::gainOilIceman() {
	m_oil_amnt++;
	getWorld()->increaseScore(1000);
}
void Iceman::gainSonarIceman() {
	m_sonar_amnt++;
	getWorld()->increaseScore(75);
}
void Iceman::gainWaterIceman() {
	m_water_amnt += 5;
	getWorld()->increaseScore(100);
}

int Iceman::getWaterAmnt() const
{
	return m_water_amnt;
}

int Iceman::getSonarAmnt() const
{
	return m_sonar_amnt;
}

int Iceman::getGoldAmnt() const
{
	return m_gold_amnt;
}

int Iceman::getOilAmnt() const {
	return m_oil_amnt;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////BOULDER//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg31
Boulder::Boulder(StudentWorld* world, int x, int y)
	:Actor(world, IID_BOULDER, x, y, down, 1.0, 1) {
	m_state = stable;
	m_BoulderTick = 29;
	getWorld()->incBouldersLeft();
}
Boulder::~Boulder() {
	getWorld()->decBouldersLeft();
}

void Boulder::setStateBoulder(state x) {
	m_state = x;
}
Boulder::state Boulder::getStateBoulder() const {
	return m_state;
}

void Boulder::doSomething() {
	if (!this->getIsAlive()) { //if they are not alive
		return;
	}

	switch (this->getStateBoulder()) {
	case Boulder::state::stable:
		//if there exists ice underneath the boulder, stay stable
		//if not change the state to waiting
		if (!getWorld()->iceInFront(*this)) { //if there is no ice underneath
			m_state = waiting;
		}
		//check if there is any ince in the 4 squares below it    pg 32
		break;
	case Boulder::state::waiting:
		if (m_BoulderTick == 0) {  // once 30 ticks have passed, change the state of the boudler to falling
			m_state = falling;
			GameController::getInstance().playSound(SOUND_FALLING_ROCK);

		}
		else {
			m_BoulderTick--;     //decrement the tick by one
		}
		break;
	case Boulder::state::falling:
		this->moveInDirection();
		getWorld()->annoyNearbyPeople(*this, 100);
		//check after one tick whether there is ice, boulder, or out of border
		if (getWorld()->iceInFront(*this) || getWorld()->boulderInFront(*this)
			|| getY() == 0)
		{
			setDead();
		}

		break;
	}


}
bool Boulder::annoy(unsigned int amt)
{
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////GOODIES//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   

// state is the state the item should be(temp,exc..) sound is the SOUNDIID of that item, type is the what type of object it is, I decided to make it with
// characters( gold = 'g', oil = 'o') By passing all of these in the parameters, we can just use one doSomething for all items. feel free to change anything 
Goodies::Goodies(StudentWorld* world, int x, int y, int imageID,
	int sound, bool activateOnPlayer, bool activateOnProtester)
	:Actor(world, imageID, x, y, right, 1, 2) {
	goodie_sound = sound;
	is_active_on_player = activateOnPlayer;
	is_active_on_protest = activateOnProtester;
}
Goodies::~Goodies() {}

bool Goodies::annoy(unsigned int amt)
{
	return false;
}

//getters

int Goodies::getItemSound() const {
	return goodie_sound;
}
bool Goodies::getIsActiveOnPlayer() const {
	return is_active_on_player;
}
bool Goodies::getIsActiveOnProtester() const {
	return is_active_on_protest;
}
Goodies::goodieState Goodies::getItemState() const {
	return goodie_state;
}
int Goodies::getitemTick() const {
	return m_ticks;
}

//setters

void Goodies::decreaseTick() {
	m_ticks--;
}
void Goodies::setItemState(bool temp) {
	if (temp == true) {
		goodie_state = temporary;
	}
	if (temp == false) {
		goodie_state = permanent;
	}
}
void Goodies::setitemTicks(int ticks) {
	m_ticks = ticks;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////GOLD/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg34
Gold::Gold(StudentWorld* world, int x, int y, bool temporary, bool activateOnPlayer, bool activateOnProtester)
	: Goodies(world, x, y, IID_GOLD, SOUND_GOT_GOODIE, activateOnPlayer, activateOnProtester) {
	getWorld()->incGoldLeft();
	setItemState(temporary);
	setitemTicks(100);
	if (temporary == false) {  // only show the gold when its in temp form
		setVisible(false);
	}
	else {
		setVisible(true);
	}
}
Gold::~Gold() {
	getWorld()->decGoldLeft();
}
void Gold::doSomething() {
	if (!this->getIsAlive()) { //if they are not alive
		return;
	}
	if (!this->isVisible()) {  //if the gold is currently not visible
		int itemX = this->getX();
		int itemY = this->getY();

		//if the ice man is in range of the item, make it visible
		if (getWorld()->icemanNearby(*this, itemX, itemY, 4.0)) {
			this->setVisible(true);
			return;
		}
	}

	if (this->isVisible() && this->getItemState() == permanent) {  //if the item is visible to the player 
		int itemX = this->getX();
		int itemY = this->getY();

		if (getWorld()->icemanNearby(*this, itemX, itemY, 3.0)) {
			setDead(); // if the iceman is 3 units away, set the item to dead
			GameController::getInstance().playSound(this->getItemSound());
			getWorld()->incIcemanItem('g');
		}
	}
	if (this->getItemState() == temporary) {  //any item that is in a temporary state will come here and count down(dropped gold, sonar, water)
		if (getWorld()->protesterFoundGold(*this))
		{
			setDead();
			GameController::getInstance().playSound(SOUND_PROTESTER_FOUND_GOLD);
		}
		if (this->getitemTick() == 0) { //if the item has no tick left destroy it
			setDead();
		}
		this->decreaseTick();  //decrease tick
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////OIL//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg33
Oil::Oil(StudentWorld* world, int x, int y)
	: Goodies(world, x, y, IID_BARREL, SOUND_FOUND_OIL, true, false)
{
	getWorld()->incOilLeft();
	setVisible(false);   //oil should be invisible
}
Oil::~Oil() {
	getWorld()->decOilLeft();
}
void Oil::doSomething() {
	if (!this->getIsAlive()) { //if they are not alive
		return;
	}
	if (!this->isVisible()) {  //if the oil is currently not visible
		int itemX = this->getX();
		int itemY = this->getY();

		//if the ice man is in range of the oil, make it visible
		if (getWorld()->icemanNearby(*this, itemX, itemY, 4.0)) {
			this->setVisible(true);
			return;
		}
	}
	if (this->isVisible() && (getWorld()->icemanNearby(*this, this->getX(), this->getY(), 3.0))) {  //if the oil is visible to the player and close enough, pick it up
		setDead(); // if the iceman is 3 units away, set the item to dead
		GameController::getInstance().playSound(this->getItemSound());
		getWorld()->incIcemanItem('o');  //increase the oil amnt
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////SONAR////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg 21pg37
Sonar::Sonar(StudentWorld* world, int x, int y, bool temp, int tick)
	: Goodies(world, x, y, IID_SONAR, SOUND_GOT_GOODIE, true, false)
{
	getWorld()->incSonarLeft();
	setVisible(true);    //sonar should be visible
	setitemTicks(tick);
}
Sonar::~Sonar() {
	getWorld()->decSonarLeft();
}
void Sonar::doSomething() {
	if (this->getitemTick() == 0) { //if the item has no tick left destroy it
		setDead();
	}
	int itemX = this->getX();
	int itemY = this->getY();
	if (getWorld()->icemanNearby(*this, itemX, itemY, 3.0)) { // if the iceman is 3 units away, set the item to dead
		setDead();
		GameController::getInstance().playSound(this->getItemSound());
		getWorld()->incIcemanItem('s');  //incrase the sonar amnt
	}
	this->decreaseTick();   //decrease the tick by one
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////WATER////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg 38
Water::Water(StudentWorld* world, int x, int y, bool temp, int tick)
	: Goodies(world, x, y, IID_WATER_POOL, SOUND_GOT_GOODIE, true, false)
{
	setitemTicks(tick);
	getWorld()->incWaterLeft();
	setVisible(true);
}
Water:: ~Water() {
	getWorld()->decWaterLeft();
}

void Water::doSomething() {
	if (this->getitemTick() == 0) { //if the item has no tick left destroy it
		setDead();
	}
	int itemX = this->getX();
	int itemY = this->getY();
	if (getWorld()->icemanNearby(*this, itemX, itemY, 3.0)) { // if the iceman is 3 units away, set the item to dead
		setDead();
		GameController::getInstance().playSound(this->getItemSound());
		getWorld()->incIcemanItem('w');  //increase the water amnt
	}
	this->decreaseTick();    //decrease the tick by one
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////PERSON///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////   pg34


Person::Person(StudentWorld* world, int imageID, int startX, int startY, Direction dir, unsigned int health)
	:Actor(world, imageID, startX, startY, dir, 1.0, 0)
{
	m_HP = health;
	//player 
}

Person::~Person()
{
}

bool Person::annoy(unsigned int amt)
{
	m_HP = m_HP - amt;
	return true;
}

int Person::getHP() const {
	return m_HP;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////PROTESTER////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

Protester::Protester(StudentWorld* world, int x, int y, int imageID, unsigned int health)
	:Person(world, imageID, x, y, left, health)
{
	m_leaveState = false;
	rest_state = 0;
	//player->getIceman();
	m_distancetoTravel = 0;
	getWorld()->incProtestersLeft();

}

Protester::~Protester()
{
	getWorld()->decProtestersLeft();
}

//bool Protester::annoy(unsigned int amt)
//{
//	if (getIsLeaving())
//		return false;
//	else
//		m_HP = m_HP - amt;
//	return true;
//}

bool Protester::getIsLeaving()
{
	return m_leaveState;
}
//bool Protester::oppositeDirection() {  //the cases are the protesters direction
//	Direction x = getWorld()->getIcemanDirection();
//	Direction dir = this->getDirection();
//	switch (dir) {
//	case(up):
//		if (x == down) {
//			return true;
//		}
//		else {
//			return false;
//		}
//		break;
//	case(down):
//		if (x == up) {
//			return true;
//		}
//		else {
//			return false;
//		}
//		break;
//	case(right):
//		if (x == left) {
//			return true;
//		}
//		else {
//			return false;
//		}
//		break;
//	case(left):
//		if (x == right) {
//			return true;
//		}
//		else {
//			return false;
//		}
//		break;
//	default:
//		return false;
//	}
//
//}
void Protester::moveProtester() {
	switch (this->getDirection()) {
	case left:  //x-1
		if (this->getX() > 0 && !(getWorld()->boulderInTheWay(*this, 1)) && !(getWorld()->iceInFront(*this))) {
			moveTo(this->getX() - 1, this->getY());
		}
		else {
			this->m_distancetoTravel = 0;
		}
		break;
	case right: //x+1
		if (this->getX() < MAX_WINDOW - 4 && !(getWorld()->boulderInTheWay(*this, 1)) && !(getWorld()->iceInFront(*this))) {
			moveTo(getX() + 1, this->getY());
		}
		else {
			this->m_distancetoTravel = 0;
		}
		break;
	case down:  //y-1
		if (this->getY() > 0 && !(getWorld()->boulderInTheWay(*this, 1)) && !(getWorld()->iceInFront(*this))) {
			moveTo(this->getX(), this->getY() - 1);
		}
		else {
			this->m_distancetoTravel = 0;
		}
		break;
	case up://y+1
		if (this->getY() < MAX_WINDOW - 4 && !(getWorld()->boulderInTheWay(*this, 1)) && !(getWorld()->iceInFront(*this))) {
			moveTo(this->getX(), this->getY() + 1);
		}
		else {
			this->m_distancetoTravel = 0;
		}
		break;

	}
}

void Protester::pickRandDirection(int protesterX, int protesterY) {
	bool outofBounds;
	do {
		int direction = rand() % 4 + 1;

		switch (direction) {
		case 1: ///move up
			this->setDirection(up);
			if (protesterY + 1 >= MAX_WINDOW - 4) {
				outofBounds = true;
			}

			else {
				outofBounds = false;
			}
			break;
		case 2: //move right
			this->setDirection(right);
			if (protesterX + 1 >= MAX_WINDOW - 4) {
				outofBounds = true;
			}
			else {
				outofBounds = false;
			}
			break;
		case 3: //move down
			this->setDirection(down);
			if (protesterY - 1 <= 0) {
				outofBounds = true;
			}
			else {
				outofBounds = false;
			}
			break;
		case 4: //move left
			this->setDirection(left);
			if (protesterX - 1 <= 0) {
				outofBounds = true;
			}
			else {
				outofBounds = false;
			}
			break;
		}

	} while (getWorld()->iceInFront(*this) || getWorld()->boulderInFront(*this) || outofBounds == true);


}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////REGULAR//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
RegularProtester::RegularProtester(StudentWorld* world, int x, int y)
	:Protester(world, x, y, IID_PROTESTER, 5)
{
	m_ticksWait = std::max(0, 3 - static_cast<int>(getWorld()->getLevel()) / 4);
	m_shout = 0;
	m_perpendicular_tick = 0;
}

bool RegularProtester::annoy(unsigned int amt) {
	if (!m_leaveState)
	{
		m_HP -= amt;

		if (getHP() <= 0)
		{
			rest_state = 0;
			GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
			if (amt == 2) //defeated by squirt
			{
				getWorld()->increaseScore(100);
			}
			else
			{
				getWorld()->increaseScore(500);
			}
			m_leaveState = true;
		}
		else
		{
			GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
			//TODO: set ticks to make protester wait for N = max(50, 100 – current_level_number * 10) 
			rest_state = -1 * std::max(50, 100 - static_cast<int>(getWorld()->getLevel()) * 10);
		}
		return true;
	}
	return false;
}

int RegularProtester::numSquaresToMoveInCurrentDirection() {
	return rand() % 53 + 8;     // 8-60
}

void RegularProtester::doSomething() {
	if (!this->getIsAlive()) { //1
		return;
	}

	if (this->rest_state != m_ticksWait) { //2
		this->rest_state++;
		return;
	}

	int protesterX = this->getX();
	int protesterY = this->getY();
	//std::cout << "the protester x and y are " << protesterX << "  " << protesterY;
	if (getIsLeaving() && rest_state == m_ticksWait) { //3

		if (protesterX == 60 && protesterY == 60) { //a
			this->setDead();
		}
		if (stepsToLeave.empty()) {  //if the vector is  empty
			stepsToLeave = getWorld()->leaveField(protesterX, protesterY);
		}
		else {
			Direction d = stepsToLeave.front();
			stepsToLeave.erase(stepsToLeave.begin());  //this should pop front of vec
			this->setDirection(d);
			this->moveProtester();
			return;
		}
		return;

	}
	if (rest_state == m_ticksWait) {  //once the waiting time is over
		if (m_distancetoTravel == 0) {
			m_distancetoTravel = numSquaresToMoveInCurrentDirection();
		}
		//4 DONE
		if (getWorld()->icemanNearby(*this, protesterX, protesterY, 4.0) && getWorld()->isFacingIceman(*this)) {
			if (m_shout == 0) { //protester has not shoulted in the last non resting 15 ticks
				m_shout = 15;
				GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
				getWorld()->annoyIceman(2);
				//inform the iceman that he been annoyed for a totoal of 2 annoynace points
			}
			m_shout--;
			return;
		}
		//5 straight hor or ver line of sight from iceman, 4 untis away from iceman DONE
		if (getWorld()->icemanInSight(protesterX, protesterY) && getWorld()->
			protesterRadius(protesterX, protesterY) >= 4.0 && getWorld()->canReachIceman(protesterX, protesterY) && !getWorld()->boulderInFront(*this)) {
			Direction dir = getWorld()->faceIceman(protesterX, protesterY);
			this->setDirection(dir);  //Change its direction to face in the direction of the Iceman]
			this->moveProtester();//then take one step toward iceman
			m_distancetoTravel = 0;
			rest_state = 0;
			return;
		}
		//6 iceman not in sight DONE
		else {
			m_distancetoTravel--; //decrement numSquaresToMoveInCurrentDirection
			if (m_distancetoTravel <= 0) {
				pickRandDirection(protesterX, protesterY);
				this->moveProtester();
				if (m_distancetoTravel <= 0) {
					m_distancetoTravel = numSquaresToMoveInCurrentDirection();

				}
			}
		}
		//7  DONE
		if (getWorld()->canTurn(protesterX, protesterY, this->getDirection())) {
			if (m_perpendicular_tick <= 0) {
				//set the direction to the selected perp. direction
				this->setDirection(getWorld()->makeTurn(protesterX, protesterY, this->getDirection()));
				//pick a new value for numSquares
				m_distancetoTravel = numSquaresToMoveInCurrentDirection();

				m_perpendicular_tick = 200;
			}
			else {
				m_perpendicular_tick--;
			}
		}
		else {
			m_perpendicular_tick--;
		}

		//8  actual movement
		this->moveProtester();
	}
	//9 DONE
	m_distancetoTravel--;
	rest_state = 0;
	if (m_shout != 0) {
		m_shout--;
	}
	//this line is just testing
	//getWorld()->leaveField(protesterX, protesterY);
}

void RegularProtester::gainGold() {
	m_leaveState = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////HARDCORE/////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
HardcoreProtester::HardcoreProtester(StudentWorld* world, int x, int y)
	:Protester(world, x, y, IID_HARD_CORE_PROTESTER, 20)
{
	m_ticksWait = std::max(0, 3 - static_cast<int>(getWorld()->getLevel()) / 4);
	m_shout = 0;
	m_perpendicular_tick = 0;
}

bool HardcoreProtester::annoy(unsigned int amt) {
	if (!m_leaveState)
	{
		m_HP -= amt;

		if (getHP() <= 0)
		{
			GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
			if (amt == 2) //defeated by squirt
			{
				getWorld()->increaseScore(250);
			}
			else
			{
				getWorld()->increaseScore(500);
			}
			m_leaveState = true;
		}
		else
		{
			GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
			//TODO: set ticks to make protester wait for N = max(50, 100 – current_level_number * 10) 
			rest_state = -1 * std::max(50, 100 - static_cast<int>(getWorld()->getLevel()) * 10);
		}
		return true;
	}
	return false;
}

void HardcoreProtester::gainGold()
{
	rest_state = -1 * std::max(50, 100 - static_cast<int>(getWorld()->getLevel()) * 10);
}
int HardcoreProtester::numSquaresToMoveInCurrentDirection() {
	return rand() % 53 + 8;     // 8-60
}
void HardcoreProtester::doSomething() {
	if (!this->getIsAlive()) { //1
		return;
	}

	if (this->rest_state != m_ticksWait) { //2
		this->rest_state++;
		return;
	}

	int protesterX = this->getX();
	int protesterY = this->getY();
	if (getIsLeaving() && rest_state == m_ticksWait) { //3

		if (protesterX == 60 && protesterY == 60) { //a
			this->setDead();
		}
		if (stepsToLeave.empty()) {  //if the vector is  empty
			stepsToLeave = getWorld()->leaveField(protesterX, protesterY);
		}
		else {
			Direction d = stepsToLeave.front();
			stepsToLeave.erase(stepsToLeave.begin());  //this should pop front of vec
			this->setDirection(d);
			this->moveProtester();
			return;
		}
		return;

	}
	if (rest_state == m_ticksWait) {  //once the waiting time is over
		if (m_distancetoTravel == 0) {
			m_distancetoTravel = numSquaresToMoveInCurrentDirection();
		}
		//4 DONE
		if (getWorld()->icemanNearby(*this, protesterX, protesterY, 4.0) && getWorld()->isFacingIceman(*this)) {
			if (m_shout == 0) { //protester has not shoulted in the last non resting 15 ticks
				m_shout = 15;
				GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
				getWorld()->annoyIceman(2);
				//inform the iceman that he been annoyed for a totoal of 2 annoynace points
			}
			m_shout--;
			return;
		}
		//5 straight hor or ver line of sight from iceman, 4 untis away from iceman DONE
		if (getWorld()->icemanInSight(protesterX, protesterY) && getWorld()->
			protesterRadius(protesterX, protesterY) >= 4.0 && getWorld()->canReachIceman(protesterX, protesterY) && !getWorld()->boulderInFront(*this)) {
			Direction dir = getWorld()->faceIceman(protesterX, protesterY);
			this->setDirection(dir);  //Change its direction to face in the direction of the Iceman]
			this->moveProtester();//then take one step toward iceman
			m_distancetoTravel = 0;
			rest_state = 0;
			return;
		}
		//6 iceman not in sight DONE
		else {
			m_distancetoTravel--; //decrement numSquaresToMoveInCurrentDirection
			if (m_distancetoTravel <= 0) {
				pickRandDirection(protesterX, protesterY);
				this->moveProtester();
				if (m_distancetoTravel <= 0) {
					m_distancetoTravel = numSquaresToMoveInCurrentDirection();

				}
			}
		}
		//7  DONE
		if (getWorld()->canTurn(protesterX, protesterY, this->getDirection())) {
			if (m_perpendicular_tick <= 0) {
				//set the direction to the selected perp. direction
				this->setDirection(getWorld()->makeTurn(protesterX, protesterY, this->getDirection()));
				//pick a new value for numSquares
				m_distancetoTravel = numSquaresToMoveInCurrentDirection();

				m_perpendicular_tick = 200;
			}
			else {
				m_perpendicular_tick--;
			}
		}
		else {
			m_perpendicular_tick--;
		}

		//8  actual movement
		this->moveProtester();
	}
	//9 DONE
	m_distancetoTravel--;
	rest_state = 0;
	if (m_shout != 0) {
		m_shout--;
	}
	//this line is just testing
	//getWorld()->leaveField(protesterX, protesterY);
}
