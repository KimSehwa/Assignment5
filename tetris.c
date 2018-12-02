/* tetris-term - Classic Tetris for your terminal.
 *
 * Copyright (C) 2014 Gjum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tetris.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// {{{ bricks
#define numBrickTypes 7
// positions of the filled cells     
//4*4 사각형    
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// [brickNr][rotation][cellNr]
const unsigned char bricks[numBrickTypes][4][4] = {
	{ { 1,  5,  9, 13}, { 8,  9, 10, 11}, { 1,  5,  9, 13}, { 8,  9, 10, 11}, }, // │모양      //7개의 블록을 정의한다.
	{ { 5,  6,  9, 10}, { 5,  6,  9, 10}, { 5,  6,  9, 10}, { 5,  6,  9, 10}, }, // Γ모양
	{ { 9,  8,  5, 10}, { 9,  5, 10, 13}, { 9, 10, 13,  8}, { 9, 13,  8,  5}, }, // ┻모양
	{ { 9, 10, 12, 13}, { 5,  9, 10, 14}, { 9, 10, 12, 13}, { 5,  9, 10, 14}, }, // ┌ 모양
	                                                                             // ┘
	{ { 8,  9, 13, 14}, { 5,  8,  9, 12}, { 8,  9, 13, 14}, { 5,  8,  9, 12}, }, // ┐ 모양
	                                                                             // └
	{ { 5,  9, 12, 13}, { 4,  8,  9, 10}, { 5,  6,  9, 13}, { 8,  9, 10, 14}, }, // ┙모양
	{ { 5,  9, 13, 14}, { 8,  9, 10, 12}, { 4,  5,  9, 13}, { 6,  8,  9, 10}, }, // ┗모양
};
// }}}

static void Excess_Memory(void *pointer) { // {{{
	if (pointer == NULL){      //pointer에 값이 NULL일 경우
		printf("Error: Out of memory\n");
		exit(1);
	}
} // }}}

static void nextBrick(TetrisGame *game) { // {{{
	game->brick = game->nextBrick;
	game->brick.x = game->width/2 - 2;             //x값의 초기값을 지정.                        //초기위치
	game->brick.y = 0;                       //y값의 초기위치를 지정.
	game->nextBrick.type = rand() % numBrickTypes;     
	game->nextBrick.rotation = rand() % 4;      
	game->nextBrick.color = game->brick.color % 7 + 1; // (color-1 + 1) % 7 + 1, range is 1..7
	// 블록의 색 1~7(int color)  (1:BLUE, 2:GREEN, 3:JADE, 4:RED, 5:PURPLE, 6:YELLOW, 7:WHITE)
	game->nextBrick.x = 0;                 //초기화 시킴 
	game->nextBrick.y = 0;                 
} // }}}

TetrisGame *newTetrisGame(unsigned int width, unsigned int height) { // {{{
	TetrisGame *game = malloc(sizeof(TetrisGame));
	Excess_Memory(game);
	game->width = width;                      //게임판의 너비,높이,사이즈,일시정지,점수 표시를 위해 포인터값을 지정
	game->height = height;
	game->size = width * height;
	game->board = calloc(game->size, sizeof(char));
	Excess_Memory(game->board);
	game->isRunning = 1;
	game->isPaused  = 0;
	game->sleepUsec = 500000;
	game->score = 0;
	nextBrick(game); // 다음 블럭을 채운다.
	nextBrick(game); // 다음 블럭을 게임 안에 채운다.
	// init terminal for non-blocking and no-echo getchar()
	struct termios term;
	tcgetattr(STDIN_FILENO, &game->termOrig);
	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	// timer 와 error에 대한 signal 들을 초기화한다.
	struct sigaction signalAction;                     //sigaction -> 어떤 신호가 생길 경우 그에 따라 행동을 할 수 있도록 정의해주는 함수.
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_handler = signalHandler;
	signalAction.sa_flags = 0;
	sigaction(SIGINT,  &signalAction, NULL);
	sigaction(SIGTERM, &signalAction, NULL);           
	sigaction(SIGSEGV, &signalAction, NULL);
	sigaction(SIGALRM, &signalAction, NULL);          
	// timer을 초기화한다.
	game->timer.it_value.tv_usec = game->sleepUsec;
	setitimer(ITIMER_REAL, &game->timer, NULL);
	return game;
} // }}}

/*게임 종료 시키는 함수*/
void destroyTetrisGame(TetrisGame *game) { // {{{
	if (game == NULL) return;                             //game의 생성 오류 시 종료
	tcsetattr(STDIN_FILENO, TCSANOW, &game->termOrig);    //터미널 출력를 갱신
	printf("Your score: %i\n", game->score);              //점수 출력
	printf("Game over.\n");                               //"Game over" 출력
	free(game->board);                                    //게임 패턴값의 메모리 해제
	free(game);                                           //게임 메모리 해제  
} // }}}

