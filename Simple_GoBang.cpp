/************************************************************************/
/*
 *			位置坐标：从1开始，36进制
 *			相关知识：类的继承与派生、纯虚函数
 *			通过输入 1 或 0 来决定人机对战还是人人对战
 *			可以修改参数来提升挑战级别
 *			本程序中五子棋规则为无禁手
 *
/************************************************************************/

#include <iostream>
#include <cstring>
#include <random>
#include <vector>
using namespace std;

/*以下为可手动修改的参数（包括实验性设置）*/
//=============================================================================
bool Computer = 0;      // 玩家2 为 电脑true 还是 人false
const int HEIGHT = 25;  // 棋盘高度，可修改，最大35（因为坐标是从1开始的36进制字符）
const int WIDTH = 25;   // 棋盘宽度，可修改，最大35
double Tolerance = 0.1; // 随机容差，1为全部随机，0为最严格
int useLevel = 0;       // 使用多层权重，开启后不稳定
//=============================================================================

char Log[10000] = { 0 }; // 记录日志，调试错误
void log(char * s) { strcat(Log, "\n"); strcat(Log, s); }
void showLog() { cout << Log << endl; }

enum CHES // 玩家棋子的值
{
	//双人五子棋就3个值，但是可以补充一下用于多人五子棋、象棋之类
	Cp0 = 0, // 默认0
	Cp1 = 1, // 玩家1
	Cp2 = 2,  // 玩家2或者电脑
	Cp999 = 9 // 临时表示假设落子后的值
};

class Point // 点类
{
public:
	int x, y;
	Point() { Point(0, 0); }
	Point(int xx, int yy) : x(xx), y(yy) {}
	Point(char xx, char yy) : x(xx), y(yy) {}
	Point(Point &pot) : x(pot.x), y(pot.y) {}
};

class ChessBoard // 棋盘类
{
protected:
	CHES lattice[HEIGHT + 1][WIDTH + 1] = { Cp0 }; // 棋盘格子的值：空0, 玩家1, 玩家2（电脑）
	int usedLattice = 0; // 已经使用的格子个数，用来判断是不是满的

public:

	ChessBoard() // 格子初始化成0
	{
		memset(lattice, 0, sizeof(lattice));
		usedLattice = 0;
	}

	bool isFull() // 判断格子是否已经满了
	{
		return usedLattice == HEIGHT * WIDTH;
	}

	int addPiece(Point pot, CHES val) // 落子的时候添加格子
	{
		if (pot.x <= 0 || pot.y <= 0 || pot.x > HEIGHT || pot.y > WIDTH)
		{
			cout << "位置越界：" << pot.x << ", " << pot.y << endl;
			return 0;
		}
		else if (lattice[pot.x][pot.y]) // 已经有棋子了
		{
			cout << "已经落子：" << pot.x << ", " << pot.y << endl;
			return 0;
		}
		lattice[pot.x][pot.y] = val;
		usedLattice++;
		return 1;
	}

	char getLattice(int x, int y)
	{
		return (char)(lattice[x][y] + '0');
	}
	CHES getLattice(Point pot)
	{
		return lattice[pot.x][pot.y];
	}

	void showBoard() // 打印棋盘（附带坐标）
	{
		system("cls"); // 清屏
		int i;
		for (i = 0; i <= WIDTH; i++)
			cout << toChar36(i) << ' ';
		cout << endl;
		for (i = 1; i <= HEIGHT; i++)
		{
			for (int j = 0; j <= WIDTH; j++)
			{
				if (j == 0)
					cout << toChar36(i) << ' ';
				else
					cout << toPiece(lattice[i][j]) << ' ';
			}
			cout << endl;
		}

		cout << "请输入对应的 x y 坐标（不分大小写）：" << endl;
	}

	char toChar36(int x) // 转到36进制坐标（可通过修改这个函数来调整棋盘最大值）
	{
		if (x < 10)
			return x + '0';
		else
			return x - 10 + 'A';
	}

