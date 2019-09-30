/*********************************************************************************************************
〇 ブレゼンハムの考え方。Bresenham's line algorithm
x（またはy）を基準として、1ドット移動したとき、y（またはx）も移動するかどうかを判定しながら進む。
基本的にすべて整数演算で高速。
オリジナル要素 --- 終点から始点に向かう, 疑似アンチエイリアシング, アルファ減衰（グラデーション）

〇 フロー（底辺 >= 高さ のとき。初期位置は終点）
1. 現在位置に点を描画
2. 現在位置xが始点xなら終了
3. xを「1ドット」移動
4. yも移動するかどうか
     ・水平(高さ0)       --- 移動しなくてよい
     ・25度(底辺2:高さ1) --- xが2進んだタイミングで移動すればよい
     ・45度(底辺1:高さ1) --- xと同じタイミングで移動すればよい
     ・それを超える角度  --- x=1の移動に対しy=1を超える（離散的）なので別処理とする
   上記を判定する
     ・e += 高さ             // 誤差を蓄積
     ・もし（e >= 底辺）なら
           yを1移動
           e -= 底辺         // 誤差をリセット。超過分を残すのがミソ
5. 上記1.へ

〇 補足
eの初期値は四捨五入を期待して「底辺/2」とする。しかし、
eを小数型にすると演算が遅くなり、整数型に代入すれば誤差が出る。そこで、
関連するパラメータを2倍して扱うことで「整数演算かつ誤差無し」にできる。
＜注意＞ ここで言う誤差とは計算の誤差のことであり、ブレゼンハムアルゴリズムの
主要パラメータの「誤差」とは別物。ちなみに、誤差の英訳はerror、eはその略
***********************************************************************************************************/

#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



// 【関数】線分をレンダリング。疑似アンチエイリアシング、アルファ減衰（グラデーション）付き
void renderLine(s3d::Image& img, s3d::ColorF col, double alphaDecayRate, s3d::Point startPos, s3d::Point endPos)
{
    // 終点を初期位置として始める
    s3d::Point now = endPos;
    // xとyそれぞれの、距離（絶対値）と進むべき方向（正負）を求める
    s3d::Point dist, step;
    if (endPos.x >= startPos.x)
        { dist.x = endPos.x - startPos.x; step.x = -1; }
    else
        { dist.x = startPos.x - endPos.x; step.x = 1; }
    if (endPos.y >= startPos.y)
        { dist.y = endPos.y - startPos.y; step.y = -1; }
    else
        { dist.y = startPos.y - endPos.y; step.y = 1; }
    // 誤差の判定時に四捨五入する、かつ整数で扱うため、関連パラメータを2倍する
    s3d::Point dist2 = dist * 2;


    if (dist.x >= dist.y) {
        // x基準
        s3d::int32 e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
        for (;;) {
            // 現在位置に点を描く
            img[now.y][now.x].set(col);

            // 始点なら終了
            if (now.x == startPos.x) break;

            // xを「1ドット」移動
            now.x += step.x;

            // 誤差を蓄積
            e += dist2.y;

            // 誤差がたまったら
            if (e >= dist2.x) {
                img[now.y][now.x].set(ColorF(col, col.a * 0.5));  // 疑似AA

                // yを「1ドット」移動
                now.y += step.y;

                img[now.y][now.x - step.x].set(ColorF(col, col.a * 0.5));  // 疑似AA

                // 誤差をリセット。超過分を残すのがミソ
                e -= dist2.x;
            }
            col.a *= alphaDecayRate;  // アルファ減衰
        }
    }
    else {
        // y基準
        s3d::int32 e = dist.y;
        for (;;) {
            img[now.y][now.x].set(col);

            if (now.y == startPos.y) break;
            now.y += step.y;
            e += dist2.x;

            if (e >= dist2.y) {
                img[now.y][now.x].set(ColorF(col, col.a * 0.5));
                now.x += step.x;

                img[now.y - step.y][now.x].set(ColorF(col, col.a * 0.5));
                e -= dist2.y;
            }
            col.a *= alphaDecayRate;
        }
    }
}





void Main()
{
    double scale = 12.0;
    KotsubuPixelBoard board(400, 300, scale);
    Font font = Font(24);
    bool isDrawing = false;
    Point startPos;

    
    while (System::Update())
    {
        // 左ドラッグ（線分を描く）
        if (MouseL.down()) {
            startPos = board.toImagePos(Cursor::Pos());
            // イメージの範囲内なら作図開始
            if (board.checkRange(startPos))
                isDrawing = true;
        }

        if (MouseL.up())
            isDrawing = false;

        if (isDrawing) {
            Point endPos = board.toImagePos(Cursor::Pos());
            // 線分をレンダリング（ブレゼンハム）
            if (board.checkRange(endPos)) {
                board.clear();
                renderLine(board.mImg, ColorF(0.4, 0.8, 1.0, 1.0), 0.92, startPos, endPos);
            }
        }



        // ピクセルボードをドロー
        s3d::RenderStateBlock2D renderState(s3d::BlendState::Additive, s3d::SamplerState::ClampNearest);
        board.draw();



        // GUI処理
        renderState = { s3d::BlendState::Default, s3d::SamplerState::Default2D };

        font(U"Scale: ", scale).draw(Vec2(Window::Width() - 210, 10));
        if (s3d::SimpleGUI::Slider(scale, 1.0, 50.0, Vec2(Window::Width() - 210, 50), 200))
            board.setScale(scale);
    }
}
