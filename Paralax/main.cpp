#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <time.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int JOYSTICK_DEAD_ZONE = 10000;
const int PLAYER_VELOCITY = 2;
const float PLAYER_WIDTH = 0.5; // * map_tile_size
float jumpHeight = 5.0f;
float jumpLength = 6.0f;
float GRAVITY = 2 * jumpHeight * (PLAYER_VELOCITY * PLAYER_VELOCITY) / (jumpLength * jumpLength);
bool FALLING = false;
bool FASTER_FALLING_SWITCH = false;
unsigned int DOUBLEJUMP = 2;
float FRONTSPEED = 1.2;
float BGROUND1SPEED = 0.4;
float BGROUND2SPEED = 0.1;

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

    //Creates image from font string
    bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

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
    int scale = 1;
    bool isActive = true;
    bool isTarget = false;
    LTexture texture;
};

class Player
{
public:
    //Maximum axis velocity of the dot
    float PLAYER_VEL = PLAYER_VELOCITY;

    //Initializes the variables
    Player(float x_pos, float y_pos, int tile_size, std::string tex, KEYVARIANTS keyVar);

    //Frees the texture
    ~Player();

    //Takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    //Moves the dot
    void move(float dT);
    void setPosition(float x, float y);

    //Shows the dot on the screen relative to the camera
    void render();
    void render(int x, int y);

    //Position accessors
    float getPosX();
    float getPosY();
    int getPWidth();
    int getPHeight();

    //The velocity of the dot
    float mVelX, mVelY;

private:
    KEYVARIANTS keyVariant;

    LTexture texture;
    int pWidth, pHeight;

    //The X and Y offsets of the dot
    float mPosX, mPosY;
};

struct Layer{
    std::string mapString;
    int width;
    int height;
    int tileSize;
};

class Level{
public:
    Level(Layer layer, float speed = 1.0f);
    ~Level();

    void render(float init_x, float init_y);
    std::vector<std::vector<int>> getAvailablePositions();
    std::vector<Wall*> getlevelWalls() { return levelWalls; }
    void setSpeed(float speed) {this->speed = speed;}

private:
    std::vector<Wall*> levelWalls;
    std::vector<std::vector<int>> availablePositions;
    float speed;
};

Layer frontGround;
Layer backGround1;
Layer backGround2;
Layer backGround3;

float camera_x = 0.0f;
float camera_y = 0.0f;

double lastTime;
double currentTime = 0.0;
double passedTime = 0.0;
double unprocessedTime = 0.0;
float frameTime = 1 / 60.0;

SDL_Renderer* gRenderer = nullptr;
SDL_Joystick* gGameController = NULL;
SDL_Window* gWindow = nullptr;
TTF_Font* gFont = nullptr;
LTexture paralax1Texture;
LTexture paralax2Texture;
LTexture paralax3Texture;

void input(Player& player, bool& quit);
void update(Player& player, Level& front, Level& back1, Level& back2, Level& back3, double dt);
void render(Player& player, Level& front, Level& back1, Level& back2, Level& back3);
bool init();
bool readLevelMaps();
void checkCircularCollision(Player &circlePlayer, Wall* wall, Level &level);
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
            srand(time(NULL));

            bool shouldRender = false;
            readLevelMaps();
            frontGround.tileSize = SCREEN_WIDTH / 10;
            backGround1.tileSize = SCREEN_WIDTH / 10;
            backGround2.tileSize = SCREEN_WIDTH / 10;
            backGround3.tileSize = SCREEN_WIDTH / 10;
            Level level(frontGround);
            Level backgroundlevel1(backGround1, FRONTSPEED);
            Level backgroundlevel2(backGround2, BGROUND1SPEED);
            Level backgroundlevel3(backGround3, BGROUND2SPEED);
            Player player((0.5 * frontGround.width) * frontGround.tileSize, (frontGround.height - 4) * frontGround.tileSize, frontGround.tileSize,"../circle.png", GAMEPAD);

            //While application is running
            lastTime = SDL_GetTicks() * 0.01;
            while (!quit)
            {
                currentTime = SDL_GetTicks() * 0.01;
                passedTime = currentTime - lastTime;
                lastTime = currentTime;
                unprocessedTime += passedTime;

                input(player, quit);

                while (unprocessedTime >= frameTime) {
                    shouldRender = true;
                    unprocessedTime -= frameTime;
                    update(player, level, backgroundlevel1, backgroundlevel2, backgroundlevel3, passedTime);
                }

                if(shouldRender){
                    render(player, level, backgroundlevel1, backgroundlevel2, backgroundlevel3);
                }
            }
        }
    }

    close();
    return 0;
}