	char toPiece(int x) // 0、1、2 转化成棋子格式
	{
		if (x == 0) return '.';
		if (x == 1) return 'O';
		if (x == 2) return '*';
		return 'X';
	}
};

class Chess // 棋类
{
protected:
	int turn = 0; // 谁的回合，初始0，玩家1，玩家2（电脑）。对应函数：nextTurn()

public:
	Chess() : turn(0) {} // 初始化棋子

	int nextTurn() // 下一回合
	{
		if (turn == 0) return turn = 1;
		if (turn == 1) return turn = 2;
		if (turn == 2) return turn = 1;
		return -1;
	}

	virtual int putPiece(Point, CHES) = 0;// 落子（玩家直接落子，电脑先获取最合适的位置再落子）

	virtual int judgeVictory(Point, CHES) = 0; // 是否判断出胜利，根据某个位置判断胜利
};

class FiveChess : public ChessBoard, public Chess // 五子棋类，从棋类和棋盘上派生
{
public:
	int victory = 0; // 是否胜利，胜利1，失败-1，平局默认0（针对玩家1来说）

	FiveChess() {}

	int putPiece(Point pot, CHES val) // 落子，成功1，失败0，结果2（玩家直接落子，电脑先获取最合适的位置再落子）
	{
		if (addPiece(pot, val))
		{
			if (judgeVictory(pot, val))
				return 2;

			return 1;
		}
		return 0;
	}

	int judgeVictory(Point pot, CHES val) // 是否判断出胜利，从 pot 点开始上下左右斜4种情况判断
	{
		if (isFull()) {
			victory = 0;
			return 1;
		}
		if (judge19(pot, val) || judge28(pot, val) || judge37(pot, val) || judge46(pot, val))
		{
			if (val == Cp1)
				victory = 1;
			else
				victory = -1;

			return 1;
		}
		return 0;
	}

	/*判断方向
	*123
	*456
	*789
	**/

	int judge19(Point pot, CHES val) // \……
	{
		char str[20] = { 0 };
		int len = 0, i;
		for (int i = -4; i <= 4; i++)
		{
			if (pot.x + i > 0 && pot.x + i <= HEIGHT && pot.x > 0 && pot.x <= HEIGHT
				&& pot.y > 0 && pot.y <= WIDTH && pot.y + i > 0 && pot.y + i <= WIDTH)
				str[len++] = (char)getLattice(pot.x + i, pot.y + i);
		}
		if (val == Cp1)
		{
			if (strstr(str, "11111") || strstr(str, "011110"))
				return 1;
		}
		else
		{
			if (strstr(str, "22222") || strstr(str, "022220"))
				return 1;
		}
		return 0;
	}
	int judge28(Point pot, CHES val) // |
	{
		char str[20] = { 0 };
		int len = 0;
		for (int i = -4; i <= 4; i++)
		{
			if (pot.x + i > 0 && pot.x + i <= HEIGHT && pot.x > 0 && pot.x <= HEIGHT)
				str[len++] = (char)getLattice(pot.x + i, pot.y);
		}
		if (val == Cp1)
		{
			if (strstr(str, "11111") || strstr(str, "011110"))
				return 1;
		}
		else
		{
			if (strstr(str, "22222") || strstr(str, "022220"))
				return 1;
		}
		return 0;
	}
	int judge37(Point pot, CHES val) // /
	{
		char str[20] = { 0 };
		int len = 0;
		for (int i = -4; i <= 4; i++)
		{
			if (pot.x - i > 0 && pot.x - i <= HEIGHT && pot.x > 0 && pot.x <= HEIGHT
				&& pot.y > 0 && pot.y <= WIDTH && pot.y + i > 0 && pot.y + i <= WIDTH)
				str[len++] = (char)getLattice(pot.x - i, pot.y + i);
		}
		if (val == Cp1)
		{
			if (strstr(str, "11111") || strstr(str, "011110"))
				return 1;
		}
		else
		{
			if (strstr(str, "22222") || strstr(str, "022220"))
				return 1;
		}
		return 0;
	}
	int judge46(Point pot, CHES val) // -
	{
		char str[20] = { 0 };
		int len = 0;
		for (int i = -4; i <= 4; i++)
		{
			if (pot.y > 0 && pot.y <= WIDTH && pot.y + i > 0 && pot.y + i <= WIDTH)
				str[len++] = getLattice(pot.x, pot.y + i);
		}
		if (val == Cp1)
		{
			if (strstr(str, "11111") || strstr(str, "011110"))
				return 1;
		}
		else
		{
			if (strstr(str, "22222") || strstr(str, "022220"))
				return 1;
		}
		return 0;
	}

