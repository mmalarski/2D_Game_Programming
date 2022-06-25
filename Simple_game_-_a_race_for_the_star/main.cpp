#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <time.h>

enum KEYVARIANTS {
    KEYBOARD,
    GAMEPAD
};

enum AVAILABLE_POSITIONS{
    AVAILABLE,
    UNAVAILABLE
};

class LTexture
{
public:
    //Initializes variables
    LTexture();

    //Deallocate memory
    ~LTexture();

    //Loads image at specified path
    bool loadFromFile(std::string path);

    //Deallocate texture
    void free();

    //Renders texture at given point
    void render1(int x, int y);
    void render2(int x, int y, int width, int height);

    //Gets image dimensions
    int getWidth() const;
    int getHeight() const;
    SDL_Texture* getTexture() {return mTexture;}

private:
    //The actual hardware texture
    SDL_Texture* mTexture;

    //Image dimensions
    int mWidth;
    int mHeight;
};

struct Wall{
    Wall(float xp, float yp, float w, float h) : xp(xp), yp(yp), w(w), h(h) {
        this->texture.loadFromFile("../level/wall_fill.png");
    }
    float xp, yp;
    float w, h;
    bool isTarget = false;
    LTexture texture;
};

class Player
{
public:
    //Maximum axis velocity of the dot
    float PLAYER_VEL = 0.6;

    //Initializes the variables
    Player(float x_pos, float y_pos, std::string tex, KEYVARIANTS keyVar);

    //Frees the texture
    ~Player();

    //Takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    //Moves the dot
    void move();
    void setPosition(float x, float y);
    void addPoint();
    void updateOldPoints();
    void resetPoints();

    //Shows the dot on the screen relative to the camera
    void render();
    void render(int x, int y);

    //Position accessors
    float getPosX();
    float getPosY();
    int getPWidth();
    int getPHeight();
    int getActivePoints(){return activePoints;}
    int getOldPoints(){return oldPoints;}

private:
    KEYVARIANTS keyVariant;

    LTexture texture;
    int pWidth, pHeight;

    //The X and Y offsets of the dot
    float mPosX, mPosY;

    //The velocity of the dot
    float mVelX, mVelY;

    int activePoints;
    int oldPoints;
};

class Level{
public:
    Level(std::string map);
    ~Level();

    void loadNewMap(std::string map);
    void render(float init_x, float init_y);
    std::vector<std::vector<int>> getAvailablePositions();
    std::vector<Wall*> getlevelWalls() { return levelWalls; }

private:
    std::vector<Wall*> levelWalls;
    std::vector<std::vector<int>> availablePositions;
};

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int JOYSTICK_DEAD_ZONE = 10000;
const float PLAYER_WIDTH = 0.5; // * map_tile_size

SDL_Window* gWindow = nullptr;
LTexture player1won;
LTexture player2won;
LTexture pointingArrow;
Wall* star;
LTexture points[5];

std::string levelMaps[3] = {
        "../level_map_1.txt",
        "../level_map_2.txt",
        "../level_map_3.txt"
};
std::string* activeLevelMap = &levelMaps[0];
bool levelHasJustBeenChanged = false;

std::string levelMapString = "";
int levelMapWidth = 0;
int levelMapHeight = 0;
int map_tile_size = 40;

float camera_x = 0.0f;
float camera_y = 0.0f;

SDL_Renderer* gRenderer = nullptr;
SDL_Joystick* gGameController = NULL;

bool init();
bool readLevelMap(std::string filename);
void checkCircularCollision(Player &circlePlayer, Wall* wall, Level &level, Player &player1);
void checkRectangularCollision(Player &player1, Wall* w, Level &level, Player &player2);
void switchLevel(Level &level, std::string* &newMap, Player &player1, Player &player2);
void generateRandomPositions(float &x, float &y, Level &level);
void renderActiveScore(Player &player1, Player &player2);
void renderOldScore(Player &player1, Player &player2);
void renderStar(Player &player1, Player &player2);
bool loadMedia();
void close();