void input(Player& player, bool& quit) {
    SDL_Event e;
    //Handle events on queue
    while (SDL_PollEvent(&e) != 0)
    {
        //User requests quit
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }

        if (gGameController != NULL) { player.handleEvent(e); }
    }
}

void update(Player& player, Level& level, Level& back1, Level& back2, Level& back3, double dt) {
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
        player.move(dt);
        camera_x = (player.getPosX() + player.getPWidth() * 0.5) - SCREEN_WIDTH * 0.5;
        camera_y = (player.getPosY() + player.getPHeight() * 0.5) - SCREEN_HEIGHT * 0.5;
    }

    //checking the camera movement
    if( camera_x < 0 ) { camera_x = 0; }
    if( camera_y < 0 ) { camera_y = 0; }
    if( camera_x > frontGround.width  * frontGround.tileSize - SCREEN_WIDTH  ) { camera_x = frontGround.width  * frontGround.tileSize - SCREEN_WIDTH ; }
    if( camera_y > frontGround.height * frontGround.tileSize - SCREEN_HEIGHT ) { camera_y = frontGround.height * frontGround.tileSize - SCREEN_HEIGHT; }

    //checking if the players aren't colliding with the map
    for (Wall* w : level.getlevelWalls()){
        if (w->isActive && SDL_NumJoysticks() > 0 && gGameController != nullptr) {
            checkCircularCollision(player, w, level);
        }
    }

    back1.setSpeed(FRONTSPEED);
    back2.setSpeed(BGROUND1SPEED);
    back3.setSpeed(BGROUND2SPEED);
}

void render(Player& player, Level& front, Level& back1, Level& back2, Level& back3) {
    //Clear screen
    SDL_RenderClear(gRenderer);

    //rendering level map based on previously calculated camera position
    back3.render(camera_x, camera_y);
    back2.render(camera_x, camera_y);
    front.render(camera_x, camera_y);
    //rendering players
    if (gGameController != nullptr) {
        player.render(camera_x, camera_y);
    }
    back1.render(camera_x, camera_y);

    paralax3Texture.render1(20, SCREEN_HEIGHT - paralax1Texture.getHeight() - 10);
    paralax2Texture.render1(20, SCREEN_HEIGHT - 2 * paralax2Texture.getHeight() - 20);
    paralax1Texture.render1(20, SCREEN_HEIGHT - 3 * paralax3Texture.getHeight() - 30);

    //Update screen
    SDL_RenderPresent(gRenderer);
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
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
                SDL_SetRenderDrawColor(gRenderer, 0xCA, 0xF5, 0xFF, 0xFF);

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags))
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }

                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
            }
        }
    }

    return success;
}