/*입력한 x, y 좌표의 색을 반환*/
/*반환값 : 브릭의 색 or 0*/
unsigned char colorOfBrickAt(FallingBrick *brick, int x, int y) { // {{{
	if (brick->type < 0) return 0;                        //brick의 생성 오류 시 종료
	int v = y - brick->y; 
	if (v < 0 || v >= 4) return 0;                        //y좌표 값이 브릭과 만나지 않을 경우 0 반환
	int u = x - brick->x;
	if (u < 0 || u >= 4) return 0;                        //x좌표 값이 브릭과 만나지 않을 경우 0 반환
	for (int i = 0; i < 4; i++) {                         //x, y 좌표를 기준 4*4 크기의 주변 좌표와 브릭이 만나는지 확인 
		if (u + 4*v == bricks[brick->type][brick->rotation][i])
			return brick->color;                          //만난다면 브릭의 색을 반환
	}
	return 0;                                             //만나지 않으면 0을 반환
} // }}}

/*생성된 브릭이 게임의 경계선 밖으로 나가는 지 확인 */
/*반환값 : 1-경계선 넘어감.  0-오류 없음*/
static char brickCollides(TetrisGame *game) { // {{{
	for (int i = 0; i < 4; i++) {                      
		int p = bricks[game->brick.type][game->brick.rotation][i];   //4*4크기의 위에서 확인함
		int x = p % 4 + game->brick.x;                    //(4*4에서의 상대적 위치)+(4*4의 위치)=해당 칸의 실제 x좌표
		if (x < 0 || x >= game->width) return 1;          //x좌표가 경계선을 넘기면 1 반환
		int y = p / 4 + game->brick.y;                    //(4*4에서의 상대적 위치)+(4*4의 위치)=해당 칸의 실제 y좌표
		if (y >= game->height) return 1;                  //y좌표가 경계선을 넘기면 1 반환
		p = x + y * game->width;       
		if (p >= 0 && p < game->size && game->board[p] != 0)    //블럭이 게임의 경계선을 넘어가면 1 반환.
			return 1;
	}
	return 0;
} // }}}

/*브릭을 터미널 상에 생성*/
static void landBrick(TetrisGame *game) { // {{{
	if (game->brick.type < 0) return;                     //brick의 생성 오류 시 종료
	for (int i = 0; i < 4; i++){
		int p = bricks[game->brick.type][game->brick.rotation][i];  //4*4 브릭 중 한 블럭
		int x = p % 4 + game->brick.x;                    //블럭의 실제 x좌표
		int y = p / 4 + game->brick.y;                    //블럭의 실제 y좌표
		p = x + y * game->width;
		game->board[p] = game->brick.color;               //좌표에 해당하는 칸을 브릭의 색으로 칠함
	}
} // }}}

/*완료된 line 삭제*/
static void clearFullRows(TetrisGame *game) { // {{{
	int width = game->width;
	int rowsCleared = 0;
	for (int y = game->brick.y; y < game->brick.y + 4; y++) {  //y좌표 한 line씩 확인
		char clearRow = 1;
		for (int x = 0; x < width; x++) {
			if (0 == game->board[x + y * width]) {
				clearRow = 0;
				break;
			}
		}                                                  //해당하는 line을 확인하고 빈칸이 있으면 loop 탈출 및 jump
		if (clearRow) {                                    //line에 빈칸이 없는 경우
			for (int d = y; d > 0; d--)
				memcpy(game->board + width*d, game->board + width*(d-1), width);
			bzero(game->board, width); // delete first line. 해당 line을 지움
			y--;
			rowsCleared++;                            
		}
	}
	if (rowsCleared > 0) { 
		// apply score: 0, 1, 3, 5, 8. 점수 적용
		game->score += rowsCleared * 2 - 1;
		if (rowsCleared >= 4) game->score++;
	}
} // }}}

void tick(TetrisGame *game) { // {{{
	if (game->isPaused) return;
	game->brick.y++;
	if (brickCollides(game)) {
		game->brick.y--;
		landBrick(game);
		clearFullRows(game);
		nextBrick(game);
		if (brickCollides(game))
			game->isRunning = 0;
	}
	printBoard(game);
} // }}}

static void pauseUnpause(TetrisGame *game) { // {{{
	if (game->isPaused) {
		// TODO de-/reactivate timer
		tick(game);
	}
	game->isPaused ^= 1;
} // }}}

static void moveBrick(TetrisGame *game, char x, char y) { // {{{
	if (game->isPaused) return;
	game->brick.x += x;
	game->brick.y += y;
	if (brickCollides(game)) {
		game->brick.x -= x;
		game->brick.y -= y;
	}
	printBoard(game);
} // }}}

static void rotateBrick(TetrisGame *game, char direction) { // {{{
	if (game->isPaused) return;
	unsigned char oldRotation = game->brick.rotation;
	game->brick.rotation += 4 + direction; // 4: keep it positive
	game->brick.rotation %= 4;
	if (brickCollides(game))
		game->brick.rotation = oldRotation;
	printBoard(game);
} // }}}

void processInputs(TetrisGame *game) { // {{{
	char c = getchar();
	do {
		switch (c) {
			case ' ': moveBrick(game, 0, 1); break;
			//case '?': dropBrick(game); break;
			case 'p': pauseUnpause(game); break;
			case 'q': game->isRunning = 0; break;
			case 27: // ESC
				getchar();
				switch (getchar()) {
					case 'w': rotateBrick(game,  1);  break; // up
					case 's': rotateBrick(game, -1);  break; // down
					case 'd': moveBrick(game,  1, 0); break; // right
					case 'a': moveBrick(game, -1, 0); break; // left
				}
				break;
		}
	} while ((c = getchar()) != -1);
} // }}}
