#include "DxLib.h"
#include <string>

int board[8][8]; // 盤のデータ(0:なし 1:黒コマ 2:白コマ)
std::string msg;
int msg_wait;

// 指定した位置にコマを置く
int putPiece(int x, int y, int turn, bool put_flag) {
    int sum = 0;
    if (board[y][x] > 0) return 0;
    for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
        int wx[8], wy[8];
        for (int wn = 0;; wn++) {
            int kx = x + dx * (wn + 1); int ky = y + dy * (wn + 1);
            if (kx < 0 || kx > 7 || ky < 0 || ky > 7 || board[ky][kx] == 0) break;
            if (board[ky][kx] == turn) {
                if (put_flag) for (int i = 0; i < wn; i++) board[wy[i]][wx[i]] = turn;
                sum += wn;
                break;
            }
            wx[wn] = kx; wy[wn] = ky;
        }
    }
    if (sum > 0 && put_flag) board[y][x] = turn;
    return sum;
}

// パスチェック
bool isPass(int turn) {
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
        if (putPiece(x, y, turn, false)) return false;
    }
    return true;
}

// 思考ルーチン1 プレイヤー
bool think1(int turn) {
    static bool mouse_flag = false;
    if (GetMouseInput() & MOUSE_INPUT_LEFT) {
        if (!mouse_flag) {
            mouse_flag = true;
            int mx, my;
            GetMousePoint(&mx, &my);
            if (putPiece(mx / 48, my / 48, turn, true)) return true;
        }
    }
    else mouse_flag = false;
    return false;
}


// 思考ルーチン2 最も多く取れるところに置く
bool think2(int turn) {
    int max = 0, wx, wy;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
        int num = putPiece(x, y, turn, false);
        if (max < num || (max == num && GetRand(1) == 0)) {
            max = num; wx = x; wy = y;
        }
    }
    putPiece(wx, wy, turn, true);
    return true;
}

// 思考ルーチン3 優先順位の高いところに置く
bool think3(int turn) {
    int priority[8][8] = {
      {0, 6, 2, 1, 1, 2, 6, 0},
      {6, 6, 5, 4, 4, 5, 6, 6},
      {2, 5, 2, 3, 3, 2, 5, 2},
      {1, 4, 3, 3, 3, 3, 4, 1},
      {1, 4, 3, 3, 3, 3, 4, 1},
      {2, 5, 2, 3, 3, 2, 5, 2},
      {6, 6, 5, 4, 4, 5, 6, 6},
      {0, 6, 2, 1, 1, 2, 6, 0},
    };
    int max = 0, wx, wy;
    for (int p = 0; p <= 6 && max == 0; p++) {
        for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
            if (priority[y][x] != p) continue;
            int num = putPiece(x, y, turn, false);
            if (max < num || (max == num && GetRand(1) == 0)) {
                max = num; wx = x; wy = y;
            }
        }
    }
    putPiece(wx, wy, turn, true);
    return true;
}

// メッセージセット
// turn ... 1:BLACK 2:WHITE 3:DRAW
// type ... 0:TURN 1:PASS 2:WIN!
void setMsg(int turn, int type) {
    std::string turn_str[] = { "BLACK", "WHITE", "DRAW" };
    std::string type_str[] = { "TURN", "PASS", "WIN!" };
    msg = turn_str[turn - 1];
    if (turn != 3) msg += " " + type_str[type];
    msg_wait = 50;
}

// 勝敗チェック
int checkResult() {
    int pnum[2] = {};
    int result = 0;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
        if (board[y][x] > 0) pnum[board[y][x] - 1]++;
    }
    if (isPass(1) && isPass(2)) {
        if (pnum[0] > pnum[1]) result = 1;
        else if (pnum[0] < pnum[1]) result = 2;
        else result = 3;
    }
    if (result) setMsg(result, 2);
    return result;
}

// WinMain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    int pieces[2];
    int back;
    int status = 2; // 1:プレイ中 2:TURNメッセージ中 3:パスメッセージ中 4:終了
    int turn = 1; // 1:黒ターン 2:白ターン
    SetGraphMode(384, 384, 32);
    ChangeWindowMode(TRUE);
    DxLib_Init();
    SetDrawScreen(DX_SCREEN_BACK);
    LoadDivGraph("piece.png", 2, 2, 1, 48, 48, pieces);
    back = LoadGraph("back.png");
    board[3][3] = board[4][4] = 1;
    board[4][3] = board[3][4] = 2;
    setMsg(turn, 0);
    while (!ProcessMessage()) {
        ClearDrawScreen();
        switch (status) {
        case 1:
            if (isPass(turn)) {
                setMsg(turn, 1);
                status = 3;
            }
            else {
                bool (*think[])(int) = { think1, think2 };
                if ((*think[turn - 1])(turn)) {
                    turn = 3 - turn; status = 2;
                    setMsg(turn, 0);
                }
            }
            if (checkResult()) status = 4;
            break;
        case 2:
            if (msg_wait > 0) msg_wait--;
            else status = 1;
            break;
        case 3:
            if (msg_wait > 0) msg_wait--;
            else {
                turn = 3 - turn; status = 2;
                setMsg(turn, 0);
            }
            break;
        }
        DrawGraph(0, 0, back, FALSE);
        for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
            if (board[y][x]) DrawGraph(x * 48, y * 48, pieces[board[y][x] - 1], TRUE);
        }
        if (status > 1) {
            int mw = GetDrawStringWidth(msg.c_str(), msg.size());
            DrawBox(192 - mw / 2 - 30, 172, 192 + mw / 2 + 30, 208, GetColor(200, 180, 150), TRUE);
            DrawString(192 - mw / 2, 182, msg.c_str(), GetColor(255, 255, 255));
        }
        ScreenFlip();
    }
    DxLib_End();
    return 0;
}