int main(int argc, char* args[])
{

    if (!init())
    {
        printf("\nFailed to initialise!\n");
    }
    else
    {

        if (!loadMedia())
        {
            printf("\nCould not load media\n");
        }
        else
        {
            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;
            srand(time(NULL));

            readLevelMap(*activeLevelMap);
            activeLevelMap++;
            map_tile_size = SCREEN_WIDTH / 10;
            Level level(levelMapString);

            float p1pos_x = 0;
            float p1pos_y = 0;
            float p2pos_x = 0;
            float p2pos_y = 0;
            generateRandomPositions(p1pos_x, p1pos_y, level);
            generateRandomPositions(p2pos_x, p2pos_y, level);
            Player player1(p1pos_x, p1pos_y, "../square_player.png", KEYBOARD);
            Player player2(p2pos_x, p2pos_y, "../circle.png", GAMEPAD);
            int shared_camera_x = 0;
            int shared_camera_y = 0;

            //While application is running
            while (!quit)
            {
                //Clear screen
                SDL_RenderClear(gRenderer);

                //Handle events on queue
                while (SDL_PollEvent(&e) != 0)
                {
                    //User requests quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }

                    player1.handleEvent(e);
                    if (gGameController != NULL) { player2.handleEvent(e); }
                }

                if (SDL_NumJoysticks() > 0) {
                    if (gGameController == NULL) {
                        gGameController = SDL_JoystickOpen(0);
                        std::cout << "Game controller detected!" << std::endl;
                    }
                } else {
                    gGameController = NULL;
                }

                //altering the position of players
                if (gGameController != NULL) {
                    player1.move();
                    player2.move();
                    shared_camera_x = (((player1.getPosX() + (player1.getPWidth()  * 0.5f)) - SCREEN_WIDTH  * 0.5f) +
                                       (player2.getPosX()  + (player2.getPWidth()  * 0.5f)) - SCREEN_WIDTH  * 0.5f) * 0.5;
                    shared_camera_y = (((player1.getPosY() + (player1.getPHeight() * 0.5f)) - SCREEN_HEIGHT * 0.5f) +
                                       (player2.getPosY()  + (player2.getPHeight() * 0.5f)) - SCREEN_HEIGHT * 0.5f) * 0.5;
                    camera_x = shared_camera_x;
                    camera_y = shared_camera_y;
                } else {
                    gGameController = NULL;
                    player1.move();
                    camera_x = (player1.getPosX() + player1.getPWidth()  * 0.5) - SCREEN_WIDTH  * 0.5;
                    camera_y = (player1.getPosY() + player1.getPHeight() * 0.5) - SCREEN_HEIGHT * 0.5;
                }

                //checking the camera movement
                if( camera_x < 0 ) { camera_x = 0; }
                if( camera_y < 0 ) { camera_y = 0; }
                if( camera_x > levelMapWidth  * map_tile_size - SCREEN_WIDTH  ) { camera_x = levelMapWidth  * map_tile_size - SCREEN_WIDTH ; }
                if( camera_y > levelMapHeight * map_tile_size - SCREEN_HEIGHT ) { camera_y = levelMapHeight * map_tile_size - SCREEN_HEIGHT; }

                //checking if the players aren't colliding with the map
                for (Wall* w : level.getlevelWalls()){
                    //player 1 (square)
                    checkRectangularCollision(player1, w, level, player2);

                    //player 2 (circle)
                    if (SDL_NumJoysticks() > 0 && gGameController != nullptr) {
                        checkCircularCollision(player2, w, level, player1);
                    }
                }

                //rendering players
                player1.render(camera_x, camera_y);
                if (gGameController != nullptr) {
                    player2.render(camera_x, camera_y);
                }

                if (levelHasJustBeenChanged) {
                    SDL_RenderClear(gRenderer);
                    renderOldScore(player1, player2);
                    SDL_RenderPresent(gRenderer);
                    SDL_Delay(1000);
                    SDL_RenderClear(gRenderer);
                    renderActiveScore(player1, player2);
                    SDL_RenderPresent(gRenderer);
                    SDL_Delay(1000);
                    levelHasJustBeenChanged = false;
                }

                //rendering level map based on previously calculated camera position
                level.render(camera_x, camera_y);
                renderStar(player1, player2);
                renderActiveScore(player1, player2);

                //Update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    close();
    return 0;
}

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow("Simple game: race to the star", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            const char* path = "wall.bmp";
            SDL_Surface* surface = SDL_LoadBMP(path);
            SDL_SetWindowIcon(gWindow, surface);
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize renderer color #b53921
                SDL_SetRenderDrawColor(gRenderer, 0xb5, 0x39, 0x21, 0xFF);

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }

    return success;
}

bool readLevelMap(std::string filename) {
    bool success = true;
    std::ifstream file;
    std::string line;
    file.open(filename);
    levelMapString = "";
    levelMapHeight = 0;
    levelMapWidth = 0;
    if (!file.good()) {
        std::cout << "Wrong file " << filename << std::endl;
        return !success;
    }
    else {
        levelMapString.clear();
        while (!file.eof()) {
            levelMapHeight++;
            std::getline(file, line);
            levelMapString += line;
        }
        levelMapWidth = line.size();

        file.close();
        return success;
    }
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;
    if(!points[0].loadFromFile( "../0.png")){
        printf("Failed to load 0.png texture image!\n");
        success = false;
    }

    if(!points[1].loadFromFile( "../1.png")){
        printf("Failed to load 1.png texture image!\n");
        success = false;
    }

    if(!points[2].loadFromFile( "../2.png")){
        printf("Failed to load 2.png texture image!\n");
        success = false;
    }

    if(!points[3].loadFromFile( "../3.png")){
        printf("Failed to load 3.png texture image!\n");
        success = false;
    }

    if(!points[4].loadFromFile( "../colon.png")){
        printf("Failed to load colon.png texture image!\n");
        success = false;
    }

    if(!player1won.loadFromFile( "../player1won.png")){
        printf("Failed to load player1won.png texture image!\n");
        success = false;
    }

    if(!player2won.loadFromFile( "../player2won.png")){
        printf("Failed to load player2won.png texture image!\n");
        success = false;
    }

    if(!pointingArrow.loadFromFile( "../pointingArrow.png")){
        printf("Failed to load pointingArrow.png texture image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded images
    for (LTexture t : points){
        t.free();
    }
    player1won.free();
    player2won.free();
    pointingArrow.free();

    //Close game controller
    SDL_JoystickClose( gGameController );
    gGameController = NULL;

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

LTexture::LTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    //Deallocate
    free();
}

bool LTexture::loadFromFile(std::string path)
{
    //Get rid of preexisting texture
    free();

    //The final texture
    SDL_Texture* newTexture = nullptr;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        //Color key image
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0x11, 0x17));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

void LTexture::free()
{
    //Free texture if it exists
    if (mTexture != NULL)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::render1(int x, int y)
{
    //Set rendering space and render1 to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

void LTexture::render2(int x, int y, int width, int height)
{
    //Set rendering space and render1 to screen
    SDL_Rect renderQuad = { x, y, width, height };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

int LTexture::getWidth() const {
    return mWidth;
}

int LTexture::getHeight() const {
    return mHeight;
}

Player::Player(float x_pos, float y_pos, std::string tex, KEYVARIANTS keyVar)
{
    keyVariant = keyVar;

    texture.loadFromFile(tex);
    pWidth = map_tile_size * PLAYER_WIDTH;
    pHeight = map_tile_size * PLAYER_WIDTH;

    //Initialize the offsets
    mPosX = x_pos;
    mPosY = y_pos;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    //initialize points
    activePoints = 0;
    oldPoints = 0;
}

Player::~Player() {
    texture.free();
}

void Player::handleEvent(SDL_Event& e)
{
    if (keyVariant == KEYBOARD) {
        //If a key was pressed
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
        {
            //Adjust the velocity
            switch (e.key.keysym.sym)
            {
                case SDLK_UP: mVelY -= PLAYER_VEL; break;
                case SDLK_w: mVelY -= PLAYER_VEL; break;

                case SDLK_DOWN: mVelY += PLAYER_VEL; break;
                case SDLK_s: mVelY += PLAYER_VEL; break;

                case SDLK_LEFT: mVelX -= PLAYER_VEL; break;
                case SDLK_a: mVelX -= PLAYER_VEL; break;

                case SDLK_RIGHT: mVelX += PLAYER_VEL; break;
                case SDLK_d: mVelX += PLAYER_VEL; break;
            }
        }
            //If a key was released
        else if (e.type == SDL_KEYUP && e.key.repeat == 0)
        {
            //Adjust the velocity
            switch (e.key.keysym.sym)
            {
                case SDLK_UP: mVelY += PLAYER_VEL; break;
                case SDLK_w: mVelY += PLAYER_VEL; break;

                case SDLK_DOWN: mVelY -= PLAYER_VEL; break;
                case SDLK_s: mVelY -= PLAYER_VEL; break;

                case SDLK_LEFT: mVelX += PLAYER_VEL; break;
                case SDLK_a: mVelX += PLAYER_VEL; break;

                case SDLK_RIGHT: mVelX -= PLAYER_VEL; break;
                case SDLK_d: mVelX -= PLAYER_VEL; break;
            }
        }
    } else if (keyVariant == GAMEPAD) {
        if (e.type == SDL_JOYAXISMOTION) {
            //X motion
            if (e.jaxis.axis == 0) {
                if (e.jaxis.value < - JOYSTICK_DEAD_ZONE) {
                    mVelX = -PLAYER_VEL;
                } else if (e.jaxis.value > JOYSTICK_DEAD_ZONE) {
                    mVelX = PLAYER_VEL;
                } else {
                    mVelX = 0;
                }
            }
            //Y motion
            if (e.jaxis.axis == 1) {
                if (e.jaxis.value < - JOYSTICK_DEAD_ZONE) {
                    mVelY = -PLAYER_VEL;
                } else if (e.jaxis.value > JOYSTICK_DEAD_ZONE) {
                    mVelY = PLAYER_VEL;
                } else {
                    mVelY = 0;
                }
            }
        }
    } else {
        std::cout << "Wrong keyVariant initialised!" << std::endl;
    }
}

void Player::move()
{
    //Move the dot left or right
    mPosX += mVelX;

    //If the dot went too far to the left or right
    if ((mPosX < camera_x) || (mPosX + pWidth > camera_x + SCREEN_WIDTH))
    {
        //Move back
        mPosX -= mVelX;
    }

    //Move the dot up or down
    mPosY += mVelY;

    //If the dot went too far up or down
    if ((mPosY < camera_y) || (mPosY + pHeight > camera_y + SCREEN_HEIGHT))
    {
        //Move back
        mPosY -= mVelY;
    }
}

void Player::render()
{
    texture.render1(mPosX, mPosY);
}

void Player::render(int x, int y) {
    texture.render2(mPosX - x, mPosY - y, pWidth, pHeight);
}

float Player::getPosX()
{
    return mPosX;
}

float Player::getPosY()
{
    return mPosY;
}

int Player::getPWidth() {
    return pWidth;
}

int Player::getPHeight() {
    return pHeight;
}

void Player::setPosition(float x, float y) {
    mPosX = x;
    mPosY = y;
}

void Player::addPoint() {
    activePoints++;
    oldPoints = activePoints - 1;
}

void Player::resetPoints() {
    activePoints = 0;
    oldPoints = 0;
}

void Player::updateOldPoints() {
    oldPoints = activePoints;
}

Level::Level(std::string map) {
    std::vector<int> line;
    availablePositions.clear();
    for (int j = 0; j < levelMapHeight; j++) {
        for (int i = 0; i < levelMapWidth; i++) {
            switch (map[j * levelMapWidth + i])
            {
                case '#':
                    levelWalls.push_back(new Wall(i * map_tile_size, j * map_tile_size, map_tile_size, map_tile_size));
                    line.push_back(UNAVAILABLE);
                    break;
                case 'o':
                    levelWalls.push_back(new Wall(i * map_tile_size, j * map_tile_size, map_tile_size, map_tile_size));
                    levelWalls.back()->texture.loadFromFile("../level/target.png");
                    star = levelWalls.back();
                    line.push_back(UNAVAILABLE);
                    levelWalls.back()->isTarget = true;
                    break;
                default:
                    std::cout << "Available position detected: " << i << ", " << j << ")" << std::endl;
                    line.push_back(AVAILABLE);
                    break;
            }
        }
        availablePositions.push_back(line);
        line.clear();
    }
}

Level::~Level() {
    for(Wall* w : levelWalls) {
        w->texture.free();
    }
}

void Level::loadNewMap(std::string map) {
    std::vector<int> line;
    for (Wall* w : levelWalls) {
        w->texture.free();
        delete w;
    }
    levelWalls.clear();
    availablePositions.clear();
    for (int j = 0; j < levelMapHeight; j++) {
        for (int i = 0; i < levelMapWidth; i++) {
            switch (map[j * levelMapWidth + i])
            {
                case '#':
                    levelWalls.push_back(new Wall(i * map_tile_size, j * map_tile_size, map_tile_size, map_tile_size));
                    line.push_back(UNAVAILABLE);
                    break;
                case 'o':
                    levelWalls.push_back(new Wall(i * map_tile_size, j * map_tile_size, map_tile_size, map_tile_size));
                    levelWalls.back()->texture.loadFromFile("../level/target.png");
                    star = levelWalls.back();
                    line.push_back(UNAVAILABLE);
                    levelWalls.back()->isTarget = true;
                    break;
                default:
                    //std::cout << "Available position detected: " << i << ", " << j << ")" << std::endl;
                    line.push_back(AVAILABLE);
                    break;
            }
        }
        availablePositions.push_back(line);
        line.clear();
    }
}

void Level::render(float init_x, float init_y) {
    for (Wall* w : levelWalls) {
        w->texture.render2(w->xp - init_x, w->yp - init_y, w->w, w->h);
//        if(w->isTarget) {
//            std::cout << ", star xp: " << w->xp << ", star yp: " << w->yp << std::endl;
//        }
    }
}

std::vector<std::vector<int>> Level::getAvailablePositions() {
    return availablePositions;
}

double distanceSquared( float x1, float y1, float x2, float y2 )
{
    float deltaX = x2 - x1;
    float deltaY = y2 - y1;
    return deltaX*deltaX + deltaY*deltaY;
}

void checkCircularCollision(Player &circlePlayer, Wall* wall, Level &level, Player &player1)
{
    float circle_center_x = circlePlayer.getPosX() + circlePlayer.getPWidth() * 0.5;
    float circle_center_y = circlePlayer.getPosY() + circlePlayer.getPHeight() * 0.5;
    float circle_radius = circlePlayer.getPWidth() * 0.5;

    //Closest point on collision box
    float cX, cY;
    float resultX, resultY;

    //Find closest x offset
    if(circle_center_x < wall->xp )
    {
        cX = wall->xp;
    }
    else if(circle_center_x > wall->xp + wall->w )
    {
        cX = wall->xp + wall->w;
    }
    else
    {
        cX = circle_center_x;
    }

    //Find closest y offset
    if(circle_center_y < wall->yp )
    {
        cY = wall->yp;
    }
    else if(circle_center_y > wall->yp + wall->h )
    {
        cY = wall->yp + wall->h;
    }
    else
    {
        cY = circle_center_y;
    }

    if(distanceSquared(circle_center_x, circle_center_y, cX, cY) < circle_radius * circle_radius){
        if (wall->isTarget){
            player1.updateOldPoints();
            circlePlayer.addPoint();
            switchLevel(level, activeLevelMap, player1, circlePlayer);
        } else {
            float dist_to_circle_centerX = circle_center_x - cX;
            float dist_to_circle_centerY = circle_center_y - cY;
            float distance = sqrtf(
                    dist_to_circle_centerX * dist_to_circle_centerX + dist_to_circle_centerY * dist_to_circle_centerY);
            float normalX = dist_to_circle_centerX / distance;
            float normalY = dist_to_circle_centerY / distance;
            resultX = cX + normalX * (distance + circlePlayer.PLAYER_VEL) - circle_radius;
            resultY = cY + normalY * (distance + circlePlayer.PLAYER_VEL) - circle_radius;
            circlePlayer.setPosition(resultX, resultY);
        }
    }
}

void checkRectangularCollision(Player &player1, Wall* w, Level &level, Player &player2){
    //left collisions
    if (int(player1.getPosX()) == w->xp + w->w && player1.getPosY() > w->yp - player1.getPHeight() && player1.getPosY() < w->yp + w->h) {
        if (w->isTarget){
            player2.updateOldPoints();
            player1.addPoint();
            switchLevel(level, activeLevelMap, player1, player2);
        } else {
            player1.setPosition(w->xp + w->w + 1, player1.getPosY());
        }
    }
    //right collision
    if (int(player1.getPosX() + player1.getPWidth()) == w->xp && player1.getPosY() > w->yp - player1.getPHeight() && player1.getPosY() < w->yp + w->h) {
        if (w->isTarget){
            player2.updateOldPoints();
            player1.addPoint();
            switchLevel(level, activeLevelMap, player1, player2);
        } else {
            player1.setPosition(w->xp - player1.getPWidth() - 1, player1.getPosY());
        }
    }
    //top collision
    if (int(player1.getPosY()) == w->yp + w->h && player1.getPosX() > w->xp - player1.getPWidth() && player1.getPosX() < w->xp + w->w) {
        if (w->isTarget){
            player2.updateOldPoints();
            player1.addPoint();
            switchLevel(level, activeLevelMap, player1, player2);
        } else {
            player1.setPosition(player1.getPosX(), w->yp + w->h + 1);
        }
    }
    //bottom collision
    if (int(player1.getPosY() + player1.getPHeight()) == w->yp && player1.getPosX() > w->xp - player1.getPWidth() && player1.getPosX() < w->xp + w->w) {
        if (w->isTarget){
            player2.updateOldPoints();
            player1.addPoint();
            switchLevel(level, activeLevelMap, player1, player2);
        } else {
            player1.setPosition(player1.getPosX(), w->yp - player1.getPHeight() - 1);
        }
    }
}

void switchLevel(Level &level, std::string* &newMap, Player &player1, Player &player2){
    std::string map = *newMap;
    if (map == levelMaps[2]) {
        levelHasJustBeenChanged = true;
        newMap = &levelMaps[0];
    }
    else if(map == levelMaps[0]) {
        SDL_RenderClear(gRenderer);
        renderOldScore(player1, player2);
        SDL_RenderPresent(gRenderer);
        SDL_Delay(1000);
        SDL_RenderClear(gRenderer);
        renderActiveScore(player1, player2);
        SDL_RenderPresent(gRenderer);
        SDL_Delay(1000);
        SDL_RenderClear(gRenderer);
        renderActiveScore(player1, player2);
        if (player1.getActivePoints() == 2 || player1.getActivePoints() == 3){
            player1won.render1(SCREEN_WIDTH * 0.5 - player1won.getWidth() * 0.5, SCREEN_HEIGHT * 0.5 - player1won.getWidth() * 0.5);
        } else {
            player2won.render1(SCREEN_WIDTH * 0.5 - player2won.getWidth() * 0.5, SCREEN_HEIGHT * 0.5 - player2won.getWidth() * 0.5);
        }
        SDL_RenderPresent(gRenderer);
        SDL_Delay(5000);
        player1.resetPoints();
        player2.resetPoints();
        levelHasJustBeenChanged = true;
        newMap++;
    }
    else {
        levelHasJustBeenChanged = true;
        newMap++;
    }
    readLevelMap(map);
    level.loadNewMap(levelMapString);

    float p1NewX = 0;
    float p1NewY = 0;
    float p2NewX = 0;
    float p2NewY = 0;
    generateRandomPositions(p1NewX, p1NewY, level);
    generateRandomPositions(p2NewX, p2NewY, level);
    player1.setPosition(p1NewX, p1NewY);
    player2.setPosition(p2NewX, p2NewY);

    camera_x = 0.0f;
    camera_y = 0.0f;
}

void generateRandomPositions(float &x, float &y, Level &level) {
    int range1 = rand() % 3 + 3;
    int range2 = rand() % 3 + 3;
    for (int i = range1; i > 0; i--) {
        for (int j = range2; j > 0; j--) {
            if(level.getAvailablePositions()[i][j] == AVAILABLE){
                x = j * map_tile_size + 0.25 * map_tile_size;
                y = i * map_tile_size + 0.25 * map_tile_size;
                level.getAvailablePositions()[i][j] = UNAVAILABLE;
                return;
            }
        }
    }
}

void renderActiveScore(Player &player1, Player &player2){
    points[player1.getActivePoints()].render2(SCREEN_WIDTH * 0.5 - 50, SCREEN_HEIGHT - 50, 50, 50);
    points[player2.getActivePoints()].render2(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT - 50, 50, 50);
    points[4].render2(SCREEN_WIDTH * 0.5 - 25, SCREEN_HEIGHT - 50, 50, 50);
}

void renderOldScore(Player &player1, Player &player2){
    points[player1.getOldPoints()].render2(SCREEN_WIDTH * 0.5 - 50, SCREEN_HEIGHT - 50, 50, 50);
    points[player2.getOldPoints()].render2(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT - 50, 50, 50);
    points[4].render2(SCREEN_WIDTH * 0.5 - 25, SCREEN_HEIGHT - 50, 50, 50);
}

void renderStar(Player &player1, Player &player2) {

    if(!(star->xp > camera_x - star->w &&
         star->xp < camera_x + SCREEN_WIDTH &&
         star->yp > camera_y - star->h &&
         star->yp < camera_y + SCREEN_HEIGHT)){
        float averagePlayersX;
        float averagePlayersY;
        if (gGameController != NULL) {
            averagePlayersX = (player1.getPosX() + (player1.getPWidth() * 0.5) +
                               player2.getPosX() + (player2.getPWidth() * 0.5)) * 0.5;
            averagePlayersY = (player1.getPosY() + (player1.getPHeight() * 0.5) +
                               player2.getPosY() + (player2.getPHeight() * 0.5)) * 0.5;
        } else {
            averagePlayersX = (player1.getPosX() + (player1.getPWidth() * 0.5));
            averagePlayersY = (player1.getPosY() + (player1.getPHeight() * 0.5));
        }

        float deltax = star->xp + (star->w * 0.5) - averagePlayersX;
        float deltay = star->yp + (star->h * 0.5) - averagePlayersY;

        float angle = (atan2(deltay, deltax) * 180.0000)/ 3.14159265;

        SDL_Rect renderQuad = { int((SCREEN_WIDTH * 0.5) - pointingArrow.getWidth() * 0.5),
                                int(SCREEN_HEIGHT - 50 - pointingArrow.getHeight()),
                                pointingArrow.getWidth(),
                                pointingArrow.getHeight() };
        SDL_RenderCopyEx(gRenderer, pointingArrow.getTexture(), NULL, &renderQuad, angle , NULL, SDL_FLIP_NONE);
    }
}