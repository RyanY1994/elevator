#include <iostream>
#include <set>
#include <list>
#include <thread>

namespace ElevatorSpace{

enum {
	DIR_UP = 1,
	DIR_DOWN = -1,
	DIR_NO_DIR = 0,
	ELEVATOR_UNPRESS = 1000,
	ELEVATOR_NO_NEXT_TARGET = -1000,
	ELEVATOR_PEOPLE_ENTER = 2,
	ELEVATOR_PROPLE_OUT = 3,
	ELEVATOR_GET_NEXT_TARGET = 4,
};
}

class People {
public:
	People(int id, int nowStair): id(id), nowStair(nowStair),
								  target(ElevatorSpace::ELEVATOR_UNPRESS),
								  dir(ElevatorSpace::DIR_UP)
	{
		std::cout << "people " << id << " 来了。所在楼层是" << nowStair << std::endl;
	}
	int getId() const{
		return id;
	}
	int getTarget() const{
		return target;
	}
	int getNowStair() const {
		return nowStair;
	}
	int getDir() {
		return dir;
	}
	void setDir(int tmp_dir) {
		this->dir = tmp_dir;
	}

	void wait(int &elevatorStair, bool &isOpen) {
		std::thread th([&]() -> void {
			while(elevatorStair != nowStair || isOpen == false){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			int targetStair = rand() % 16 + 1;
			this->target = targetStair;
			std::cout << "people " << id << " 等到了电梯，并进入了电梯，按了 "<< targetStair << "楼" << std::endl;
			while(elevatorStair != targetStair || isOpen == false) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::cout << "people " << id << " 出了电梯" << std::endl;
			nowStair = ElevatorSpace::ELEVATOR_PROPLE_OUT;
			target = ElevatorSpace::ELEVATOR_PROPLE_OUT;
		});
		th.detach();
	}
private:
	int dir;
	int nowStair;
	int id;
	int target;
};

class Elevator {
public:
	Elevator(int top, int bottom): mNowStair(1), mTop(top), mBottom(bottom), mDir(ElevatorSpace::DIR_NO_DIR), isOpen(false){

	}
	~Elevator() {
		mPeopleId.clear();
		for (auto p : mTask) {
			delete p;
		}
		mTask.clear();
	}
	int getNowStair() const{
		return mNowStair;
	}
	bool isElevatorOpen() {
		return isOpen;
	}
	void upStairs(People *);
	void downStairs(People *);
	void treatPeople(People *, bool);
	void pressButton(People *people);
	int handleElevatorRequest(int);
	void peopleOut(); //TODO
	size_t getPeopleCount() {
		return mPeopleCount;
	}
	void showPeople() {
		for(auto id : mPeopleId) {
			std::cout << id << " ";
		}
		std::cout << std::endl;
	}
	void run();
	struct ComparePeople: public std::binary_function<People *, People *, bool> {
		bool operator() (const People *a, const People *b) const {
			if (a->getNowStair() != b->getNowStair())
				return a->getNowStair() < b->getNowStair();
			return a->getTarget() < b->getTarget();
		}
	};

private:
	std::list<People*> mTask;
	//std::set<People*, ComparePeople> Task;
	std::set<int> mPeopleId;
	size_t mPeopleCount;
	int mNowStair;
	int mTop;
	int mBottom;
	int mDir;
	bool isOpen;
};

void Elevator::upStairs(People *people)
{
	people->setDir(ElevatorSpace::DIR_UP);
	mTask.push_back(people);
}

void Elevator::downStairs(People *people)
{
	people->setDir(ElevatorSpace::DIR_DOWN);
	mTask.push_back(people);
}

void Elevator::treatPeople(People *people, bool isUp)
{
	if (isUp) {
		std::cout << "people " << people->getId() << " 按了向上的按钮" << std::endl;
		this->upStairs(people);
	} else {
		std::cout << "people " << people->getId() << " 按了向下的按钮" << std::endl;
		this->downStairs(people);
	}
	people->wait(mNowStair, isOpen);
}

//此方法中对mTask进行各种处理。
int Elevator::handleElevatorRequest(int flag)
{
	//TODO 加锁
	if (flag == ElevatorSpace::ELEVATOR_GET_NEXT_TARGET) {
		if (mTask.empty()) return ElevatorSpace::ELEVATOR_NO_NEXT_TARGET;

	}
	
}
void Elevator::peopleOut()
{
	//todo
	for (auto taskIteraotr = mTask.begin(); taskIteraotr != mTask.end();) {
		if ((*taskIteraotr)->getNowStair() == ElevatorSpace::ELEVATOR_PROPLE_OUT && (*taskIteraotr)->getTarget() == ElevatorSpace::ELEVATOR_PROPLE_OUT) {
			delete(*taskIteraotr);
			taskIteraotr = mTask.erase(taskIteraotr);
		} else {
			++taskIteraotr;
		}
	}
}

void Elevator::run()
{
	std::cout << "enter run function" << std::endl;
	std::thread th([this]() -> void {
		while(true) {
			while (mTask.empty()){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			//用第一个人现在的楼层确定上下方向
			if (mDir == ElevatorSpace::DIR_NO_DIR) {
				std::cout << "电梯已经启动" << std::endl;
				//先来先服务
				auto people = mTask.front();
				int target = people->getNowStair();
				if (mNowStair < target) {
					mDir = ElevatorSpace::DIR_UP;
				} else if (mNowStair > target) {
					mDir = ElevatorSpace::DIR_DOWN;
				} else {
					mDir = ElevatorSpace::DIR_NO_DIR;
				}
			} else if (mDir == ElevatorSpace::DIR_UP) {
				mNowStair++;
			} else {
				mNowStair--;
			}
			for (auto people : mTask) {
				if (people->getNowStair() == mNowStair && people->getDir() == this->mDir) {
					isOpen = true;
				}
				if (people->getTarget() != ElevatorSpace::ELEVATOR_UNPRESS && people->getTarget() == mNowStair) {
					isOpen = true;
				}
			}
			if (isOpen) {
				std::cout << "电梯开门了...电梯现在在" << mNowStair << "楼" << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			}
			peopleOut();
			isOpen = false;
			if (mTask.empty()) {
				mDir = ElevatorSpace::DIR_NO_DIR;
			}
			if (mDir == ElevatorSpace::DIR_UP) {
				for (auto people : mTask) {
					if (people->getNowStair() > mNowStair ||
						(people->getTarget() != ElevatorSpace::ELEVATOR_UNPRESS && people->getTarget() > mNowStair)) {

					} else {
						mDir = ElevatorSpace::DIR_DOWN;
					}
				}
			} else if (mDir == ElevatorSpace::DIR_DOWN) {
				for (auto people : mTask) {
					if (people->getNowStair() < mNowStair ||
						(people->getTarget() != ElevatorSpace::ELEVATOR_UNPRESS && people->getTarget() < mNowStair)) {

					} else {
						mDir = ElevatorSpace::DIR_UP;
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
	});
	th.detach();
}

void Elevator::pressButton(People *people)
{
	int nowStair = getNowStair();
	int targetStair = people->getTarget();
	if (mDir > 0) {
		if (targetStair <= nowStair) {
			std::cout << "现在楼层是" << nowStair << " 目标楼层是" << targetStair << ", 会忽略掉。" << std::endl;
			return;
		}
	} else if(mDir < 0){
		if (targetStair >= nowStair) {
			std::cout << "现在楼层是" << nowStair << " 目标楼层是" << targetStair << ", 会忽略掉。" << std::endl;
			return;
		}
	}
	//进电梯时已经添加了
	//mTask.insert(people);
}

int main()
{
	Elevator *elevator = new Elevator(-3, 16);
	elevator->run();

	for(int i = 1; i <= 3; i++) {
		People *people = new People(i, rand() % 16 + 1);
		bool isUp = (i & 1) == 1;
		elevator->treatPeople(people, isUp);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
	std::this_thread::sleep_for(std::chrono::seconds(60));
	return 0;
}