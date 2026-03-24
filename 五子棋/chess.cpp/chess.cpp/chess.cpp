#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <graphics.h> 

#define SIZE 15

int board[SIZE][SIZE] = {0};

void gotoxy(int x, int y) {
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 控制台绘制棋盘
void drawBoard() {
    system("cls");
    printf("  "); 
    for (int i = 0; i < SIZE; i++) {
        printf("%2d", i); 
    }
    printf("\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d", i);
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 1)
                printf("● ");
            else if (board[i][j] == 2)
                printf("○ ");
            else
                printf("十");
        }
        printf("\n");
    }
    printf("请输入：行 列 → \n");
}

// 图形窗口绘制棋子
void drawPiecesOnWindow() {
    int GRID = 30;
    int OFFSET = 40;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int x = OFFSET + j * GRID;
            int y = OFFSET + i * GRID;
            if (board[i][j] == 1) { // 黑棋
                setfillcolor(BLACK);
                solidcircle(x, y, 12);
            } else if (board[i][j] == 2) { // 白棋
                setfillcolor(WHITE);
                solidcircle(x, y, 12);
                setlinecolor(BLACK);
                circle(x, y, 12);
            }
        }
    }
}

// 判断胜负
int checkWin(int x, int y, int player) {
    int dir[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};
    for (int d = 0; d < 4; d++) {
        int dx = dir[d][0];
        int dy = dir[d][1];
        int count = 1;
        for (int i = 1; i < 5; i++) {
            int nx = x + dx * i;
            int ny = y + dy * i;
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] == player)
                count++;
            else
                break;
        }
        for (int i = 1; i < 5; i++) {
            int nx = x - dx * i;
            int ny = y - dy * i;
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] == player)
                count++;
            else
                break;
        }
        if (count >= 5)
            return 1;
    }
    return 0;
}

int main() {
    // 初始化图形窗口
    initgraph(640, 480);
    setbkcolor(RGB(222, 184, 135));
    cleardevice();
    setlinecolor(BLACK);
    int GRID = 30;
    int OFFSET = 40;
    for (int i = 0; i < SIZE; i++) {
        line(OFFSET, OFFSET + i * GRID, OFFSET + (SIZE-1)*GRID, OFFSET + i * GRID);
        line(OFFSET + i * GRID, OFFSET, OFFSET + i * GRID, OFFSET + (SIZE-1)*GRID);
    }

    int x, y;
    int player = 1;

    while (1) {
        drawBoard();
        drawPiecesOnWindow(); 

        printf("%s棋下棋：", player == 1 ? "黑" : "白");
        scanf("%d %d", &x, &y);

        if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
            printf("输入超出范围！按回车重试...\n");
            system("pause");
            continue;
        }

        if (board[x][y] != 0) {
            printf("这里已有棋子！按回车重试...\n");
            system("pause");
            continue;
        }

        board[x][y] = player;

        if (checkWin(x, y, player)) {
            drawBoard();
            drawPiecesOnWindow(); 
            printf("%s棋胜利！游戏结束\n", player == 1 ? "黑" : "白");
            break;
        }

        player = (player == 1) ? 2 : 1;
    }

    system("pause");
    closegraph(); 
    return 0;
}