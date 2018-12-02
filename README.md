![Screenshot](https://github.com/Gjum/tetris-term/blob/master/screenshot.png)

tetris-term
===========

Classic Tetris for your terminal. 
터미널 상에서 실행되는 클래식 테트리스

Usage
사용법
-----

### Installation:
설치 방법

`git clone https://github.com/Gjum/tetris-term.git && make -C tetris-term`

### Controls:
조작법

 - `Left`  move brick left
 - `Right` move brick right
 - `Up`    rotate brick clockwise
 - `Down`  rotate brick counter-clockwise
 - `Space` move brick down by one step
 - `p`     pause game
 - `q`     quit game  

 - `Left`  브릭 왼쪽으로 움직이기
 - `Right` 브릭 오른쪽으로 움직이기
 - `Up`    브릭 시계방향으로 회전
 - `Down`  브릭 반시계방향으로 회전
 - `Space` 브릭 아래로 내리기
 - `p`     일시정지
 - `q`     게임 중지

Features
기능
--------

- 7 different colors
- no fancy dependencies
- lightweight on your resources  

- 7가지 색
- 종속성 없음
- 가벼운 프로그램

### Roadmap:
추후 계획

- changing speed:
  - getting faster with every block or line
  - via argument
- more arguments:
  - disable color
  - change size
- more controls:
  - move brick to left/right border
  - drop brick
  - restart/new game
- config file for controls, colors, default size, speed  

- 속도 변경:
  - 블럭의 더 빠른 속도
  - 인자를 통해 제어 가능
- 다양한 선택 옵션:
  - 색 비활성화
  - 크기 변경
- 다양한 조작키:
  - 브릭을 양 끝으로 이동
  - 브릭 떨어뜨리기
  - 재시작
- 조작키, 색, 크기, 속도에 대한 설정 파일

License
-------

`tetris-term` is licensed under the **GNU General Public License**. See [`LICENSE`](https://github.com/Gjum/tetris-term/blob/master/LICENSE).

