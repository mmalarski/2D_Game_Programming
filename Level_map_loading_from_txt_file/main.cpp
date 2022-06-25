#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL_image.h>
#include <vector>

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
    void render(int x, int y);
    void render(int x, int y, int width, int height);

    //Gets image dimensions
    int getWidth();
    int getHeight();

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
    int PLAYER_VEL = 2;

    //Initializes the variables
    Player(std::string tex);

    //Frees the texture
    ~Player();

    //Takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    //Moves the dot
    void move();

    //Shows the dot on the screen relative to the camera
    void render();
    void render(int x, int y);

    //Position accessors
    float getPosX();
    float getPosY();
    int getPWidth() const;
    int getPHeight() const;

private:
    LTexture texture;
    int pWidth, pHeight;

    //The X and Y offsets of the dot
    float mPosX, mPosY;

    //The velocity of the dot
    int mVelX, mVelY;
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int CIRCLE_SIZE = 40;
const float CAMERA_SPEED = 0.5f;

SDL_Window* gWindow = nullptr;
LTexture gCurrentTexture;
LTexture circle;

std::string levelMap = "";
std::vector<LTexture> levelTextures;
int levelMapWidth = 0;
int levelMapHeight = 0;

SDL_Renderer* gRenderer = nullptr;

const Uint8* keys;
Sint32 m_xpos = SCREEN_WIDTH / 2 - 100;
Sint32 m_ypos = SCREEN_HEIGHT / 2 - 100;
bool movemouse = true;
int levelMapPos_X = 0;
int levelMapPos_Y = 0;

bool init();
bool readLevelMap(std::string filename);
void loadLevelMap();
void levelMapDisplay(float init_x, float init_y);
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
            keys = SDL_GetKeyboardState(NULL);
            Player player1("../press.png");
//            Player player2("player2.png");
            float camera_x = player1.getPosX() + player1.getPWidth() / 2.0f - SCREEN_WIDTH / 2.0f;
            float camera_y = player1.getPosY() + player1.getPHeight() / 2.0f - SCREEN_HEIGHT / 2.0f;

            int mouse_pressed_x = 0;
            int mouse_pressed_y = 0;
            int mouse_lifted_x = 0;
            int mouse_lifted_y = 0;
            int mouse_offset_x = 0;
            int mouse_offset_y = 0;

            readLevelMap("../level_map.txt");
            loadLevelMap();

            //While application is running
            while (!quit)
            {
                //Handle events on queue
                while (SDL_PollEvent(&e) != 0)
                {
                    //User requests quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }

                    player1.handleEvent(e);

                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        SDL_GetMouseState(&mouse_pressed_x, &mouse_pressed_y);
                        movemouse = false;
                    }
                    else if (e.type == SDL_MOUSEBUTTONUP) {
                        movemouse = true;
                        SDL_GetMouseState(&mouse_lifted_x, &mouse_lifted_y);
                        mouse_offset_x += mouse_lifted_x - mouse_pressed_x;
                        mouse_offset_y += mouse_lifted_y - mouse_pressed_y;
                    }
                    else if (e.type == SDL_MOUSEMOTION && movemouse) {
                        m_xpos = e.motion.x - mouse_offset_x;
                        m_ypos = e.motion.y - mouse_offset_y;
                    }

                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
                    {
                        SDL_GetMouseState(&m_xpos, &m_ypos);
                        mouse_offset_x = 0;
                        mouse_offset_y = 0;
                    }

                }

                //Clear screen
                SDL_RenderClear(gRenderer);
                levelMapDisplay(camera_x, camera_y);
                if(keys[SDL_SCANCODE_D]) {
                    player1.move();
                    player1.render();
                } else {
                    if (keys[SDL_SCANCODE_UP])
                    {
                        camera_y += 1;
                    }
                    if (keys[SDL_SCANCODE_DOWN])
                    {
                        camera_y -= 1;
                    }
                    if (keys[SDL_SCANCODE_LEFT])
                    {
                        camera_x += 1;
                    }
                    if (keys[SDL_SCANCODE_RIGHT])
                    {
                        camera_x -= 1;
                    }

                    if (camera_x < -std::abs(levelMapWidth * levelTextures[0].getWidth() - SCREEN_WIDTH)) {
                        camera_x = -std::abs(levelMapWidth * levelTextures[0].getWidth() - SCREEN_WIDTH);
                        if(keys[SDL_SCANCODE_D]){player1.move();}
                    }
                    if (camera_y < -std::abs(levelMapHeight * levelTextures[0].getHeight() - SCREEN_HEIGHT)) {
                        camera_y = -std::abs(levelMapHeight * levelTextures[0].getHeight() - SCREEN_HEIGHT);
                        if(keys[SDL_SCANCODE_D]){player1.move();}
                    }
                    if (camera_x > 0) {
                        camera_x = 0;
                        if(keys[SDL_SCANCODE_D]){player1.move();}
                    }
                    if (camera_y > 0) {
                        camera_y = 0;
                        if(keys[SDL_SCANCODE_D]){player1.move();}
                    }
                    player1.render();
                }

                circle.render(m_xpos - CIRCLE_SIZE / 2, m_ypos - CIRCLE_SIZE / 2, CIRCLE_SIZE, CIRCLE_SIZE);

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
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow("Level map loading from .txt file", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    levelTextures[9].loadFromFile("../level/bottom_left_bit.png");
    levelTextures[10].loadFromFile("../level/bottom_right_bit.png");
    levelTextures[11].loadFromFile("../level/top_left_bit.png");
    levelTextures[12].loadFromFile("../level/top_right_bit.png");
    levelTextures[13].loadFromFile("../level/wall_fill.png");
}