	/*智能AI*/
	Point getSuitPoint();
	int explorePoint(int x, int y);
	int exploreCase(char *);

	bool isInTol(int a, int b) // 根据权重和容差判断一个点能不能落子(a >= b)
	{
		b = a - b;
		if (b <= 0) b = -b;
		return (double)b / a < Tolerance;
	}
};

class FiveChessGame : public FiveChess // 五子棋游戏类
{
private:
	const int P1TURN = 1;
	const int P2TURN = 2;

public:
	FiveChessGame() {}

	void startGame()
	{
		showBoard();

		runGame();

		showEnd();
	}

	void runGame()
	{
		char chx, chy;

		if (!Computer) // 如果是【玩家对战】
		{
			while (cin >> chx >> chy)
			{
				chx = charToInt(chx);
				chy = charToInt(chy);

				if (nextTurn() == P1TURN)
				{
					int cas = putPiece(Point(chx, chy), Cp1); // 玩家1落子
					if (cas == 0) { nextTurn(); continue; } // 落子失败
					if (cas == 2) return; // 判断出胜负
				}
				else
				{
					int cas = putPiece(Point(chx, chy), Cp2); // 玩家2落子
					if (cas == 0) { nextTurn(); continue; } // 落子失败
					if (cas == 2) return; // 判断出胜负
				}

				showBoard(); // 棋盘刷新

				cout << "上次落子：" << (int)chx << ", " << (int)chy << endl;

				if (turn == P1TURN)
					cout << "玩家2(*) 请落子：";
				else
					cout << "玩家1(O) 请落子：";
			}
		}
		else // 如果是【人机对战】
		{
			while (cin >> chx >> chy)
			{
				chx = charToInt(chx);
				chy = charToInt(chy);

				int cas = putPiece(Point(chx, chy), Cp1); // 玩家1落子
				if (cas == 0) continue; // 落子失败
				if (cas == 2) return; // 判断出胜负

				Point pot(getSuitPoint());
				cas = putPiece(pot, Cp2); // 玩家2落子
				if (cas == 0) continue; // 落子失败
				if (cas == 2) return; // 判断出胜负

				showBoard(); // 棋盘刷新

				cout << "您的上次落子：" << toChar36(chx) << ", " << toChar36(chy) << endl;
				cout << "电脑上次落子：" << toChar36(pot.x) << ", " << toChar36(pot.y) << endl;
			}
		}
	}

	void showEnd()
	{
		showBoard();

		cout << "============================\n" << endl;
		if (victory == 0)
		{
			cout << "        平局        " << endl;
		}
		else if (victory == 1)
		{
			cout << "    恭喜【玩家1】胜利啦    " << endl;
		}
		else if (victory == -1)
		{
			if (Computer)
				cout << "    很遗憾，再来一局    " << endl;
			else
				cout << "    恭喜【玩家2】胜利啦    " << endl;
		}
		else
		{
			cout << "    貌似出现了点小错误    " << endl;
		}
		cout << "\n============================" << endl;
	}

	char charToInt(char ch) // 因为返回后用回 ch，所以返回值依旧是 char
	{
		if (ch >= '0' && ch <= '9') ch -= '0';
		else if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 10;
		else ch = 0;
		return ch;
	}

	char toChar36(int x) // 转到36进制坐标（可通过修改这个函数来调整棋盘最大值）
	{
		if (x < 10)
			return x + '0';
		else
			return x - 10 + 'A';
	}
};

