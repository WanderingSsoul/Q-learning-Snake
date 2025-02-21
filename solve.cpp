#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/system.hpp>
#include <SFML/window.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <windows.h>
#include <set>
#include <map>
#include <stdio.h>
#include <random>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <unordered_map>
#include <conio.h>
#pragma GCC Optimize("O3")
#pragma GCC target("avx2")
#pragma GCC Optimize("unroll-loops")
using namespace std;
using namespace sf;
const int ogrx = 10, ogry = 10;
const int dequesize = 200000;
const long double epsmin = 0.0009;
const long double epsconst = 0.9995;
struct AI {
    long double alpha = 0.05;
    long double gamma = 0.97;
    short sost[ogrx][ogry];
    short direct;
    float r(pair <short, short> pos) {
        if (pos.first == ogrx || pos.first == -1 || pos.second == ogry || pos.second == -1) {
            return -100;
        }
        else if (sost[pos.first][pos.second] <= 3) {
            return -100;
        }
        else if (sost[pos.first][pos.second] == 4) return 10;
        else return -0.03;
    }
    bool operator<(const AI& other) const {
        if (direct > other.direct) return true;
        if (direct < other.direct) return false;
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                if (sost[i][j] > other.sost[i][j]) return true;
                if (sost[i][j] < other.sost[i][j]) return false;
            }
        }
        return false;
    }
    bool operator==(const AI& other) const {
        if (direct == other.direct) {
            for (int i = 0; i < ogrx; i++) {
                for (int j = 0; j < ogry; j++) {
                    if (sost[i][j] != other.sost[i][j]) {
                        return 0;
                    }
                }
            }
            return 1;
        }
        return 0;
    }
};
struct AIHash {
    std::size_t operator()(const AI& ai) const {
        std::size_t hash_value = std::hash<int>()(ai.direct);
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                hash_value ^= std::hash<short>()(ai.sost[i][j]) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
            }
        }
        return hash_value;
    }
};
sf::RenderWindow win(sf::VideoMode(1300, 700), "Snake.io");
Sprite Snakehead;
Sprite field[10][10];
Sprite Snakebody;
Texture Snakeheadd;
Texture Snakebodyy;
Texture fieldd_with_apple;
Texture fieldd;
Font font;
Text score;
Text scoreText;
Image Snakeheaddd;
Image Snakebodyyy;
Image fielddd;
Image fff;
deque <AI> dq;
unordered_map <AI, long double, AIHash> sost;
vector <pair <pair <int,int>,int> > pos;
bool checkapple[ogrx][ogry];
bool usedbysnake[ogrx][ogry];
bool wasat[dequesize] = {};
int cntepisode = 0;
long double epsilon = 1;
short mx = 0;
vector <pair <short, short> > shuffle(vector <pair <short, short> > v) {
    //shuffles an array in random order, only deterministically for all identical arrays
    for (int i = 0; i < v.size(); i++) {
        swap(v[i], v[((i + v[i].first) % v.size() + v.size() / 3) % v.size()]);
    }
    return v;
}
pair <short, short> posNewApple(AI x) {
    //finds a position for a new apple in free positions, this position is deterministic for a specific array
    long long hash = 0;
    for (int i = 0; i < ogrx; i++) {
        for (int j = 0; j < ogry; j++) {
            hash += i * x.sost[i][j]*10193842 + j * x.sost[i][j]*139221 + 21343;
        }
    }
    hash %= 10108230;
    vector <pair <short, short> > pos;
    //the pos array will store all free positions in a specific location, the function will return a specific index
    for (short i = 0; i < ogrx; i++) {
        for (short j = 0; j < ogry; j++) {
            if (x.sost[i][j] == 5) {
                pos.push_back({ i , j });
            }
        }
    }
    if (pos.empty()) {
        //if that will work if we won the game
        cout << "GG!!!! \n";
        _getch();
        exit(0);
    }
    pos = shuffle(pos);
    return pos[hash % pos.size()];
}
void buildAI() {
    AI fr;
    //fr - starting position and parameters for the snake
    for (short i = 0; i < ogrx; i++) {
        for (short j = 0; j < ogry; j++) {
            fr.sost[i][j] = 5;
        }
    }
    fr.sost[3][3] = -1;
    fr.sost[3][2] = 1;
    fr.sost[3][1] = 1;
    fr.sost[3][0] = 1;
    fr.direct = 1;
    pair <short, short> apple1 = posNewApple(fr);
    fr.sost[apple1.first][apple1.second] = 4;
    pair <short, short> apple2 = posNewApple(fr);
    fr.sost[apple2.first][apple2.second] = 4;
    pair <short, short> apple3 = posNewApple(fr);
    fr.sost[apple3.first][apple3.second] = 4;
    pair <short, short> apple4 = posNewApple(fr);
    fr.sost[apple4.first][apple4.second] = 4;
    //deque will store no more than the last 200000 positions in our episodes
    for (int i = 0; i < 64; i++) {
        dq.push_back(fr);
    }
}
int rrand() {
    //this function will return a random number, the rand() function is not suitable because it only returns a value up to 32767
    int x = rand(), y = rand(), z = rand();
    return x * y + z;
}
void updateAI() {
    //we will have a buffer that will store up to 200,000 past states
    while (dq.size() > dequesize) {
        dq.pop_front();
    }
    //we will select 32 random times and the last 15 and update the answer for them
    vector <int> nw;
    //the vector nw will store the selected indices
    for (int i = 0; i < 32; i++) {
        int x = rrand() % dq.size();
        if (wasat[x]) {
            i--;
            continue;
        }
        else {
            wasat[x] = true;
            nw.push_back(x);
        }
    }
    for (int i = dq.size() - 1; i >= dq.size() - 15; i--) {
        if (!wasat[i]) {
            nw.push_back(i);
            wasat[i] = true;
        }
    }
    for (auto x : nw) {
        AI current = dq[x];
        AI nwmatrix;
        //Let's update by finding the best move when we make a move, 
        //and using it we will update the optimality of choosing the current.direct direction
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                if (current.sost[i][j] != 4) {
                    nwmatrix.sost[i][j] = 5;
                }
                else {
                    nwmatrix.sost[i][j] = 4;
                }
            }
        }
        bool was[ogrx][ogry];
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                was[i][j] = false;
            }
        }
        pair <short, short> posHead = { -1,-1 };
        for (short i = 0; i < ogrx; i++) {
            for (short j = 0; j < ogry; j++) {
                if (current.sost[i][j] == 0) {
                    was[i + 1][j] = true;
                }
                if (current.sost[i][j] == 1) {
                    was[i][j + 1] = true;
                }
                if (current.sost[i][j] == 2) {
                    was[i - 1][j] = true;
                }
                if (current.sost[i][j] == 3) {
                    was[i][j - 1] = true;
                }
                if (current.sost[i][j] == -1) {
                    posHead = { i , j };
                }
            }
        }
        for (short i = 0; i < ogrx; i++) {
            for (short j = 0; j < ogry; j++) {
                if (was[i][j]) {
                    nwmatrix.sost[i][j] = current.sost[i][j];
                }
            }
        }
        nwmatrix.sost[posHead.first][posHead.second] = current.direct;
        short nwx = posHead.first, nwy = posHead.second;
        if (current.direct == 0) nwx++;
        else if (current.direct == 1) nwy++;
        else if (current.direct == 2) nwx--;
        else nwy--;
        long double mx = 0;
        //Now, in the new state, we will choose the direction that will give the maximum score and update the values.
        if (nwx >= 0 && nwx < ogrx && nwy >= 0 && nwy < ogry&&current.sost[nwx][nwy] > 3) {
            if (nwmatrix.sost[nwx][nwy] == 4) {
                nwmatrix.sost[nwx][nwy] = -1;
                for (int i = 0; i < ogrx; i++) {
                    for (int j = 0; j < ogry; j++) {
                        if (nwmatrix.sost[i][j] == 5 && current.sost[i][j] <= 3) {
                            nwmatrix.sost[i][j] = current.sost[i][j];
                        }
                    }
                }
                pair <int, int> posApple = posNewApple(nwmatrix);
                nwmatrix.sost[posApple.first][posApple.second] = 4;
            }
            nwmatrix.sost[nwx][nwy] = -1;
            nwmatrix.direct = 0;
            mx = sost[nwmatrix];
            nwmatrix.direct = 1;
            mx = max(mx, sost[nwmatrix]);
            nwmatrix.direct = 2;
            mx = max(mx, sost[nwmatrix]);
            nwmatrix.direct = 3;
            mx = max(mx, sost[nwmatrix]);
        }
        //let's update our score at Q(s,a) via the basic Q-learning formula
        //Q(s,a) ← Q(s,a) + α[r + γ * max(s',a') - Q(s,a)] 
        sost[current] = sost[current] + current.alpha * (current.r(make_pair(nwx, nwy)) + current.gamma * mx - sost[current]);
    }
    for (auto x : nw) {
        wasat[x] = false;
    }
}
void DrawScore(int scoreint) {
    //this function will display the current score on the screen
    scoreText.setFont(font);
    scoreText.setCharacterSize(100);
    scoreText.setString("Score: ");
    scoreText.setPosition(850,80);
    score.setFont(font);
    score.setCharacterSize(200);
    score.setString(to_string(scoreint));
    score.setFillColor(sf::Color::White);
    score.setPosition(850, 200);
    win.draw(score);
    win.draw(scoreText);
}
void Draw(short x, short i, short j) {
    //this function will display some cell on the field on the screen
    Sprite gg;
    if (x == -1) gg.setTexture(Snakeheadd);
    if (x >= 0 && x <= 3) gg.setTexture(Snakebodyy);
    if (x == 4) gg.setTexture(fieldd_with_apple);
    if (x == 5) gg.setTexture(fieldd);
    gg.setPosition(200 + j * 40, 100 + i * 40);
    win.draw(gg);
}
void Draww(AI x) {
    //this function will display the position on the field to the screen, where it takes about 200 milliseconds per move
    Sleep(200);
    win.clear();
    int scoreint = 0;
    for (short i = 0; i < ogrx; i++) {
        for (short j = 0; j < ogry; j++) {
            win.draw(field[i][j]);
            if (x.sost[i][j] <= 3) {
                scoreint++;
            }
        }
    }
    DrawScore(scoreint);
    for (short i = 0; i < ogrx; i++) {
        for (short j = 0; j < ogry; j++) {
            Draw(x.sost[i][j],i,j);
        }
    }
    win.display();
}
void Episode() {
    cntepisode++;
    epsilon = max(epsmin, epsilon * epsconst);//updating our chance to choose a random move
    AI episode;
    //episode will store the position of the snake on the current episode
    for (short i = 0; i < ogrx; i++) {
        for (short j = 0; j < ogry; j++) {
            episode.sost[i][j] = 5;
        }
    }
    episode.sost[3][3] = -1;
    episode.sost[3][2] = 1;
    episode.sost[3][1] = 1;
    episode.sost[3][0] = 1;
    episode.direct = 1;
    pair <short, short> apple1 = posNewApple(episode);
    episode.sost[apple1.first][apple1.second] = 4;
    pair <short, short> apple2 = posNewApple(episode);
    episode.sost[apple2.first][apple2.second] = 4;
    pair <short, short> apple3 = posNewApple(episode);
    episode.sost[apple3.first][apple3.second] = 4;
    pair <short, short> apple4 = posNewApple(episode);
    episode.sost[apple4.first][apple4.second] = 4;
    int steps = 0;
    while (true) {
        //During each iteration of this while the following code will be selected
        //Also the function will end if the snake collides with an obstacle
        if(steps % 4 == 0) updateAI();
        steps++;
        AI nwmatrix;
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                if (episode.sost[i][j] != 4) {
                    nwmatrix.sost[i][j] = 5;
                }
                else {
                    nwmatrix.sost[i][j] = 4;
                }
            }
        }
        //nwmatrix will store the position to which the snake goes after its move
        bool was[ogrx][ogry];
        for (int i = 0; i < ogrx; i++) {
            for (int j = 0; j < ogry; j++) {
                was[i][j] = false;
            }
        }
        //move all elements of the snake 1 position forward
        pair <short, short> posHead;
        for (short i = 0; i < ogrx; i++) {
            for (short j = 0; j < ogry; j++) {
                if (episode.sost[i][j] >= 0 && episode.sost[i][j] <= 3) {
                    if (episode.sost[i][j] == 0) {
                        was[i + 1][j] = true;
                    }
                    if (episode.sost[i][j] == 1) {
                        was[i][j + 1] = true;
                    }
                    if (episode.sost[i][j] == 2) {
                        was[i - 1][j] = true;
                    }
                    if (episode.sost[i][j] == 3) {
                        was[i][j - 1] = true;
                    }
                }
                else if (episode.sost[i][j] == -1) {
                    posHead = { i,j };
                }
            }
        }
        //let's add this move to our buffer
        dq.push_back(episode);
        for (short i = 0; i < ogrx; i++) {
            for (short j = 0; j < ogry; j++) {
                if (was[i][j]) {
                    nwmatrix.sost[i][j] = episode.sost[i][j];
                }
            }
        }
        nwmatrix.sost[posHead.first][posHead.second] = episode.direct;
        //let's find a new head position
        short nwx = posHead.first, nwy = posHead.second;
        if (episode.direct == 0) nwx++;
        else if (episode.direct == 1) nwy++;
        else if (episode.direct == 2) nwx--;
        else nwy--;
        //Let's check if the snake's head collided with an obstacle
        if (nwx >= 0 && nwx < ogrx && nwy >= 0 && nwy < ogry && episode.sost[nwx][nwy] > 3) {
            if (nwmatrix.sost[nwx][nwy] == 4) {
                nwmatrix.sost[nwx][nwy] = -1;
                short cntt = 0;
                for (int i = 0; i < ogrx; i++) {
                    for (int j = 0; j < ogry; j++) {
                        if (nwmatrix.sost[i][j] == 5 && episode.sost[i][j] <= 3) {
                            nwmatrix.sost[i][j] = episode.sost[i][j];
                        }
                        if (nwmatrix.sost[i][j] <= 3) cntt++;
                    }
                }
                mx = max(mx, cntt);
                pair <int, int> posApple = posNewApple(nwmatrix);
                nwmatrix.sost[posApple.first][posApple.second] = 4;
            }
            nwmatrix.sost[nwx][nwy] = -1;
        }
        else {
            //if the game is over, we will display 100 times the maximum score that the 
            //snake reached in all episodes, the current chance to choose a random action and also the episode number
            if(cntepisode % 100 == 0) cout << "Episode " << cntepisode << " Ended, probablity e = " << epsilon << " mx = " << mx << "\n";
            updateAI();
            break;
        }
        //lets select the next move
        //with probability epsilon we will choose a random move, otherwise with the best outcome at the moment
        if ((rrand() % 10000) / 10000.0 > epsilon) {
            nwmatrix.direct = 0;
            float zn0 = sost[nwmatrix];
            nwmatrix.direct = 1;
            float zn1 = sost[nwmatrix];
            nwmatrix.direct = 2;
            float zn2 = sost[nwmatrix];
            nwmatrix.direct = 3;
            float zn3 = sost[nwmatrix];
            float mx = max({ zn0,zn1,zn2,zn3 });
            if (zn3 == mx) {
                nwmatrix.direct = 3;
            }
            if (zn2 == mx) {
                nwmatrix.direct = 2;
            }
            if (zn1 == mx) {
                nwmatrix.direct = 1;
            }
            if (zn0 == mx) {
                nwmatrix.direct = 0;
            }
        }
        else {
            nwmatrix.direct = rand() % 4;
        }
        //let's update our episode status.
        episode = nwmatrix;
        //let's show every thousandth episode in its entirety
        if (cntepisode % 1000 == 0) {
            Draww(nwmatrix);
        }
    }
}
void loadAllFiles() {
    //this function will load all the necessary files into our code. For example, pictures and font
    font.loadFromFile("fonts/minecraft.ttf");
    Snakeheaddd.loadFromFile("images/Snake_head.png");
    Snakeheadd.loadFromImage(Snakeheaddd);
    Snakehead.setTexture(Snakeheadd);
    Snakebodyyy.loadFromFile("images/Snake_body.png");
    Snakebodyy.loadFromImage(Snakebodyyy);
    Snakebody.setTexture(Snakebodyy);
    fielddd.loadFromFile("images/field_stylized.png");
    fieldd.loadFromImage(fielddd);
    fff.loadFromFile("images/apple_field.png");
    fieldd_with_apple.loadFromImage(fff);
}
int main() {
    ios_base::sync_with_stdio(0);
    cin.tie(0);
    cout.tie(0);
    srand(time(0));
    loadAllFiles();
    buildAI();
    while (true) {
        Episode();
    }
}