void levelMapDisplay(float init_x, float init_y) {
    LTexture texture;
    int y = 0;
    for (int j = 0; j < levelMapHeight; j++) {
        for (int i = 0; i < levelMapWidth; i++) {
            switch (levelMap[j * levelMapWidth + i])
            {
                case '/':
                    levelTextures[0].render(i * levelTextures[0].getWidth() + init_x, y * levelTextures[0].getHeight() + init_y);
                    break;
                case '.':
                    levelTextures[1].render(i * levelTextures[1].getWidth() + init_x, y * levelTextures[1].getHeight() + init_y);
                    break;
                case '\\':
                    levelTextures[2].render(i * levelTextures[2].getWidth() + init_x, y * levelTextures[2].getHeight() + init_y);
                    break;
                case '[':
                    levelTextures[3].render(i * levelTextures[3].getWidth() + init_x, y * levelTextures[3].getHeight() + init_y);
                    break;
                case '-':
                    levelTextures[4].render(i * levelTextures[4].getWidth() + init_x, y * levelTextures[4].getHeight() + init_y);
                    break;
                case ']':
                    levelTextures[5].render(i * levelTextures[5].getWidth() + init_x, y * levelTextures[5].getHeight() + init_y);
                    break;
                case ':':
                    levelTextures[6].render(i * levelTextures[6].getWidth() + init_x, y * levelTextures[6].getHeight() + init_y);
                    break;
                case '_':
                    levelTextures[7].render(i * levelTextures[7].getWidth() + init_x, y * levelTextures[7].getHeight() + init_y);
                    break;
                case ';':
                    levelTextures[8].render(i * levelTextures[8].getWidth() + init_x, y * levelTextures[8].getHeight() + init_y);
                    break;
                case 'L':
                    levelTextures[9].render(i * levelTextures[9].getWidth() + init_x, y * levelTextures[9].getHeight() + init_y);
                    break;
                case 'J':
                    levelTextures[10].render(i * levelTextures[10].getWidth() + init_x, y * levelTextures[10].getHeight() + init_y);
                    break;
                case '\'':
                    levelTextures[11].render(i * levelTextures[11].getWidth() + init_x, y * levelTextures[11].getHeight() + init_y);
                    break;
                case '+':
                    levelTextures[12].render(i * levelTextures[12].getWidth() + init_x, y * levelTextures[12].getHeight() + init_y);
                    break;
                case '#':
                    levelTextures[13].render(i * levelTextures[13].getWidth() + init_x, y * levelTextures[13].getHeight() + init_y);
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
    if (!circle.loadFromFile("../circle.png")) {
        printf("Failed to load Circle texture image!\n");
        success = false;
    }

    //Load default surface
    if (!gCurrentTexture.loadFromFile("../press.png")) {
        printf("Failed to load Press texture image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded images
    circle.free();

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

void LTexture::render(int x, int y)
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

void LTexture::render(int x, int y, int width, int height)
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, width, width };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

Player::Player(std::string tex)
{
    texture.loadFromFile(tex);
    pHeight = texture.getHeight();
    pWidth = texture.getWidth();

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
    //If a key was pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
    {
        //Adjust the velocity
        switch (e.key.keysym.sym)
        {
            case SDLK_UP: mVelY -= PLAYER_VEL; break;
            case SDLK_DOWN: mVelY += PLAYER_VEL; break;
            case SDLK_LEFT: mVelX -= PLAYER_VEL; break;
            case SDLK_RIGHT: mVelX += PLAYER_VEL; break;
        }
    }
        //If a key was released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0)
    {
        //Adjust the velocity
        switch (e.key.keysym.sym)
        {
            case SDLK_UP: mVelY += PLAYER_VEL; break;
            case SDLK_DOWN: mVelY -= PLAYER_VEL; break;
            case SDLK_LEFT: mVelX += PLAYER_VEL; break;
            case SDLK_RIGHT: mVelX -= PLAYER_VEL; break;
        }
    }

//    if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_d)) {
//        mPosX = SCREEN_WIDTH / 2.0f - texture.getWidth() / 2.0f;
//        mPosY = SCREEN_HEIGHT / 2.0f - texture.getHeight() / 2.0f;
//    }
}

void Player::move()
{
    //Move the dot left or right
    mPosX += mVelX;

    //If the dot went too far to the left or right
    if ((mPosX < 0) || (mPosX + pWidth > SCREEN_WIDTH))
    {
        //Move back
        mPosX -= mVelX;
    }

    //Move the dot up or down
    mPosY += mVelY;

    //If the dot went too far up or down
    if ((mPosY < 0) || (mPosY + pHeight > SCREEN_HEIGHT))
    {
        //Move back
        mPosY -= mVelY;
    }
}

void Player::render()
{
    texture.render(mPosX, mPosY);
}

void Player::render(int x, int y)
{
    texture.render(x, y);
}

float Player::getPosX()
{
    return mPosX;
}

float Player::getPosY()
{
    return mPosY;
}

int Player::getPWidth() const {
    return pWidth;
}

int Player::getPHeight() const {
    return pHeight;
}
