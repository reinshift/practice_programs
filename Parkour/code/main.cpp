/*
	跑酷游戏开发日志（基于EasyX库）
	1.创建项目
	2.导入素材
	3.实现游戏界面
	4.实现玩家奔跑
	5.实现玩家跳跃
*/

#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <ctime>
#include <vector>
#include "tools.h"

using namespace std;

#define Window_Width 1012 //窗口宽度
#define Window_Height 396 //窗口高度
#define OBSTACLE_COUNT 10//总的障碍物数量
#define WIN_SCORE 100//胜利条件

clock_t start;
clock_t END;
clock_t ending;

IMAGE imgBgs[3];// 背景图
int bgX[3];//背景图片的x坐标是个变量
int bgSpeed[3] = { 1,2,4 };//背景图滚动速度

IMAGE imgHero[12];//角色图
int heroX;//角色的x坐标
int heroY;//角色的y坐标
int heroIndex;//角色图片帧序号
int heroHealth;//角色生命值
int score;//分数

IMAGE imgHeroDown[2];//角色下蹲图

IMAGE imgSZ[10];

//IMAGE imgTortoise;//乌龟
//int tortoiseX;//乌龟的水平坐标
//int tortoiseY;//乌龟的水平坐标
//bool tortoiseExist;//当前窗口是否有乌龟（用于保证只有一只）

bool heroJump;//表示角色是否正在跳跃
bool herodown;//表示角色是否正在下蹲

int jumpHeightMax;//最大跳跃高度
int heroJumpOff;//偏移量
bool update;//表示是否需要刷新

typedef enum {
	TORTOISE,//0
	LION,//1
	HOOK1,
	HOOK2,
	HOOK3,
	HOOK4,
	OBSTACLE_TYPE_COUNT//6
}obstacle_type;
	
vector<vector<IMAGE>> obstacleImgs;//相当于IMAGE obstacleImgs[][]，用于存放所有障碍物图片

typedef struct obstacle {
	int type;//障碍物的类型
	int imgIndex;
	int x, y;//障碍物的坐标
	int speed;//速度
	int damage;//伤害
	bool exist;
	bool hit;//是否发生碰撞
	bool passed;//是否越过障碍物
}obstacle_t;

obstacle_t obstacle[OBSTACLE_COUNT];//障碍物组
int lastObsIndex;

//初始化游戏
void init() {
	//创建游戏窗口
	initgraph(Window_Width, Window_Height,0);

	//加载背景资源
	char name[64];
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/bg%03d.png", i + 1);
		loadimage(&imgBgs[i], name);

		bgX[i] = 0;
	}

	//加载奔跑中角色的图片素材
	for (int i = 0; i < 12; i++) {
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHero[i], name);
	}

	//加载下蹲素材
		loadimage(&imgHeroDown[0], "res/d1.png");
		loadimage(&imgHeroDown[1], "res/d2.png");

	//加载小乌龟
	IMAGE imgTort;
	vector<IMAGE> imgTortArray;
	for (int i = 0; i < 7; i++) {
		sprintf(name, "res/t%d.png", i + 1);
		loadimage(&imgTort, name);
		imgTortArray.push_back(imgTort);
	}
	obstacleImgs.push_back(imgTortArray);

	//加载狮子
	IMAGE imgLion;
	vector<IMAGE> imgLionArray;
	for (int i = 0; i < 6; i++) {
		sprintf(name, "res/p%d.png", i + 1);
		loadimage(&imgLion, name);
		imgLionArray.push_back(imgLion);
	}
	obstacleImgs.push_back(imgLionArray);

	//加载柱子
	IMAGE imgHook;
	for (int i = 0; i < 4; i++) {
		vector<IMAGE> imgHookArray;
		sprintf(name, "res/h%d.png", i + 1);
		loadimage(&imgHook, name,63,260,true);
		imgHookArray.push_back(imgHook);
		obstacleImgs.push_back(imgHookArray);
	}

	//加载数字图片
	for (int i = 0; i < 10; i++) {
		sprintf(name, "res/sz/%d.png", i);
		loadimage(&imgSZ[i], name);
	}

	heroHealth = 100;
	heroX = Window_Width * 0.5 - imgHero[0].getwidth() * 0.5;
	heroY = 345 - imgHero[0].getheight();
	heroIndex = 0;

	heroJump = false;
	herodown = false;

	jumpHeightMax = 345 - imgHero[0].getheight() - 120;

	heroJumpOff = -4;

	update = false;

	//预加载音效
	preLoadSound("res/hit_female.mp3");
	mciSendString("play res/bg1.mp3 repeat", 0, 0, 0);
	
	lastObsIndex = -1;//记录上一个障碍物指标
	score = 0;

	//初始化障碍物池
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		obstacle[i].exist = false;
	}
	start = clock();//计时
}

