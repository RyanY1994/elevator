#include <iostream>
#include <set>
#include <list>
#include <thread>
const int UNPRESS = 1000;

namespace ElevatorDir{

enum {
	DIR_UP = 1,
	DIR_DOWN = -1,
	DIR_NO_DIR = 0;
};
}

class People {
public:
	People(int id, int nowStair): id(id), nowStair(nowStair), target(UNPRESS){
		std::cout << "people " << id << " 来了。" << std::endl;
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
	void pressUpOrDownButton(bool isUp, Elevator *elevator) {
		if (isUp) {
			std::cout << "people " << id << " 按了向上的按钮" << std::endl;
			elevator->upStairs(this);
		} else {
			std::cout << "people " << id << " 按了向下的按钮" << std::endl;
			elevator->downStairs(this);
		}

	}
	void pressTargetStairButton(int button, Elevator *elevator) {
		target = button;
		std::cout << "people " << id << " 在电梯里按了第" << button << "层" << std::endl;
		elevator->pressButton(this);
	}

	void wait(Elevator *elevator) {
		[&]() -> void {
			while(elevator->getNowStair() != nowStair){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			std::cout << "people " << id << " 等到了电梯，并进入了电梯" << std::endl;
			int targetStair = rand() % 16 + 1;
			pressTargetStairButton(targetStair, elevator);
			//std::cout << "people " << id << " 在电梯中按了" << targetStair << "楼" << std::endl;
		};
	}
private:
	int nowStair;
	int id;
	int target;
};

class Elevator {
public:
	Elevator(int top, int bottom): mNowStair(1), mTop(top), mBottom(bottom), mDir(ElevatorDir::DIR_NO_DIR){

	}
	~Elevator() {
		mPeopleId.clear();
		for (auto p : task) {
			delete p;
		}
		task.clear();
	}
	int getNowStair() const{
		return mNowStair;
	}
	void upStairs(People *);
	void downStairs(People *);
	void pressButton(People *people);
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
			return a->getTarget() < b->getTarget();
		}
	};

private:
	std::set<People*, ComparePeople> task;
	std::set<int> mPeopleId;
	size_t mPeopleCount;
	int mNowStair;
	int mTop;
	int mBottom;
	int mDir;
};

void Elevator::upStairs(People *people)
{
	task.insert(people);
}

void Elevator::downStairs(People *people)
{
	task.insert(people);
}

void Elevator::run()
{
	[this]() -> void {
		int nowStair = mNowStair;
		while(true) {
			while (task.empty()){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			if (mDir == ElevatorDir::DIR_NO_DIR) {
				auto people = *task.begin();
				auto target = people->getNowStair();
				if (nowStair < target) {
					mDir = ElevatorDir::DIR_UP;
				} else if (nowStair > target) {
					mDir = ElevatorDir::DIR_DOWN;
				} else {
					mDir = ElevatorDir::DIR_NO_DIR;
				}
			} else if (mDir == ElevatorDir::DIR_UP){
				nowStair++;
			} else {
				nowStair--;
			}
			//TODO 按照targetStair排序还是nowStair排序
			//TODO 对set加锁
			if (nowStair == (*task.begin())->getNowStair()) {
				mNowStair = nowStair;
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));//开门两秒等待人进电梯
			}
			if (task.empty()) {
				mDir = ElevatorDir::DIR_NO_DIR;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
	};

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
	//task.insert(people);
}

int main()
{
	Elevator *elevator = new Elevator(-3,16);
	elevator->run();

	for(int i = 1; i <= 5; i++) {
		People people(i, rand() % 16 + 1);
		people.pressUpOrDownButton(true, elevator);
		people.wait(elevator);
	}

	return 0;
}