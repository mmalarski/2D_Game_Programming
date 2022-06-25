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
    bool isTarget = false;
    LTexture texture;
};

class Player
{
public:
    //Maximum axis velocity of the dot
    float PLAYER_VEL = PLAYER_VELOCITY;

    //Initializes the variables
    Player(float x_pos, float y_pos, std::string tex, KEYVARIANTS keyVar);

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

std::string levelMapString = "";
int levelMapWidth = 0;
int levelMapHeight = 0;
int map_tile_size = 40;

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
LTexture jumpHeightTexture;
LTexture jumpLengthTexture;
LTexture fasterFallingSwitch;

void input(Player& player, bool& quit);
void update(Player& player, Level& level, double dt);
void render(Player& player, Level& level);
bool init();
bool readLevelMap(std::string filename);
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
            readLevelMap("../jump_map.txt");
            map_tile_size = SCREEN_WIDTH / 10;
            Level level(levelMapString);
            Player player((0.5 * levelMapWidth) * map_tile_size, (levelMapHeight - 4) * map_tile_size, "../circle.png", GAMEPAD);

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
                    update(player, level, passedTime);
                }

                if(shouldRender){
                    render(player, level);
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

void update(Player& player, Level& level, double dt) {
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
    if( camera_x > levelMapWidth  * map_tile_size - SCREEN_WIDTH  ) { camera_x = levelMapWidth  * map_tile_size - SCREEN_WIDTH ; }
    if( camera_y > levelMapHeight * map_tile_size - SCREEN_HEIGHT ) { camera_y = levelMapHeight * map_tile_size - SCREEN_HEIGHT; }

    //checking if the players aren't colliding with the map
    for (Wall* w : level.getlevelWalls()){
        if (SDL_NumJoysticks() > 0 && gGameController != nullptr) {
            checkCircularCollision(player, w, level);
        }
    }

}

void render(Player& player, Level& level) {
    //Clear screen
    SDL_RenderClear(gRenderer);

    //rendering players
    if (gGameController != nullptr) {
        player.render(camera_x, camera_y);
    }


    //rendering level map based on previously calculated camera position
    level.render(camera_x, camera_y);
    jumpHeightTexture.render1(20, SCREEN_HEIGHT - jumpHeightTexture.getHeight() - 10);
    jumpLengthTexture.render1(20, SCREEN_HEIGHT - 2 * jumpLengthTexture.getHeight() - 20);
    fasterFallingSwitch.render1(400, SCREEN_HEIGHT - jumpHeightTexture.getHeight() - 10);

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
        gWindow = SDL_CreateWindow("Jumping playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            const char* path = "../wall.bmp";
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
        SDL_Color textColor = { 0, 0, 0 };
        if( !jumpHeightTexture.loadFromRenderedText("Jump height: " + std::to_string(jumpHeight), textColor ) )
        {
            printf( "Failed to render jump height text texture!\n" );
            success = false;
        }
        if( !jumpLengthTexture.loadFromRenderedText("Jump length: " + std::to_string(jumpLength), textColor ) )
        {
            printf( "Failed to render jump length text texture!\n" );
            success = false;
        }
        if( !fasterFallingSwitch.loadFromRenderedText("Faster falling: OFF", textColor ) )
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
    jumpHeightTexture.free();

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
            SDL_Color textColor = { 0, 0, 0 };
            //Adjust the velocity
            switch (e.key.keysym.sym)
            {
                case SDLK_i:
                    jumpLength++;
                    jumpLengthTexture.loadFromRenderedText("Jump length: " + std::to_string(jumpLength), textColor);
                    break;
                case SDLK_o:
                    jumpHeight++;
                    jumpHeightTexture.loadFromRenderedText("Jump height: " + std::to_string(jumpHeight), textColor);
                    break;
                case SDLK_k:
                    if (jumpLength > 1) jumpLength--;
                    jumpLengthTexture.loadFromRenderedText("Jump length: " + std::to_string(jumpLength), textColor);
                    break;
                case SDLK_l:
                    if (jumpHeight > 1) jumpHeight--;
                    jumpHeightTexture.loadFromRenderedText("Jump height: " + std::to_string(jumpHeight), textColor);
                    break;
                case SDLK_g:
                    FASTER_FALLING_SWITCH = !FASTER_FALLING_SWITCH;
                    if (FASTER_FALLING_SWITCH) {
                        fasterFallingSwitch.loadFromRenderedText("Faster falling: ON", textColor);
                    } else {
                        fasterFallingSwitch.loadFromRenderedText("Faster falling: OFF", textColor);
                    }
                    break;
            }
            GRAVITY = 2 * jumpHeight * (PLAYER_VELOCITY * PLAYER_VELOCITY) / (jumpLength * jumpLength);
            std::cout << "Gravity: " << GRAVITY << ", v_0: " << -2 * jumpHeight * PLAYER_VELOCITY / jumpLength << std::endl;
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
                    line.push_back(UNAVAILABLE);
                    levelWalls.back()->isTarget = true;
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

void Level::render(float init_x, float init_y) {
    for (Wall* w : levelWalls) {
        w->texture.render2(w->xp - init_x, w->yp - init_y, w->w, w->h);
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

void checkRectangularCollision(Player &player1, Wall* w, Level &level, Player &player2){
    //left collisions
    if (int(player1.getPosX()) == w->xp + w->w && player1.getPosY() > w->yp - player1.getPHeight() && player1.getPosY() < w->yp + w->h) {
        player1.setPosition(w->xp + w->w + 1, player1.getPosY());
    }
    //right collision
    if (int(player1.getPosX() + player1.getPWidth()) == w->xp && player1.getPosY() > w->yp - player1.getPHeight() && player1.getPosY() < w->yp + w->h) {
        player1.setPosition(w->xp - player1.getPWidth() - 1, player1.getPosY());
    }
    //top collision
    if (int(player1.getPosY()) == w->yp + w->h && player1.getPosX() > w->xp - player1.getPWidth() && player1.getPosX() < w->xp + w->w) {
        player1.setPosition(player1.getPosX(), w->yp + w->h + 1);
    }
    //bottom collision
    if (int(player1.getPosY() + player1.getPHeight()) == w->yp && player1.getPosX() > w->xp - player1.getPWidth() && player1.getPosX() < w->xp + w->w) {
        player1.setPosition(player1.getPosX(), w->yp - player1.getPHeight() - 1);
    }
}