Point FiveChess::getSuitPoint() // AI获取最适合的位置（电脑），暂时是随机位置
{
	int i, j;
	int maxscore = 0;
	int score[HEIGHT + 10][WIDTH + 10] = { 0 };
	for (i = 1; i <= HEIGHT; i++)
	{
		for (j = 1; j <= WIDTH; j++)
		{
			if (lattice[i][j] == Cp0)
			{
				score[i][j] = explorePoint(i, j);
				if (maxscore < score[i][j])
				{
					maxscore = score[i][j];
				}
			}
		}
	}

	// 获取等权值的随机位置
	vector < Point > pots;
	int num = 0;
	for (i = 1; i <= HEIGHT; i++)
	{
		for (j = 1; j <= WIDTH; j++)
		{
			if (isInTol(maxscore, score[i][j]))
			{
				pots.push_back(Point(i, j));
				num++;
			}
		}
	}
	int randNum = rand() % num;//rand() % pots.size + 1;
	Point pot = pots[randNum];

	/*srand(time(0)); // 使用随机位置
	return Point(rand() % (HEIGHT - 1) + 1, rand() % (WIDTH - 1) + 1);*/
	return Point(pot.x, pot.y);
}

int FiveChess::explorePoint(int x, int y) // 获取该位置的权值（值为2）
{
	// 假设在这个地方落子
	lattice[x][y] = Cp999;

	/*判断方向
	*123
	*456
	*789
	**/
	// 获取棋子布局
	int score = 0; // 总成绩
	char str[100] = { 0 };
	int len = 0;
	int i;
	for (i = -4; i <= 4; i++) // \.
	{
		if (x + i > 0 && x + i <= HEIGHT && y + i > 0 && y + i <= WIDTH)
			str[len++] = (char)lattice[x + i][y + i] + '0';
	}
	str[len++] = 'X'; // 分割
	for (i = -4; i <= 4; i++) // |
	{
		if (x + i > 0 && x + i <= HEIGHT)
			str[len++] = (char)lattice[x + i][y] + '0';
	}
	str[len++] = 'X'; // 分割
	for (i = -4; i <= 4; i++) // /
	{
		if (x - i > 0 && x - i <= HEIGHT && y + i > 0 && y + i <= WIDTH)
			str[len++] = (char)lattice[x - i][y + i] + '0';
	}
	str[len++] = 'X'; // 分割
	for (i = -4; i <= 4; i++) // -
	{
		if (y + i > 0 && y + i <= WIDTH)
			str[len++] = (char)lattice[x][y + i] + '0';
	}

	score = exploreCase(str);

	// 恢复空白
	lattice[x][y] = Cp0;

	return score;
}