bool readLevelMaps() {
    bool success = true;
    std::ifstream file1;
    std::ifstream file2;
    std::ifstream file3;
    std::ifstream file4;
    std::string line;


    file1.open("../paralax1.txt");
    frontGround.mapString = "";
    frontGround.height = 0;
    frontGround.width = 0;
    if (!file1.good()) {
        std::cout << "Wrong file paralax1" << std::endl;
        return !success;
    }
    else {
        frontGround.mapString.clear();
        while (!file1.eof()) {
            frontGround.height++;
            std::getline(file1, line);
            frontGround.mapString += line;
        }
        frontGround.width = line.size();

        file1.close();
    }
    line.clear();

    file2.open("../paralax-1.txt");
    backGround1.mapString = "";
    backGround1.height = 0;
    backGround1.width = 0;
    if (!file2.good()) {
        std::cout << "Wrong file paralax2" << std::endl;
        return !success;
    }
    else {
        backGround1.mapString.clear();
        while (!file2.eof()) {
            backGround1.height++;
            std::getline(file2, line);
            backGround1.mapString += line;
        }
        backGround1.width = line.size();

        file2.close();
    }
    line.clear();


    file3.open("../paralax2.txt");
    backGround2.mapString = "";
    backGround2.height = 0;
    backGround2.width = 0;
    if (!file3.good()) {
        std::cout << "Wrong file paralax3" << std::endl;
        return !success;
    }
    else {
        backGround2.mapString.clear();
        while (!file3.eof()) {
            backGround2.height++;
            std::getline(file3, line);
            backGround2.mapString += line;
        }
        backGround2.width = line.size();

        file3.close();
    }
    line.clear();


    file4.open("../paralax3.txt");
    backGround3.mapString = "";
    backGround3.height = 0;
    backGround3.width = 0;
    if (!file4.good()) {
        std::cout << "Wrong file paralax4" << std::endl;
        return !success;
    }
    else {
        backGround3.mapString.clear();
        while (!file4.eof()) {
            backGround3.height++;
            std::getline(file4, line);
            backGround3.mapString += line;
        }
        backGround3.width = line.size();

        file4.close();
    }
    return success;
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;
//    if(!points[0].loadFromFile( "0.png")){
//        printf("Failed to load 0.png texture image!\n");
//        success = false;
//    }

    //Open the font
    gFont = TTF_OpenFont( "../SourceSansPro-Regular.ttf", 20 );
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        //Render text
        SDL_Color textColor = { 255, 255, 255 };
        if( !paralax1Texture.loadFromRenderedText("Front layer: " + std::to_string(FRONTSPEED), textColor ) )
        {
            printf( "Failed to render jump height text texture!\n" );
            success = false;
        }
        if( !paralax2Texture.loadFromRenderedText("Second layer: " + std::to_string(BGROUND1SPEED), textColor ) )
        {
            printf( "Failed to render jump length text texture!\n" );
            success = false;
        }
        if( !paralax3Texture.loadFromRenderedText("Third layer: " + std::to_string(BGROUND2SPEED), textColor ) )
        {
            printf( "Failed to render faster falling switch text texture!\n" );
            success = false;
        }
    }

    return success;

    return success;
}

void close()
{
    //Free loaded images
    paralax1Texture.free();
    paralax2Texture.free();
    paralax3Texture.free();

    //Close game controller
    SDL_JoystickClose( gGameController );
    gGameController = NULL;

    //Free global font
    TTF_CloseFont( gFont );
    gFont = NULL;

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = NULL;

    //Quit SDL subsystems
    TTF_Quit();
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

bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor) {
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    return mTexture != NULL;
}

Player::Player(float x_pos, float y_pos, int tile_size, std::string tex, KEYVARIANTS keyVar)
{
    keyVariant = keyVar;

    texture.loadFromFile(tex);
    pWidth = tile_size * PLAYER_WIDTH;
    pHeight = tile_size * PLAYER_WIDTH;

    //Initialize the offsets
    mPosX = x_pos;
    mPosY = y_pos;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

Player::~Player() {
    texture.free();
}

void Player::handleEvent(SDL_Event& e)
{
    if (keyVariant == KEYBOARD)
    {
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
        }
        if (e.type == SDL_JOYBUTTONDOWN) {
            if (e.jbutton.button == SDL_CONTROLLER_BUTTON_A && DOUBLEJUMP != 0){
                mVelY = -2 * jumpHeight * PLAYER_VELOCITY / jumpLength;
//                std::cout << "skakanie" << std::endl;
                DOUBLEJUMP--;
            }
        }
        //If a key was pressed
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
        {
            SDL_Color textColor = { 255, 255, 255 };
            //Adjust the velocity
            switch (e.key.keysym.sym)
            {
                case SDLK_o:
                    FRONTSPEED += 0.1;
                    paralax1Texture.loadFromRenderedText("First layer: " + std::to_string(FRONTSPEED), textColor);
                    break;
                case SDLK_i:
                    FRONTSPEED -= 0.1;
                    paralax1Texture.loadFromRenderedText("First layer: " + std::to_string(FRONTSPEED), textColor);
                    break;
                case SDLK_l:
                    BGROUND1SPEED += 0.1;
                    paralax2Texture.loadFromRenderedText("Second layer: " + std::to_string(BGROUND1SPEED), textColor);
                    break;
                case SDLK_k:
                    BGROUND1SPEED -= 0.1;
                    paralax2Texture.loadFromRenderedText("Second layer: " + std::to_string(BGROUND1SPEED), textColor);
                    break;
                case SDLK_COMMA:
                    BGROUND2SPEED += 0.1;
                    paralax3Texture.loadFromRenderedText("Third layer: " + std::to_string(BGROUND2SPEED), textColor);
                    break;
                case SDLK_m:
                    BGROUND2SPEED -= 0.1;
                    paralax3Texture.loadFromRenderedText("Third layer: " + std::to_string(BGROUND2SPEED), textColor);
                    break;
            }
        }
    } else {
        std::cout << "Wrong keyVariant initialised!" << std::endl;
    }
}

