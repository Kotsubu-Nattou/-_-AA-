/*********************************************************************************************************
〇 ブレゼンハムの考え方。Bresenham's line algorithm
x（またはy）を基準として、1ドット移動したとき、y（またはx）も移動するかどうかを判定しながら進む。
基本的にすべて整数演算で高速。
オリジナル要素 --- 終点から始点に向かって描画, 疑似アンチエイリアシング, アルファ減衰（グラデーション）

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



// 【関数】線分をレンダリング
void renderLine(s3d::Image& img, s3d::Point startPos, s3d::Point endPos, s3d::ColorF col)
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
                // yを「1ドット」移動
                now.y += step.y;
                // 誤差をリセット。超過分を残すのがミソ
                e -= dist2.x;
            }
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
                now.x += step.x;
                e -= dist2.y;
            }
        }
    }
}



// 【関数】線分をレンダリング。疑似アンチエイリアシング付き
void renderLineAA(s3d::Image& img, s3d::Point startPos, s3d::Point endPos, s3d::ColorF col,
                  double aaColorRate = 0.3)
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
    // AA部分の通常部分に対する色の割合
    if (aaColorRate < 0.0) aaColorRate = 0.0;
    if (aaColorRate > 1.0) aaColorRate = 1.0;
    // AA部分の色
    s3d::ColorF aaCol = s3d::ColorF(col, col.a * aaColorRate);


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
                img[now.y][now.x].set(aaCol);           // 疑似AA

                // yを「1ドット」移動
                now.y += step.y;

                img[now.y][now.x - step.x].set(aaCol);  // 疑似AA

                // 誤差をリセット。超過分を残すのがミソ
                e -= dist2.x;
            }
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
                img[now.y][now.x].set(aaCol);
                now.x += step.x;

                img[now.y - step.y][now.x].set(aaCol);
                e -= dist2.y;
            }
        }
    }
}



// 【関数】減衰する線分をレンダリング。疑似アンチエイリアシング付き
void renderDecayLine(s3d::Image& img, s3d::Point startPos, s3d::Point endPos, s3d::ColorF col,
                     double decaySectionRate = 0.5, double aaColorRate = 0.3)
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
    // AA部分の通常部分に対する色の割合
    if (aaColorRate < 0.0) aaColorRate = 0.0;
    if (aaColorRate > 1.0) aaColorRate = 1.0;
    // AA部分の色
    s3d::ColorF aaCol = s3d::ColorF(col, col.a * aaColorRate);
    // 減衰区間の割合
    if (decaySectionRate < 0.0) decaySectionRate = 0.0;
    if (decaySectionRate > 1.0) decaySectionRate = 1.0;


    if (dist.x >= dist.y) {
        // ◎◎ x基準
        s3d::int32 e        = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
        s3d::int32 decayLen = (endPos.x - startPos.x) * decaySectionRate;  // 減衰区間の長さ
        s3d::int32 splitX   = startPos.x + decayLen;                       // 分割点x

        // ◎ 終点xから分割点xまでループ（通常のAA付き線分の処理）
        for (;;) {
            // 現在位置に点を描く
            img[now.y][now.x].set(col);

            // 分割点ならループを抜ける
            if (now.x == splitX) break;

            // xを「1ドット」移動
            now.x += step.x;

            // 誤差を蓄積
            e += dist2.y;

            // 誤差がたまったら
            if (e >= dist2.x) {
                img[now.y][now.x].set(aaCol);           // 疑似AA

                // yを「1ドット」移動
                now.y += step.y;

                img[now.y][now.x - step.x].set(aaCol);  // 疑似AA

                // 誤差をリセット。超過分を残すのがミソ
                e -= dist2.x;
            }
        }

        // 始点なら終了
        if (now.x == startPos.x) return;

        // ◎ 分割点xから始点xまでループ（ここが減衰する）
        double alphaFadeVol = col.a / (1 + std::abs(decayLen));  // アルファのフェード量
        for (;;) {
            // 初回の重複描画を避けるためフローを変更
            now.x += step.x;
            e += dist2.y;

            col.a -= alphaFadeVol;  // アルファをフェードアウト

            if (e >= dist2.x) {
                img[now.y][now.x].set(ColorF(col, col.a * aaColorRate));
                now.y += step.y;
                img[now.y][now.x - step.x].set(ColorF(col, col.a * aaColorRate));
                e -= dist2.x;
            }

            img[now.y][now.x].set(col);
            if (now.x == startPos.x) break;
        }
    }

    else {
        // ◎◎ y基準
        s3d::int32 e        = dist.y;
        s3d::int32 decayLen = (endPos.y - startPos.y) * decaySectionRate;
        s3d::int32 splitY   = startPos.y + decayLen;

        for (;;) {
            img[now.y][now.x].set(col);
            if (now.y == splitY) break;

            now.y += step.y;
            e += dist2.x;

            if (e >= dist2.y) {
                img[now.y][now.x].set(aaCol);
                now.x += step.x;
                img[now.y - step.y][now.x].set(aaCol);
                e -= dist2.y;
            }
        }
        if (now.y == startPos.y) return;

        double alphaFadeVol = col.a / (1 + std::abs(decayLen));
        for (;;) {
            now.y += step.y;
            e += dist2.x;

            col.a -= alphaFadeVol;

            if (e >= dist2.y) {
                img[now.y][now.x].set(ColorF(col, col.a * aaColorRate));
                now.x += step.x;
                img[now.y - step.y][now.x].set(ColorF(col, col.a * aaColorRate));
                e -= dist2.y;
            }

            img[now.y][now.x].set(col);
            if (now.y == startPos.y) break;
        }
    }
}





void Main()
{
    double scale = 16.0;
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
                renderDecayLine(board.mImg, startPos, endPos, ColorF(0.4, 0.8, 1.0, 1.0), 0.5);
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