int FiveChess::exploreCase(char * str) // 根据布局分析权值
{
	int score = 0;
	int level = 0;

	/*进攻（自己）：
	*10000 成5：构成五子连珠
	*100 活4：构成两边都不被拦截的四子连珠
	*10 死4：一边被拦截的四子连珠
	*8 活3：两边不被拦截的三子连珠
	*4 死3：一边被拦截的三子连珠
	*4 活2：两边均不被拦截的二子连珠
	*2 死2：一边被拦截的二子连珠
	*1 单子：四周无相连子棋
	*0 无子：第一个棋子
	**/

	// 10000 成5：构成五子连珠
	if (strstr(str, "92222")) score += (useLevel ? ++level : 1) * 10000;
	if (strstr(str, "29222")) score += (useLevel ? ++level : 1) * 10000;
	if (strstr(str, "22922")) score += (useLevel ? ++level : 1) * 10000;
	if (strstr(str, "22292")) score += (useLevel ? ++level : 1) * 10000;
	if (strstr(str, "22229")) score += (useLevel ? ++level : 1) * 10000;
	// 100 活4：构成两边都不被拦截的四子连珠（加上 死4）
	if (strstr(str, "092220")) score += (useLevel ? ++level : 1) * 100;
	if (strstr(str, "029220")) score += (useLevel ? ++level : 1) * 100;
	if (strstr(str, "022920")) score += (useLevel ? ++level : 1) * 100;
	if (strstr(str, "022290")) score += (useLevel ? ++level : 1) * 100;
	// 10 死4：一边被拦截的四子连珠
	if (strstr(str, "09222")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "02922")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "02292")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "02229")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "92220")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "29220")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "22920")) score += (useLevel ? ++level : 1) * 10;
	if (strstr(str, "22290")) score += (useLevel ? ++level : 1) * 10;
	// 8 活3：两边不被拦截的三子连珠（加上 死3）
	if (strstr(str, "09220")) score += (useLevel ? ++level : 1) * 8;
	if (strstr(str, "02920")) score += (useLevel ? ++level : 1) * 8;
	if (strstr(str, "02290")) score += (useLevel ? ++level : 1) * 8;
	// 7 011090
	if (strstr(str, "022090")) score += (useLevel ? ++level : 1) * 7;
	if (strstr(str, "090220")) score += (useLevel ? ++level : 1) * 7;
	// 4 死3：一边被拦截的三子连珠
	if (strstr(str, "9220")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "2920")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "2290")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "0922")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "0292")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "0229")) score += (useLevel ? ++level : 1) * 4;
	// 4 活2：两边均不被拦截的二子连珠（加上 死2）
	if (strstr(str, "0290")) score += (useLevel ? ++level : 1) * 4;
	if (strstr(str, "0920")) score += (useLevel ? ++level : 1) * 4;
	// 2 死2：一边被拦截的二子连珠
	if (strstr(str, "290")) score += (useLevel ? ++level : 1) * 2;
	if (strstr(str, "920")) score += (useLevel ? ++level : 1) * 2;
	if (strstr(str, "029")) score += (useLevel ? ++level : 1) * 2;
	if (strstr(str, "092")) score += (useLevel ? ++level : 1) * 2;
	// 1 单子：四周无相连子棋
	if (strstr(str, "09000")) score += (useLevel ? ++level : 1) * 1;
	if (strstr(str, "00900")) score += (useLevel ? ++level : 1) * 1;
	if (strstr(str, "00090")) score += (useLevel ? ++level : 1) * 1;
	// 0 无子：第一个棋子
	if (strstr(str, "00900")) score += (useLevel ? ++level : 1) * 1;

	/*防守（对方）：
	*500 死4
	*50 活3
	*5 死3
	*3 活2
	**/
	// 500 死4
	level = 0;
	if (strstr(str, "91111")) score += (useLevel ? ++level : 1) * 500;
	if (strstr(str, "19111")) score += (useLevel ? ++level : 1) * 500;
	if (strstr(str, "11911")) score += (useLevel ? ++level : 1) * 500;
	if (strstr(str, "11191")) score += (useLevel ? ++level : 1) * 500;
	if (strstr(str, "11119")) score += (useLevel ? ++level : 1) * 500;
	// 50 活3（加上死3）
	if (strstr(str, "91110")) score += (useLevel ? ++level : 1) * 50;
	if (strstr(str, "01119")) score += (useLevel ? ++level : 1) * 50;
	// 5 死3
	if (strstr(str, "9111")) score += (useLevel ? ++level : 1) * 5;
	if (strstr(str, "1119")) score += (useLevel ? ++level : 1) * 5;
	// 3 活2
	if (strstr(str, "0910")) score += (useLevel ? ++level : 1) * 3;
	if (strstr(str, "0190")) score += (useLevel ? ++level : 1) * 3;

	/*其他判断（综合）
	*30 双活2
	*
	**/

	char * pos = 0;
	if ((pos = strstr(str, "01910")) && strstr(pos, "01910")) score += (useLevel ? ++level : 1) * 30;

	return score;
}

int main()
{
	cout << "请输入对战方式：\n0玩家对战   1人机对战" << endl;
	while (cin >> Computer)
	{
		while (!(Computer == 1 || Computer == 0)) cin >> Computer;

		FiveChessGame fcg;

		fcg.startGame();

		cout << "请输入对战方式：\n0玩家对战   1人机对战" << endl;
	}

	system("pause");
	return 0;
}