void Player::move(float dT)
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
    mPosY += mVelY * dT * 25 + 0.5 * GRAVITY * dT * dT;

    //If the dot went too far up or down
    if ((mPosY < camera_y) || (mPosY + pHeight > camera_y + SCREEN_HEIGHT))
    {
        //Move back
        mPosY -= mVelY * dT * 25 + 0.5 * GRAVITY * dT * dT;
        mVelY = 0;
    }

    if (FASTER_FALLING_SWITCH) {
        FALLING = mVelY > 0.5;

        if ( FALLING ){
            mVelY += 5 * GRAVITY * dT;
        } else {
            mVelY += GRAVITY * dT;
        }
    } else {
        mVelY += GRAVITY * dT;
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

Level::Level(Layer layer, float speed) {
    this->speed = speed;
    std::vector<int> line;
    availablePositions.clear();
    for (int j = 0; j < layer.height; j++) {
        for (int i = 0; i < layer.width; i++) {
            switch (layer.mapString[j * layer.width + i])
            {
                case '#':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/ground.png");
                    line.push_back(UNAVAILABLE);
                    break;
                case 's':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/sun.png");
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                case 't':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/tree_tile.png");
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                case 'T':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/tree_tile.png");
                    levelWalls.back()->scale = 2;
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                case 'o':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/Rock.png");
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                case 'O':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/Rock.png");
                    levelWalls.back()->isActive = false;
                    levelWalls.back()->scale = 2;
                    line.push_back(AVAILABLE);
                    break;
                case 'c':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/cloud.png");
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                case 'f':
                    levelWalls.push_back(new Wall(i * layer.tileSize, j * layer.tileSize, layer.tileSize, layer.tileSize));
                    levelWalls.back()->texture.loadFromFile("../paralax_assets/flower.png");
                    levelWalls.back()->scale = 2;
                    levelWalls.back()->isActive = false;
                    line.push_back(AVAILABLE);
                    break;
                default:
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

void Level::render(float init_x, float init_y) {
    for (Wall* w : levelWalls) {
        w->texture.render2(w->xp - init_x * this->speed, w->yp - init_y, w->w * w->scale, w->h * w->scale);
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

void checkCircularCollision(Player &circlePlayer, Wall* wall, Level &level)
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
        float dist_to_circle_centerX = circle_center_x - cX;
        float dist_to_circle_centerY = circle_center_y - cY;
        float distance = sqrtf(dist_to_circle_centerX * dist_to_circle_centerX + dist_to_circle_centerY * dist_to_circle_centerY);
        float normalX = dist_to_circle_centerX / distance;
        float normalY = dist_to_circle_centerY / distance;
        resultX = cX + normalX * (distance + circlePlayer.PLAYER_VEL) - circle_radius;
        resultY = cY + normalY * (distance + circlePlayer.PLAYER_VEL) - circle_radius;
        circlePlayer.setPosition(resultX, resultY);
        if (cX != wall->xp && cX != wall->xp + wall->w){
            circlePlayer.mVelY = 0;
            DOUBLEJUMP = 2;
            FALLING = false;
        }
    }
}