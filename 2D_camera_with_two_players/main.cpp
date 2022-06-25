#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL_image.h>
#include <vector>

enum KEYVARIANTS {
    KEYBOARD,
    GAMEPAD
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

private:
    //The actual hardware texture
    SDL_Texture* mTexture;

    //Image dimensions
    int mWidth;
    int mHeight;
};

class Player
{
public:
    //Maximum axis velocity of the dot
    int PLAYER_VEL = 1;

    //Initializes the variables
    Player(std::string tex, KEYVARIANTS keyVar);

    //Frees the texture
    ~Player();

    //Takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    //Moves the dot
    void move();
    void setPosition(float x, float y);

    //Shows the dot on the screen relative to the camera
    void render();
    void render(int x, int y);

    //Position accessors
    float getPosX();
    float getPosY();
    int getPWidth();
    int getPHeight();

private:
    KEYVARIANTS keyVariant;

    LTexture texture;
    int pWidth, pHeight;

    //The X and Y offsets of the dot
    float mPosX, mPosY;

    //The velocity of the dot
    int mVelX, mVelY;
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int JOYSTICK_DEAD_ZONE = 8000;

SDL_Window* gWindow = nullptr;
LTexture camera_focus_1_texture;
LTexture camera_focus_2_texture;

std::string levelMap = "";
std::vector<LTexture> levelTextures;
int levelMapWidth = 0;
int levelMapHeight = 0;
int map_tile_size = 40;

float camera_x = 0.0f;
float camera_y = 0.0f;

SDL_Renderer* gRenderer = nullptr;
SDL_Joystick* gGameController = NULL;

bool init();
bool readLevelMap(std::string filename);
void loadLevelMap();
void levelMapDisplay(float init_x, float init_y, float single_tile_w, float single_tile_h);
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

            const Uint8* keys = SDL_GetKeyboardState(NULL);
            Player player1("../player1.png", KEYBOARD);
            Player player2("../player2.png", GAMEPAD);
            player2.setPosition(player1.getPosX() + 2 * player1.getPWidth(), player1.getPosY());
            int shared_camera_x = 0;
            int shared_camera_y = 0;
            int camera_mode = 0;