//创建障碍物
void createObstacle() {
	int i;
	for (i = 0; i < OBSTACLE_COUNT; i++) {
		if (!obstacle[i].exist) {
			break;
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;
	}
	obstacle[i].exist = true;
	obstacle[i].hit = false;
	obstacle[i].imgIndex = 0;
	//obstacle[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacle[i].type = (obstacle_type)(rand() % 3);

	if (lastObsIndex>=0 && 
		obstacle[lastObsIndex].type >= HOOK1 
		&& obstacle[lastObsIndex].type <= HOOK4
		&& obstacle[i].type==LION 
		&& obstacle[lastObsIndex].x>(Window_Width-500)){
		obstacle[i].type = TORTOISE;
	}
	lastObsIndex = i;

	if (obstacle[i].type == HOOK1) {
		obstacle[i].type += rand() % 4;//0~3;
	}
	obstacle[i].x = Window_Width;
	obstacle[i].y = 345 + 5 - obstacleImgs[obstacle[i].type][0].getheight();
	if (obstacle[i].type == TORTOISE) {
		obstacle[i].speed = bgSpeed[2];
		obstacle[i].damage = 5;
	}
	else if (obstacle[i].type == LION) {
		obstacle[i].speed = bgSpeed[2]+4;
		obstacle[i].damage = 20;
	}
	else if (obstacle[i].type >= HOOK1 && obstacle[i].type <= HOOK4) {
		obstacle[i].speed = bgSpeed[2];
		obstacle[i].damage = 20;
		obstacle[i].y = 0;
	}

	obstacle[i].passed = false;
}

//渲染游戏背景
void updateBg() {
	putimagePNG2(bgX[0], 0, &imgBgs[0]);//前两个参数是x和y坐标
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}

//渲染英雄
void updateHero() {
	if (!herodown) {
		putimagePNG2(heroX, heroY, &imgHero[heroIndex]);
	}
	else {
		int y = 345 - imgHeroDown[heroIndex].getheight();
		putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
	}
}

//角色和障碍物碰撞检测
void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacle[i].exist && obstacle[i].hit==false) {
			int a1x, a1y, a2x, a2y;
			int off = 30;
			if (!herodown) {//非下蹲
				a1x = heroX + off;
				a1y = heroY + off;
				a2x = heroX + imgHero[heroIndex].getwidth() - off;
				a2y = heroY + imgHero[heroIndex].getheight();
			}
			else {
				a1x = heroX + off;
				a1y = 345 - imgHeroDown[heroIndex].getheight();
				a2x = heroX + imgHeroDown[heroIndex].getwidth() - off;
				a2y = 345;
			}

			int b1x = obstacle[i].x + off;
			int b1y = obstacle[i].y + off;
			int b2x = obstacle[i].x + obstacleImgs[obstacle[i].type][obstacle[i].imgIndex].getwidth() - off;
			int b2y = obstacle[i].y + obstacleImgs[obstacle[i].type][obstacle[i].imgIndex].getheight() - 10;

			if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y)) {
				heroHealth -= obstacle[i].damage;
				printf("health remain:%d\n", heroHealth);
				playSound("res/hit_female.mp3");
				obstacle[i].hit = true;
			}

		}
	}
}

//让背景和角色进行各种行动
void moveBg() {
	for (int i = 0; i < 3; i++) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -Window_Width) {
			bgX[i] = 0;
		}
	}

	//实现跳跃
	if (heroJump) {
		if (heroY < jumpHeightMax) {
			heroJumpOff = 4;
		}
		heroY += heroJumpOff;

		if (heroY > 345 - imgHero[0].getheight()) {
			heroJump = false;
			heroJumpOff = -4;
		}
	}
	else if (herodown) {
		static int count = 0;
		int delay[2] = { 8, 30 };
		count++;
		if(count>=delay[heroIndex]) {
			count = 0;
			heroIndex++;
			if (heroIndex > 2) {
				heroIndex = 0;
				herodown = false;
			}
		}
	}
	else {
		//不跳跃时，让角色奔跑
		heroIndex = (heroIndex + 1) % 12;
	}

	//创建障碍物
	static int frameCount = 0;//帧统计，随机enemyFre帧以后开始刷怪
	static int enemyFre = 70;
	frameCount++;
	srand(time(0));
	END = clock();
	int elapsed_time = (int)(END - start) / CLOCKS_PER_SEC;
	if (frameCount > enemyFre && elapsed_time < 45) {
		frameCount = 0;
		enemyFre = 80 + rand() % 80;//80~160
		createObstacle();
	}
	else if (frameCount > enemyFre && elapsed_time < 75) {
		frameCount = 0;
		enemyFre = 70 + rand() % 60;//70~120
		createObstacle();
	}
	else if (frameCount > enemyFre && elapsed_time >= 75) {
		frameCount = 0;
		enemyFre = 60 + rand() % 40;//60~100
		createObstacle();
	}

	//更新障碍物坐标
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacle[i].exist) {
			obstacle[i].x -= obstacle[i].speed;
			if (obstacle[i].x < -2 * (obstacleImgs[obstacle[i].type][0].getwidth())) {
				obstacle[i].exist = false;
			}

			int len = obstacleImgs[obstacle[i].type].size();
			obstacle[i].imgIndex = (obstacle[i].imgIndex + 1) % len;
		}
	}

	//碰撞检测处理
	checkHit();

}

