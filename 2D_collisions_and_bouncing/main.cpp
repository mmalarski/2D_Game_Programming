#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <SDL2/SDL_image.h>
#include <vector>
#include <random>
#include <ctime>

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
    void render1(int x, int y);

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

struct Circle {
    float xp, yp;
    LTexture texture;
    float vx;
    float vy;
    int id;

    Circle(float x, float y, int id);
    void render();
    void setV(float x_vel, float y_vel);
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const float TEXTURE_RADIUS = 20;
const float SPEED = 0.2;

SDL_Window* gWindow = nullptr;
LTexture bumpingTexture;
LTexture bumpingOFFTexture;
LTexture separationTexture;
LTexture separationOFFTexture;

SDL_Renderer* gRenderer = nullptr;
std::vector<Circle *> circlesVector;
std::vector<std::pair<Circle *, Circle*>> circlesColliding;

bool init();
bool loadMedia();
void close();bool isPointInCircle(int xp, int yp, int radius, int mouse_x, int mouse_y);
bool doCirclesOverlap(int xp1, int yp1, int xp2, int yp2, int radius2);

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
            Uint32 mouse = 0;
            int mouse_x = 0, mouse_y = 0;
            float distance = 0;
            float overlap = 0;
            Circle* b1 = nullptr;
            Circle* b2 = nullptr;
            Circle* activeCircle = NULL;
            std::srand(std::time(nullptr));
            bool separation = true;
            bool bumping = true;

            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 7; j++) {
                    circlesVector.push_back(new Circle(SCREEN_WIDTH / 8.0 + j * SCREEN_WIDTH / 8.0, SCREEN_HEIGHT / 6.0 + i * SCREEN_HEIGHT / 6.0, i * 7 + j));
                    circlesVector[circlesVector.size() - 1]->setV((rand() % 10) - 5, (rand() % 10) - 5);
                }
            }

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
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_1) {
                            separation = !separation;
                        }
                    }
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_2) {
                            bumping = !bumping;
                        }
                    }
                }

                mouse = SDL_GetMouseState(&mouse_x, &mouse_y);
                circlesColliding.clear();

                if (activeCircle == nullptr && mouse == 1) {
                    std::cout << "mouse = 1" << std::endl;
                    activeCircle = nullptr;
                    for (Circle* c : circlesVector) {
                        if (isPointInCircle(c->xp, c->yp, TEXTURE_RADIUS, mouse_x, mouse_y)){
                            activeCircle = c;
                            std::cout << c->id << std::endl;
                            break;
                        }
                    }

                } else if (activeCircle != nullptr && mouse == 1) {
                    activeCircle->xp = mouse_x;
                    activeCircle->yp = mouse_y;
                } else if (activeCircle != nullptr && mouse == 0) {
                    std::cout << activeCircle->xp << ", " << activeCircle->yp << std::endl;
                    activeCircle = nullptr;
                }

                if (separation) {
                    for (Circle* circle : circlesVector) {
                        for (Circle *target : circlesVector) {
                            if (circle->id != target->id) {
                                if (doCirclesOverlap(circle->xp, circle->yp, target->xp, target->yp, TEXTURE_RADIUS)) {
                                    circlesColliding.push_back({circle, target});
                                    distance = sqrtf((circle->xp - target->xp) * (circle->xp - target->xp) +
                                                     (circle->yp - target->yp) * (circle->yp - target->yp));
                                    overlap = 0.5 * (distance - TEXTURE_RADIUS - TEXTURE_RADIUS);
                                    circle->xp -= overlap * (circle->xp - target->xp) / distance;
                                    circle->yp -= overlap * (circle->yp - target->yp) / distance;

                                    target->xp += overlap * (circle->xp - target->xp) / distance;
                                    target->yp += overlap * (circle->yp - target->yp) / distance;
                                }
                            }
                        }
                    }
                }

                for (Circle* circle : circlesVector) {
                    circle->xp += circle->vx * SPEED;
                    circle->yp += circle->vy * SPEED;
                }

                if (separation) {
                    separationTexture.render1(SCREEN_WIDTH * 0.5 - separationTexture.getWidth() - 50, SCREEN_HEIGHT - separationTexture.getHeight() - 5);
                    if (bumping) {
                        bumpingTexture.render1(SCREEN_WIDTH * 0.5 + 50, SCREEN_HEIGHT - bumpingTexture.getHeight() - 5);
                        //dynamic collisions
                        for (std::pair<Circle*, Circle*> pair : circlesColliding) {
                            b1 = pair.first;
                            b2 = pair.second;

                            // distance
                            // Distance between balls
                            float fDistance = sqrtf((b1->xp - b2->xp) * (b1->xp - b2->xp) + (b1->yp - b2->yp) * (b1->yp - b2->yp));

                            // Normal
                            float nx = (b2->xp - b1->xp) / fDistance;
                            float ny = (b2->yp - b1->yp) / fDistance;

                            // Tangent
                            float tx = -ny;
                            float ty = nx;

                            // Dot Product Tangent
                            float dpTan1 = b1->vx * tx + b1->vy * ty;
                            float dpTan2 = b2->vx * tx + b2->vy * ty;

                            // Dot Product Normal
                            float dpNorm1 = b1->vx * nx + b1->vy * ny;
                            float dpNorm2 = b2->vx * nx + b2->vy * ny;

                            // Update ball velocities
                            b1->vx = b1->vx - 2 * dpNorm1 * nx;
                            b1->vy = b1->vy - 2 * dpNorm1 * ny;
                            b2->vx = b2->vx - 2 * dpNorm2 * nx;
                            b2->vy = b2->vy - 2 * dpNorm2 * ny;
                        }
                    } else {
                        bumpingOFFTexture.render1(SCREEN_WIDTH * 0.5 + 50, SCREEN_HEIGHT - bumpingOFFTexture.getHeight() - 5);
                    }
                } else {
                    separationOFFTexture.render1(SCREEN_WIDTH * 0.5 - separationOFFTexture.getWidth() - 50, SCREEN_HEIGHT - separationOFFTexture.getHeight() - 5);
                    bumpingOFFTexture.render1(SCREEN_WIDTH * 0.5 + 50, SCREEN_HEIGHT - bumpingOFFTexture.getHeight() - 5);
                }

                for (Circle* circle : circlesVector) {
                    if (circle->xp < TEXTURE_RADIUS) {
                        circle->xp = TEXTURE_RADIUS;
                        circle->vx = -circle->vx;
                    }
                    if (circle->xp > SCREEN_WIDTH - TEXTURE_RADIUS) {
                        circle->xp = SCREEN_WIDTH - TEXTURE_RADIUS;
                        circle->vx = -circle->vx;
                    }
                    if (circle->yp < TEXTURE_RADIUS) {
                        circle->yp = TEXTURE_RADIUS;
                        circle->vy = -circle->vy;
                    }
                    if (circle->yp > SCREEN_HEIGHT - TEXTURE_RADIUS) {
                        circle->yp = SCREEN_HEIGHT - TEXTURE_RADIUS;
                        circle->vy = -circle->vy;
                    }

                    circle->render();
                }

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
        gWindow = SDL_CreateWindow("2D Collisions and Bouncing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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


bool loadMedia()
{
    //Loading success flag
    bool success = true;
    if (!bumpingTexture.loadFromFile("../bumping.png")) {
        printf("Failed to load bumping texture image!\n");
        success = false;
    }

    if (!bumpingOFFTexture.loadFromFile("../bumpingOFF.png")) {
        printf("Failed to load bumpingOFF texture image!\n");
        success = false;
    }

    if (!separationTexture.loadFromFile("../separation.png")) {
        printf("Failed to load separation texture image!\n");
        success = false;
    }

    if (!separationOFFTexture.loadFromFile("../separationOFF.png")) {
        printf("Failed to load separationOFF texture image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded images
    bumpingTexture.free();
    bumpingOFFTexture.free();
    separationTexture.free();
    separationOFFTexture.free();

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
    //Set rendering space and render_basicScale_topLeft to screen
    SDL_Rect renderQuad = { (int)(x - TEXTURE_RADIUS), (int)(y - TEXTURE_RADIUS), (int) (2 * TEXTURE_RADIUS), (int)(2 * TEXTURE_RADIUS) };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

void LTexture::render1(int x, int y)
{
    //Set rendering space and render_basicScale_topLeft to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    SDL_RenderCopy(gRenderer, mTexture, NULL, &renderQuad);
}

int LTexture::getWidth() const {
    return mWidth;
}

int LTexture::getHeight() const {
    return mHeight;
}

Circle::Circle(float x, float y, int id) {
    xp = x;
    yp = y;
    texture.loadFromFile("../circle.png");
    this->id = id;

    vx = 0;
    vy = 0;
}

void Circle::render() {
    texture.render(xp, yp);
}

void Circle::setV(float x_vel, float y_vel) {
    float distance = sqrt((x_vel * x_vel) + (y_vel * y_vel));
    this->vx = x_vel / distance;
    this->vy = y_vel / distance;
}

bool isPointInCircle(int xp, int yp, int radius, int mouse_x, int mouse_y) {
    return ((xp - mouse_x) * (xp - mouse_x) + (yp - mouse_y) * (yp - mouse_y)) < (radius * radius);
}

bool doCirclesOverlap(int xp1, int yp1, int xp2, int yp2, int radius2) {
    return ((xp1 - xp2) * (xp1 - xp2) + (yp1 - yp2) * (yp1 - yp2)) <= ((radius2 + radius2) * (radius2 + radius2));
}