            readLevelMap("../level_map.txt");
            loadLevelMap();

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

                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_k && e.key.repeat == 0) {
                        map_tile_size++;
                    } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_l && e.key.repeat == 0) {
                        if ((map_tile_size - 1) * levelMapWidth > SCREEN_WIDTH && (map_tile_size - 1) * levelMapHeight > SCREEN_HEIGHT){
                            map_tile_size--;
                        } else {
                            //ceiling of int division in std::max() here:
                            map_tile_size = std::max((SCREEN_WIDTH + levelMapWidth - 1) / levelMapWidth, (SCREEN_HEIGHT + levelMapHeight - 1) / levelMapHeight);
                        }
                    }

                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_c && e.key.repeat == 0) {
                        camera_mode++;
                        if (camera_mode == 3) {camera_mode = 0;}
                    }
                }

                if (SDL_NumJoysticks() > 0) {
                    if (gGameController == NULL) {
                        gGameController = SDL_JoystickOpen(0);
                        player2.setPosition(player1.getPosX() + 2 * player1.getPWidth(), player1.getPosY());
                        camera_mode = 2;
                        std::cout << "Game controller detected!" << std::endl;
                    }
                } else {
                    camera_mode = 0;
                    gGameController = NULL;
                }

                //altering the position of players
                if (gGameController != NULL) {
                    if (camera_mode == 2) {
                        player1.move();
                        player2.move();
                        shared_camera_x = (((player1.getPosX() + player1.getPWidth() / 2.0f) - SCREEN_WIDTH / 2.0f) +
                                           (player2.getPosX() + player2.getPWidth() / 2.0f) - SCREEN_WIDTH / 2.0f) / 2;
                        shared_camera_y = (((player1.getPosY() + player1.getPHeight() / 2.0f) - SCREEN_HEIGHT / 2.0f) +
                                           (player2.getPosY() + player2.getPHeight() / 2.0f) - SCREEN_HEIGHT / 2.0f) /
                                          2;
                        camera_x = shared_camera_x;
                        camera_y = shared_camera_y;
                    } else if (camera_mode == 0) {
                        player1.move();
                        camera_x = (player1.getPosX() + player1.getPWidth() / 2.0f) - SCREEN_WIDTH / 2.0f;
                        camera_y = (player1.getPosY() + player1.getPHeight() / 2.0f) - SCREEN_HEIGHT / 2.0f;
                    } else if (camera_mode == 1) {
                        player2.move();
                        camera_x = (player2.getPosX() + player2.getPWidth() / 2.0f) - SCREEN_WIDTH / 2.0f;
                        camera_y = (player2.getPosY() + player2.getPHeight() / 2.0f) - SCREEN_HEIGHT / 2.0f;
                    }
                } else {
                    gGameController = NULL;
                    player1.move();
                    camera_x = (player1.getPosX() + player1.getPWidth() / 2.0f) - SCREEN_WIDTH / 2.0f;
                    camera_y = (player1.getPosY() + player1.getPHeight() / 2.0f) - SCREEN_HEIGHT / 2.0f;
                }

                //checking the camera movement
                if( camera_x < 0 ) { camera_x = 0; }
                if( camera_y < 0 ) { camera_y = 0; }
                if( camera_x > levelMapWidth * map_tile_size - SCREEN_WIDTH ) { camera_x = levelMapWidth * map_tile_size - SCREEN_WIDTH; }
                if( camera_y > levelMapHeight * map_tile_size - SCREEN_HEIGHT ) { camera_y = levelMapHeight * map_tile_size - SCREEN_HEIGHT; }

                //rendering level map based on previously calculated camera position
                levelMapDisplay(-camera_x, -camera_y, map_tile_size, map_tile_size);

                //checking if players are on screen
                if (player1.getPosX() < camera_x && player2.getPosX() + player1.getPWidth() > camera_x + SCREEN_WIDTH) {
                    if (keys[SDL_SCANCODE_LEFT]) {
                        player1.setPosition(camera_x, player1.getPosY());
                    }
                    if (SDL_JoystickGetAxis(gGameController, 0) > 8000) {
                        player2.setPosition(camera_x + SCREEN_WIDTH - player2.getPWidth(), player2.getPosY());
                    }
                }
                if (player1.getPosX() + player1.getPWidth() > camera_x + SCREEN_WIDTH && player2.getPosX() < camera_x) {
                    if (keys[SDL_SCANCODE_RIGHT]) {
                        player1.setPosition(camera_x + SCREEN_WIDTH - player1.getPWidth(), player1.getPosY());
                    }
                    if (SDL_JoystickGetAxis(gGameController, 0) < -8000) {
                        player2.setPosition(camera_x, player2.getPosY());
                    }
                }
                if (player1.getPosY() < camera_y && player2.getPosY() + player2.getPHeight() > camera_y + SCREEN_HEIGHT) {
                    if (keys[SDL_SCANCODE_UP]) {
                        player1.setPosition(player1.getPosX(), camera_y);
                    }
                    if (SDL_JoystickGetAxis(gGameController, 1) > 8000) {
                        player2.setPosition(player2.getPosX(), camera_y + SCREEN_HEIGHT - player2.getPHeight());
                    }
                }
                if (player1.getPosY() + player1.getPHeight() > camera_y + SCREEN_HEIGHT && player2.getPosY() < camera_y) {
                    if (keys[SDL_SCANCODE_DOWN]) {
                        player1.setPosition(player1.getPosX(), camera_y + SCREEN_HEIGHT - player1.getPHeight());
                    }
                    if (SDL_JoystickGetAxis(gGameController, 1) < -8000) {
                        player2.setPosition(player2.getPosX(), camera_y);
                    }
                }

                //rendering camera mode indicators
                if (gGameController == NULL) {
                    camera_focus_1_texture.render1(0, SCREEN_HEIGHT - camera_focus_1_texture.getHeight());
                } else {
                    if (camera_mode == 0) {
                        camera_focus_1_texture.render1(0, SCREEN_HEIGHT - camera_focus_1_texture.getHeight());
                    } else if (camera_mode == 1) {
                        camera_focus_2_texture.render1(camera_focus_1_texture.getWidth(), SCREEN_HEIGHT - camera_focus_1_texture.getHeight());
                    } else {
                        camera_focus_1_texture.render1(0, SCREEN_HEIGHT - camera_focus_1_texture.getHeight());
                        camera_focus_2_texture.render1(camera_focus_1_texture.getWidth(), SCREEN_HEIGHT - camera_focus_1_texture.getHeight());
                    }
                    player2.render(camera_x, camera_y);
                }
                player1.render(camera_x, camera_y);