//生成敌人（各种障碍物）
void updateEnemy() {
	//渲染小乌龟
	//if (tortoiseExist) {
	//	putimagePNG2(tortoiseX, tortoiseY,Window_Width,&imgTortoise);
	//}
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacle[i].exist) {
			putimagePNG2(obstacle[i].x, obstacle[i].y, Window_Width,
				&obstacleImgs[obstacle[i].type][obstacle[i].imgIndex]);
		}
	}
}

//跳跃函数开关
void jump() {
	heroJump = true;
	update = true;
	//printf("Jump action triggered!\n");
}

//下蹲函数开关
void down() {
	herodown = true;
	update = true;
	heroIndex = 0;
}

//接收用户键盘输入
void keyEvent() {
	char ch=' ';
	//判断有无输入，用conio.h中的_kbhit接口，有输入则返回1
	if (_kbhit()) {
		ch = _getch();//不需要回车来读取字符
		if (ch == ' ') {
			jump();
		}
		else if (ch == 's') {
			down();
		}
	}
}

void updateBloodBar() {
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, heroHealth / 100.0);
}

void checkOver() {
	if (heroHealth <= 0) {
		loadimage(0, "res/over1.png");
		FlushBatchDraw();
		mciSendString("stop res/bg1.mp3", 0, 0, 0);
		mciSendString("play res/failure.wav", 0, 0, 0);
		ending = clock();
		int elapse_time = (int)(ending - start) / CLOCKS_PER_SEC;
		printf("game over!\n");
		printf("consumed_time:%d\n", elapse_time);

		system("pause");
		//暂停后，开始下一局
		heroHealth = 100;
		score = 0;
		start = clock();
		ending = clock();
		END = clock();

		mciSendString("play res/bg1.mp3 repeat", 0, 0, 0);
	}
}

void calScore() {
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacle[i].exist &&
			obstacle[i].passed == false &&
			obstacle[i].hit == false && 
			obstacle[i].x+obstacleImgs[obstacle[i].type][0].getwidth()< heroX ) {
			score++;
			obstacle[i].passed = true;
			printf("score:%d\n", score);
		}
	}
}

void updateScore() {
	char str[8];
	int x = 20, y = 25;
	sprintf(str, "%d", score);
	for (int i = 0; str[i]; i++) {
		int sz = str[i] - '0';
		putimagePNG(x, y, &imgSZ[sz]);
		x += imgSZ[i].getwidth()+5;
	}
}

void checkWin() {
	if (score >= WIN_SCORE) {
		FlushBatchDraw();
		loadimage(0, "res/win.png");
		mciSendString("stop res/bg1.mp3", 0, 0, 0);
		mciSendString("play res/win1.wav", 0, 0, 0);
		Sleep(1000);
		FlushBatchDraw();
		system("pause");
		//暂停后，开始下一局
		heroHealth = 100;
		score = 0;
		start = clock();
		ending = clock();
		END = clock();

		mciSendString("play res/bg1.mp3 repeat", 0, 0, 0);
	}
}

int main(void) {
	init();

	int timer = 0;

	while(1){
		keyEvent();
		timer+=getDelay();//距离上一次调用多少毫秒
		if (timer > 30) {
			timer = 0;
			update = true;
		}
		if(update){
			update = false;
			BeginBatchDraw();//提前加载资源
			updateBg();
			updateHero();
			//putimagePNG2(heroX, heroY, &imgHero[heroIndex]);
			updateEnemy();
			updateBloodBar();//显示血条
			updateScore();
			checkWin();
			EndBatchDraw();

			checkOver();//判断是否游戏结束
			calScore();
			
			moveBg();
		}
		
		//Sleep(30);
	}

	system("pause");
	return 0;
}