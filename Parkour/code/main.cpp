/*
	�ܿ���Ϸ������־������EasyX�⣩
	1.������Ŀ
	2.�����ز�
	3.ʵ����Ϸ����
	4.ʵ����ұ���
	5.ʵ�������Ծ
*/

#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <ctime>
#include <vector>
#include "tools.h"

using namespace std;

#define Window_Width 1012 //���ڿ��
#define Window_Height 396 //���ڸ߶�
#define OBSTACLE_COUNT 10//�ܵ��ϰ�������
#define WIN_SCORE 100//ʤ������

clock_t start;
clock_t END;
clock_t ending;

IMAGE imgBgs[3];// ����ͼ
int bgX[3];//����ͼƬ��x�����Ǹ�����
int bgSpeed[3] = { 1,2,4 };//����ͼ�����ٶ�

IMAGE imgHero[12];//��ɫͼ
int heroX;//��ɫ��x����
int heroY;//��ɫ��y����
int heroIndex;//��ɫͼƬ֡���
int heroHealth;//��ɫ����ֵ
int score;//����

IMAGE imgHeroDown[2];//��ɫ�¶�ͼ

IMAGE imgSZ[10];

//IMAGE imgTortoise;//�ڹ�
//int tortoiseX;//�ڹ��ˮƽ����
//int tortoiseY;//�ڹ��ˮƽ����
//bool tortoiseExist;//��ǰ�����Ƿ����ڹ꣨���ڱ�ֻ֤��һֻ��

bool heroJump;//��ʾ��ɫ�Ƿ�������Ծ
bool herodown;//��ʾ��ɫ�Ƿ������¶�

int jumpHeightMax;//�����Ծ�߶�
int heroJumpOff;//ƫ����
bool update;//��ʾ�Ƿ���Ҫˢ��

typedef enum {
	TORTOISE,//0
	LION,//1
	HOOK1,
	HOOK2,
	HOOK3,
	HOOK4,
	OBSTACLE_TYPE_COUNT//6
}obstacle_type;
	
vector<vector<IMAGE>> obstacleImgs;//�൱��IMAGE obstacleImgs[][]�����ڴ�������ϰ���ͼƬ

typedef struct obstacle {
	int type;//�ϰ��������
	int imgIndex;
	int x, y;//�ϰ��������
	int speed;//�ٶ�
	int damage;//�˺�
	bool exist;
	bool hit;//�Ƿ�����ײ
	bool passed;//�Ƿ�Խ���ϰ���
}obstacle_t;

obstacle_t obstacle[OBSTACLE_COUNT];//�ϰ�����
int lastObsIndex;

//��ʼ����Ϸ
void init() {
	//������Ϸ����
	initgraph(Window_Width, Window_Height,0);

	//���ر�����Դ
	char name[64];
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/bg%03d.png", i + 1);
		loadimage(&imgBgs[i], name);

		bgX[i] = 0;
	}

	//���ر����н�ɫ��ͼƬ�ز�
	for (int i = 0; i < 12; i++) {
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHero[i], name);
	}

	//�����¶��ز�
		loadimage(&imgHeroDown[0], "res/d1.png");
		loadimage(&imgHeroDown[1], "res/d2.png");

	//����С�ڹ�
	IMAGE imgTort;
	vector<IMAGE> imgTortArray;
	for (int i = 0; i < 7; i++) {
		sprintf(name, "res/t%d.png", i + 1);
		loadimage(&imgTort, name);
		imgTortArray.push_back(imgTort);
	}
	obstacleImgs.push_back(imgTortArray);

	//����ʨ��
	IMAGE imgLion;
	vector<IMAGE> imgLionArray;
	for (int i = 0; i < 6; i++) {
		sprintf(name, "res/p%d.png", i + 1);
		loadimage(&imgLion, name);
		imgLionArray.push_back(imgLion);
	}
	obstacleImgs.push_back(imgLionArray);

	//��������
	IMAGE imgHook;
	for (int i = 0; i < 4; i++) {
		vector<IMAGE> imgHookArray;
		sprintf(name, "res/h%d.png", i + 1);
		loadimage(&imgHook, name,63,260,true);
		imgHookArray.push_back(imgHook);
		obstacleImgs.push_back(imgHookArray);
	}

	//��������ͼƬ
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

	//Ԥ������Ч
	preLoadSound("res/hit_female.mp3");
	mciSendString("play res/bg1.mp3 repeat", 0, 0, 0);
	
	lastObsIndex = -1;//��¼��һ���ϰ���ָ��
	score = 0;

	//��ʼ���ϰ����
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		obstacle[i].exist = false;
	}
	start = clock();//��ʱ
}

//�����ϰ���
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

//��Ⱦ��Ϸ����
void updateBg() {
	putimagePNG2(bgX[0], 0, &imgBgs[0]);//ǰ����������x��y����
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}

//��ȾӢ��
void updateHero() {
	if (!herodown) {
		putimagePNG2(heroX, heroY, &imgHero[heroIndex]);
	}
	else {
		int y = 345 - imgHeroDown[heroIndex].getheight();
		putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
	}
}

//��ɫ���ϰ�����ײ���
void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacle[i].exist && obstacle[i].hit==false) {
			int a1x, a1y, a2x, a2y;
			int off = 30;
			if (!herodown) {//���¶�
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

//�ñ����ͽ�ɫ���и����ж�
void moveBg() {
	for (int i = 0; i < 3; i++) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -Window_Width) {
			bgX[i] = 0;
		}
	}

	//ʵ����Ծ
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
		//����Ծʱ���ý�ɫ����
		heroIndex = (heroIndex + 1) % 12;
	}

	//�����ϰ���
	static int frameCount = 0;//֡ͳ�ƣ����enemyFre֡�Ժ�ʼˢ��
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

	//�����ϰ�������
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

	//��ײ��⴦��
	checkHit();

}

//���ɵ��ˣ������ϰ��
void updateEnemy() {
	//��ȾС�ڹ�
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

//��Ծ��������
void jump() {
	heroJump = true;
	update = true;
	//printf("Jump action triggered!\n");
}

//�¶׺�������
void down() {
	herodown = true;
	update = true;
	heroIndex = 0;
}

//�����û���������
void keyEvent() {
	char ch=' ';
	//�ж��������룬��conio.h�е�_kbhit�ӿڣ��������򷵻�1
	if (_kbhit()) {
		ch = _getch();//����Ҫ�س�����ȡ�ַ�
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
		//��ͣ�󣬿�ʼ��һ��
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
		//��ͣ�󣬿�ʼ��һ��
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
		timer+=getDelay();//������һ�ε��ö��ٺ���
		if (timer > 30) {
			timer = 0;
			update = true;
		}
		if(update){
			update = false;
			BeginBatchDraw();//��ǰ������Դ
			updateBg();
			updateHero();
			//putimagePNG2(heroX, heroY, &imgHero[heroIndex]);
			updateEnemy();
			updateBloodBar();//��ʾѪ��
			updateScore();
			checkWin();
			EndBatchDraw();

			checkOver();//�ж��Ƿ���Ϸ����
			calScore();
			
			moveBg();
		}
		
		//Sleep(30);
	}

	system("pause");
	return 0;
}