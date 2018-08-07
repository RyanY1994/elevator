#include <iostream>
#include <set>
#include <list>
#include <thread>
#include <cstdlib>
#include <mutex>

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
//TODO ɾ�����ò���
class People {
public:
	People(int id, int nowStair): id(id), nowStair(nowStair),
								  target(ElevatorSpace::ELEVATOR_UNPRESS),
								  dir(ElevatorSpace::DIR_UP)
	{
		printf("people%d ���ˡ�����¥����%d\n",id, nowStair);
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

	void wait(int &elevatorStair, bool &isOpen, int &elevatorDir) {
		std::thread th([&]() -> void {
			//TODO people�������µİ�ť�����ڵ������е��ǿ���ʱ��ȥ��
			while(elevatorStair != nowStair || isOpen == false || elevatorDir != this->dir){
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			int targetStair = nowStair + (dir == ElevatorSpace::DIR_DOWN? -2 : 3);
			this->target = targetStair;
			printf("people%d �ȵ��˵��ݣ��������˵��ݣ�����%d¥\n", id, targetStair);
			while(elevatorStair != targetStair || isOpen == false) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			printf("people%d ���˵���\n",id);
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
	std::mutex mTaskLock;
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
	//TODO ����
	std::lock_guard<std::mutex> lck(this->mTaskLock);
	people->setDir(ElevatorSpace::DIR_UP);
	mTask.push_back(people);
}

void Elevator::downStairs(People *people)
{
	//TODO JIASUO
    std::lock_guard<std::mutex> lck(this->mTaskLock);
	people->setDir(ElevatorSpace::DIR_DOWN);
	mTask.push_back(people);
}

void Elevator::treatPeople(People *people, bool isUp)
{
	if (isUp) {
		printf("people%d �������ϵİ�ť\n", people->getId());
		this->upStairs(people);
	} else {
        printf("people%d �������µİ�ť\n", people->getId());
		this->downStairs(people);
	}
	people->wait(mNowStair, isOpen, mDir);
}

//�˷����ж�mTask���и��ִ���
int Elevator::handleElevatorRequest(int flag)
{
	//TODO ����
    std::lock_guard<std::mutex> lck(this->mTaskLock);
	if (flag == ElevatorSpace::ELEVATOR_GET_NEXT_TARGET) {
		if (mTask.empty()) return ElevatorSpace::ELEVATOR_NO_NEXT_TARGET;

	}
	
}
void Elevator::peopleOut()
{
	//todo
    std::lock_guard<std::mutex> lck(this->mTaskLock);
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
	std::thread th([this]() -> void {
		while(true) {
		    bool first = true;
			while (mTask.empty()){
			    if (first) {
			        first = false;
			        printf("������ʱû��������ͣ�У�������%d¥\n", mNowStair);
			    }
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			//�õ�һ�������ڵ�¥��ȷ�����·���
			if (mDir == ElevatorSpace::DIR_NO_DIR) {
				printf("�����Ѿ�����\n" );
				//�����ȷ���
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
			printf("======%d" ,mNowStair);
			if (mDir == ElevatorSpace::DIR_DOWN) {
				printf("====== ��\n");
			} else if(mDir == ElevatorSpace::DIR_UP) {
                printf("====== ��\n");
			} else {
                printf("====== ͣ\n");
			}
			if (isOpen) {
				printf("���ݿ�����...����������%d¥\n", mNowStair);
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			}
			peopleOut();
			isOpen = false;
			if (mTask.empty()) {
				mDir = ElevatorSpace::DIR_NO_DIR;
			}
			if (mDir == ElevatorSpace::DIR_UP) {
				bool continueUp = false;
				for (auto people : mTask) {
					if (people->getNowStair() > mNowStair ||
						(people->getTarget() != ElevatorSpace::ELEVATOR_UNPRESS && people->getTarget() > mNowStair)) {
						if (people->getDir() == mDir) {
							continueUp = true;
						}
					}
				}
				if (continueUp == false) {
					mDir = ElevatorSpace::DIR_DOWN;
				}
			} else if (mDir == ElevatorSpace::DIR_DOWN) {
				bool continueDown = false;
				for (auto people : mTask) {
					if (people->getNowStair() < mNowStair ||
						(people->getTarget() != ElevatorSpace::ELEVATOR_UNPRESS && people->getTarget() < mNowStair)) {
						if(people->getDir() == mDir) {
							continueDown = true;
						}
					}
				}
				if (continueDown == false) {
					mDir = ElevatorSpace::DIR_UP;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
	});
	th.detach();
}

int main()
{
	Elevator *elevator = new Elevator(-3, 16);
	elevator->run();

	for(int i = 1; i <= 4; i++) {
		People *people = new People(i, rand() % 16 + 1);
		bool isUp = (i & 1) == 1;
		elevator->treatPeople(people, isUp);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
	std::this_thread::sleep_for(std::chrono::seconds(900));
	return 0;
}