//                std::cout << " player1(" << player1.getPosX() << ", " << player1.getPosY() << "), player2(" << player2.getPosX() << ", " << player2.getPosY() << ")" << std::endl;

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
        gWindow = SDL_CreateWindow("2D camera for two players", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(gRenderer, 0x44, 0x81, 0x88, 0xFF);

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
    if (!file.good()) {
        std::cout << "Wrong file " << filename << std::endl;
        return !success;
    }
    else {
        levelMap.clear();
        while (!file.eof()) {
            levelMapHeight++;
            std::getline(file, line);
            levelMap += line;
        }
        levelMapWidth = line.size();

        file.close();
        return success;
    }
}

void loadLevelMap() {
    levelTextures.resize(14);
    levelTextures[0].loadFromFile("../level/left_top_corner.png");
    levelTextures[1].loadFromFile("../level/top_edge.png");
    levelTextures[2].loadFromFile("../level/right_top_corner.png");
    levelTextures[3].loadFromFile("../level/left_edge.png");
    levelTextures[4].loadFromFile("../level/floor.png");
    levelTextures[5].loadFromFile("../level/right_edge.png");
    levelTextures[6].loadFromFile("../level/left_bottom_corner.png");
    levelTextures[7].loadFromFile("../level/bottom_edge.png");
    levelTextures[8].loadFromFile("../level/right_bottom_corner.png");

    levelTextures[9].loadFromFile( "../level/bottom_left_bit.png");
    levelTextures[10].loadFromFile("../level/bottom_right_bit.png");
    levelTextures[11].loadFromFile("../level/top_left_bit.png");
    levelTextures[12].loadFromFile("../level/top_right_bit.png");
    levelTextures[13].loadFromFile("../level/wall_fill.png");
}

void levelMapDisplay(float init_x, float init_y, float single_tile_w, float single_tile_h) {
    LTexture texture;
    int y = 0;
    for (int j = 0; j < levelMapHeight; j++) {
        for (int i = 0; i < levelMapWidth; i++) {
            switch (levelMap[j * levelMapWidth + i])
            {
                case '/':
                    levelTextures[0].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case '.':
                    levelTextures[1].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case '\\':
                    levelTextures[2].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case '[':
                    levelTextures[3].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case '-':
                    levelTextures[4].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case ']':
                    levelTextures[5].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case ':':
                    levelTextures[6].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case '_':
                    levelTextures[7].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case ';':
                    levelTextures[8].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case 'L':
                    levelTextures[9].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                             single_tile_h);
                    break;
                case 'J':
                    levelTextures[10].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                              single_tile_h);
                    break;
                case '\'':
                    levelTextures[11].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                              single_tile_h);
                    break;
                case '+':
                    levelTextures[12].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                              single_tile_h);
                    break;
                case '#':
                    levelTextures[13].render2(i * single_tile_w + init_x, y * single_tile_h + init_y, single_tile_w,
                                              single_tile_h);
                    break;
                default:
                    std::cout << std::endl;
                    break;
            }
        }
        y++;
    }
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;
    if (!camera_focus_1_texture.loadFromFile("../camerafocus1.png")) {
        printf("Failed to load camerafocus1 texture image!\n");
        success = false;
    }

    if (!camera_focus_2_texture.loadFromFile("../camerafocus2.png")) {
        printf("Failed to load camerafocus2 texture image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded images
    camera_focus_1_texture.free();

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

Player::Player(std::string tex, KEYVARIANTS keyVar)
{
    keyVariant = keyVar;

    texture.loadFromFile(tex);
    pWidth = texture.getWidth();
    pHeight = texture.getHeight();

    //Initialize the offsets
    mPosX = SCREEN_WIDTH / 2.0f - texture.getWidth() / 2.0f;
    mPosY = SCREEN_HEIGHT / 2.0f - texture.getHeight() / 2.0f;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
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

    if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_SPACE)) {
        mPosX = SCREEN_WIDTH / 2.0f - texture.getWidth() / 2.0f;
        mPosY = SCREEN_HEIGHT / 2.0f - texture.getHeight() / 2.0f;
    }
}

void Player::move()
{
    //Move the dot left or right
    mPosX += mVelX;

    //If the dot went too far to the left or right
    if ((mPosX < 0) || (mPosX + pWidth > levelMapWidth * map_tile_size))
//    if ((mPosX < 0) || (mPosX + pWidth > background.getWidth()))
    {
        //Move back
        mPosX -= mVelX;
    }

    //Move the dot up or down
    mPosY += mVelY;

    //If the dot went too far up or down
    if ((mPosY < 0) || (mPosY + pHeight > levelMapHeight * map_tile_size))
//    if ((mPosY < 0) || (mPosY + pHeight > background.getHeight()))
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
    texture.render1(mPosX - x, mPosY - y);
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
