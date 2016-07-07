#include "includes.h"

//Lets set up some data types
struct segment {
    int x1;
    int y1;
    int x2;
    int y2;
};

struct obstical {
    std::string texture;
    std::vector<segment> death;
    std::vector<segment> life;
    int width;
};

enum gameState {TITLE, GAME, PAUSE, DEATH};
enum buttonState {BUTTON_OFF, BUTTON_ON};
enum jumpState {ON_GROUND, IN_AIR};

// Declare functions
SDL_Window* initSDL();
bool checkWindow(SDL_Window*);  // Checks if a window exists
SDL_Texture * loadImg(SDL_Renderer *ren, std::string);
std::map<std::string, SDL_Texture *> textures;
std::map<std::string, Mix_Chunk *> sounds;
std::map<std::string, Mix_Music *> music;
void scrollLayer(SDL_Renderer* ren, std::string tex, int speed, unsigned int& counter);
obstical obsticalLoad(std::string filename);
int loopint(int what, int howmuch, int howbig);
int rando(int start, int endnum);
segment offsetX(segment thing, int offset);
float intersects(segment one, segment two);
unsigned long int getHighScore();
void setHighScore(unsigned long int score);
void changeState(gameState & oldState, gameState newState);
std::string num2str(int num);
std::string num2str(unsigned long int num);
SDL_Rect makeRect(int x, int y, int w, int h);
SDL_Texture * wordTexture(SDL_Renderer *ren, TTF_Font * font, std::string thing, SDL_Color color);
int getTextureW(SDL_Texture* tex);
int getTextureH(SDL_Texture* tex);

// GLOBALS

const int groundLevel = 415;
const int playerForward = 50;
const int FPS = 60;
unsigned long score = 0;
float jumpVel = 15;
float gravity = 50;

/*-------------------------------------*/
/* main                                */
/*-------------------------------------*/
int main(int argc, char *argv[]) {
    // Init stuff & hose cleaning
    SDL_Window *window = initSDL();
    SDL_Event e;
    // Initialize random number generation
    srand (time(NULL));

    // Create renderer
    SDL_Renderer *ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if ( ren == NULL) {
        //std::cout << "SO SAD!!!\t" << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    if (!checkWindow(window))
        return 1;

    // Initialize font
    if (TTF_Init() == -1) {
        //printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }
    TTF_Font *scoreFont;
    TTF_Font *titleFont;
    TTF_Font *huge;
    titleFont = TTF_OpenFont("./res/fonts/3Dumb.ttf", 60);
    scoreFont = TTF_OpenFont("./res/fonts/3Dumb.ttf", 48);
    huge = TTF_OpenFont("./res/fonts/3Dumb.ttf", 64);
    if( !scoreFont ) {
        //printf("TTF_OpenFontIndex: %s\n", TTF_GetError());
    }
    int flags = 0;
    int initted = Mix_Init(flags);
    if ((initted&flags) != flags) {
        //std::cout << "Mix_Init: Failed to init required support\n";
        //std::cout << "Mix_init: " << Mix_GetError() << std::endl;
    }

    if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        //std::cout << "Mixer init: failed to initialize mixer" << std::endl;
    }



    // Game stuff now
    bool running = true;
    unsigned long int score = 0;SDL_SetRenderDrawColor(ren, 0, 0, 0, 100);
                SDL_Rect box = makeRect(640 / 20, 480 / 4, 640 / 20 * 18, 480 / 1.75);
                SDL_RenderFillRect(ren, &box);
    SDL_Rect  scorePos;
    float playerVel = 0.0f;
    float playerY = 0.0f;
    buttonState currentButtonState = BUTTON_OFF;
    jumpState currentJumpState = IN_AIR;
    float volume = 0.5f;
    std::vector<obstical> obsticals;
    int obspeed = 10;
    int obsticalPos = 0;
    int obsticalArray[] = {0,0,0,0,0};
    int obsticalPosA[] = {0, 680, 1360, 2040, 2720};
    gameState stage = TITLE;
    gameState buffer = TITLE;
    int flashsize = 0;

    // Load In all of our textures
    textures.insert(std::make_pair("ground", loadImg(ren, "./res/reg/ground2.bmp")));
    textures.insert(std::make_pair("mountains", loadImg(ren, "./res/reg/mountainss.bmp")));
    textures.insert(std::make_pair("mnt1", loadImg(ren, "./res/reg/mnt1.bmp")));
    textures.insert(std::make_pair("mnt2", loadImg(ren, "./res/reg/mnt2.bmp")));
    textures.insert(std::make_pair("mnt3", loadImg(ren, "./res/reg/mnt3.bmp")));
    textures.insert(std::make_pair("obj1", loadImg(ren, "./res/reg/obj1.bmp")));
    textures.insert(std::make_pair("obj3", loadImg(ren, "./res/reg/obj3.bmp")));
    textures.insert(std::make_pair("mountains2", loadImg(ren, "./res/reg/mountains2.bmp")));
    textures.insert(std::make_pair("sky", loadImg(ren, "./res/reg/sky.bmp")));
    textures.insert(std::make_pair("clouds", loadImg(ren, "./res/reg/clouds.bmp")));
    textures.insert(std::make_pair("player", loadImg(ren, "./res/reg/char.bmp")));
    textures.insert(std::make_pair("title", loadImg(ren, "./res/reg/title.bmp")));

    // Load our obsticals data
    obsticals.push_back(obsticalLoad("./res/maps/ob2.txt"));
    obsticals.push_back(obsticalLoad("./res/maps/ob1.txt"));
    obsticals.push_back(obsticalLoad("./res/maps/ob3.txt"));

    //sounds.insert(std::make_pair("jump", Mix_LoadWAV("./res/sounds/regular_jump.wav")));
    Mix_Chunk * jumpS = NULL;
    jumpS = Mix_LoadWAV("./res/sounds/regular_jumpr.wav");
    if( jumpS == NULL) {
        //std::cout << "Problem: " << Mix_GetError() << std::endl;
    }
    music.insert(std::make_pair("appler", Mix_LoadMUS("./res/music/appler_track_1.wav")));
    Mix_Volume(-1, MIX_MAX_VOLUME * volume);
    Mix_VolumeMusic(MIX_MAX_VOLUME/2.5f);

    //Mix_FadeInMusic(music["appler"], -1, 2000);
    int musicStart = 0;

    unsigned int scrollPos = 0;
    unsigned int m2pos = 0;
    unsigned int m3pos = 0;
    unsigned int cloudPos = 0;

    // Set FPS Vars
    unsigned long int currentTick = SDL_GetTicks();
    unsigned long int target = currentTick + 1000 / FPS;

    // Setting this to true will restart the game
    //bool titlescreen = true;


    // What happens
    while(running) {
        // Check if we are within FPS range
        currentTick = SDL_GetTicks();
        if (currentTick >= target) {
            target = currentTick + 1000/FPS;

//-------------------------------------------//SDL_Rect setRect(SDL_Rect & rect, int x, int y, int w, int h)
// HANDLE GAMESTATE SWITCHES                 //
//-------------------------------------------//
            // if true, restart game
            switch (stage) {
            case DEATH:
            case TITLE:
                for (int i = 0; i < 5; i++)
                    obsticalArray[i] = 0;
                playerVel = 0.0f;
                playerY = 0.0f;

                if (musicStart == 0) {
                    Mix_HaltMusic();
                    Mix_FadeInMusic(music["appler"], -1, 5000);
                    musicStart = 1;
                }
            default:;

            }

//------------------------------------------//
// GET INPUT EVENTS                         //
//------------------------------------------//
            // Get physical events
            while(SDL_PollEvent(&e) != 0) {
                // Quit is pressed
                if (e.type == SDL_QUIT) {
                    running = false;
                }

                if (e.type == SDL_KEYDOWN) {
                    SDL_Keycode key = e.key.keysym.sym;
                    if (stage == DEATH)
                        changeState(stage, TITLE);
                    else if (stage != GAME && stage != PAUSE)
                        changeState(stage, GAME);
                    if( key == SDLK_SPACE) {
                        currentButtonState = BUTTON_ON;

                    }

                    if (key == SDLK_ESCAPE) {
                        if (stage == PAUSE) {
                            changeState(stage, buffer);
                        }
                        else if (stage != PAUSE) {
                            buffer = stage;
                            changeState(stage, PAUSE);

                        }

                    }
                }

                if (e.type == SDL_KEYUP) {
                    SDL_Keycode key = e.key.keysym.sym;

                    if( key == SDLK_SPACE) {
                        currentButtonState = BUTTON_OFF;
                    }
                }

                if (e.type == SDL_CONTROLLERDEVICEADDED) {
                    SDL_GameController *pad = SDL_GameControllerOpen(int(e.cdevice.which));
                    if (pad) {
                        SDL_Joystick *joy = SDL_GameControllerGetJoystick( pad);
                        int instanceID = SDL_JoystickInstanceID(joy);
                        if(instanceID);

                    } else {
                        //std::cout << "uhh-ohh" << std::endl;
                    }
                }
                if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                    if (stage == DEATH)
                        changeState(stage, TITLE);
                    if (stage != GAME && stage != PAUSE)
                        changeState(stage, GAME);
                    currentButtonState = BUTTON_ON;
                    //std::cout << "BUTTON DOWN, I repeat... BUTTON DOWN!!!" << std::endl;
                }
                if (e.type == SDL_CONTROLLERBUTTONUP) {
                    currentButtonState = BUTTON_OFF;
                }
            }


//-------------------------------------------//
// EVENT HANDLING                            //
//-------------------------------------------//
            // Enter game logic
            // Jump!!!
            if (stage == GAME) {
                if (currentButtonState == BUTTON_ON && currentJumpState == ON_GROUND) {
                    playerVel = jumpVel;
                    playerY += 0.001f;
                    currentJumpState = IN_AIR;
                    float jumpVol = .2;
                    int chan = Mix_PlayChannel(-1, jumpS, 0);
                    Mix_Volume(chan, MIX_MAX_VOLUME * (volume * jumpVol));
                } else {
                    // Reverse jump!!!
                    playerVel -= gravity * ((1000 / (float)FPS) /1000 );
                }
            }
//----------------------------------------------//
// COLLISION DECTECTION                         //
//----------------------------------------------//
            if (stage == GAME) {
                // For the next two obsticals, get the number of ground segments and kill segments
                int numoflife  = obsticals[obsticalArray[obsticalPos]].life.size();
                int numofDeath = obsticals[obsticalArray[obsticalPos]].death.size();
                int numoflife2 = obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life.size();
                int numofDeath2= obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].death.size();

                // Assume we are in the air so we fall if necessary
                currentJumpState = IN_AIR;

                // Loop through all the ground segments and see if we are touching any, if we are set currentJumpState to ON_GROUND
                // and exit the ltexoop
                for(int i = 0; i < numoflife; i++) {
                    if (obsticals[obsticalArray[obsticalPos]].life[i].x1 + obsticalPosA[obsticalPos] <= playerForward + 50 &&
                        obsticals[obsticalArray[obsticalPos]].life[i].x2 + obsticalPosA[obsticalPos] >= playerForward ) {
                        if (playerY == (float)groundLevel- obsticals[obsticalArray[obsticalPos]].life[i].y1) {
                            playerY = groundLevel - obsticals[obsticalArray[obsticalPos]].life[i].y1;
                            currentJumpState = ON_GROUND;
                            playerVel = 0;
                            break;
                        } else if ((playerY  > groundLevel - obsticals[obsticalArray[obsticalPos]].life[i].y1 ) ) {
                                if (playerY + playerVel <= groundLevel - obsticals[obsticalArray[obsticalPos]].life[i].y1 ) {
                                    playerY = groundLevel - obsticals[obsticalArray[obsticalPos]].life[i].y1;
                                    playerVel = 0;
                                    currentJumpState = ON_GROUND;
                                    break;
                                }
                            }
                    }
                }
                // If we haven't touched ground on the previous obstical, check the next one
                if (currentJumpState == IN_AIR) {
                    for(int i = 0; i < numoflife2; i++) {
                        if (obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].x1 + obsticalPosA[loopint(obsticalPos, 1, 5)] <= playerForward + 50 &&
                        obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].x2 + obsticalPosA[loopint(obsticalPos, 1, 5)] >= playerForward ) {
                            if (playerY == (float)groundLevel - obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].y1) {
                                playerY = groundLevel - obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].y1;
                                currentJumpState = ON_GROUND;
                                playerVel = 0;
                                break;
                            } else if ((playerY > groundLevel - obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].y1) ) {
                                if (playerY + playerVel <= groundLevel - obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].y1) {
                                    playerY = groundLevel - obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].life[i].y1;
                                    playerVel = 0;
                                    currentJumpState = ON_GROUND;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Lol if we are still not touching any ground, decrease velocity or fall
                if (currentJumpState == IN_AIR) {
                    playerY += playerVel;
                }


                // Collision detection for death segments
                segment playerTop;
                segment playerBot;
                segment playerLef;
                segment playerRig;

                // Set all of the vars ever
                playerTop.x1 = playerForward;
                playerBot.x1 = playerForward;
                playerLef.x1 = playerForward;
                playerRig.x1 = playerForward + 50;
                playerTop.y1 = groundLevel - 50 - playerY;
                playerBot.y1 = groundLevel - playerY;
                playerLef.y1 = groundLevel - 50 - playerY;
                playerRig.y1 = groundLevel - 50 - playerY;
                playerTop.x2 = playerForward + 50;
                playerBot.x2 = playerForward + 50;
                playerLef.x2 = playerForward + 1;
                playerRig.x2 = playerForward + 49;
                playerTop.y2 = groundLevel - playerY - 50;
                playerBot.y2 = groundLevel - playerY;
                playerLef.y2 = groundLevel - playerY;
                playerRig.y2 = groundLevel - playerY;

                for(int i = 0; i < numofDeath; i++) {

                    segment deathseg = offsetX(obsticals[obsticalArray[obsticalPos]].death[i], obsticalPosA[obsticalPos] );
                    float top = intersects(playerTop, deathseg);
                    float bot = intersects(playerBot, deathseg);
                    float lef =  intersects(playerLef, deathseg);
                    float rig = intersects(playerRig, deathseg);

                    if ( top || bot || lef || rig){
                        changeState(stage, DEATH);
                        musicStart = 0;
                    }

                }

                for(int i = 0; i < numofDeath2; i++) {
                    // @TODO make these if cases like the ones above
                    float top = intersects(playerTop, obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].death[i]);
                    float bot = intersects(playerBot, obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].death[i]);
                    float lef = intersects(playerLef, obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].death[i]);
                    float rig = intersects(playerRig, obsticals[obsticalArray[loopint(obsticalPos, 1, 5)]].death[i]);
                    if ( top || bot || lef || rig ) {
                        changeState(stage, DEATH);
                        musicStart = 0;
                    }

                }
                // End collision detection
            }

//--------------------------------------------//
// RENDER                                     //
//--------------------------------------------//
            SDL_RenderClear(ren);
            SDL_RenderCopy(ren, textures["sky"], NULL, NULL);
            if (stage != PAUSE) {
                // Each layer has it's own scroll speed and position, function just moves them and displays them
                scrollLayer(ren, "clouds", 1, cloudPos);
                scrollLayer(ren, "mnt1", 2, m2pos);
                scrollLayer(ren, "mnt2", 3, scrollPos);
                scrollLayer(ren, "mnt3", 5, m3pos);
            } else {
                scrollLayer(ren, "clouds", 0, cloudPos);
                scrollLayer(ren, "mnt1", 0, m2pos);
                scrollLayer(ren, "mnt2", 0, scrollPos);
                scrollLayer(ren, "mnt3", 0, m3pos);
            }
            // Move and change obstical things for each one and display them
            for (int i = 0; i < 5; i++) {
                if (stage != PAUSE) {
                    obsticalPosA[i] -= obspeed;
                    if (obsticalPosA[i] <= 0 - obsticals[obsticalArray[i]].width) {
                        // Move position i to last x coordinate
                        obsticalPosA[i] = obsticalPosA[loopint(i, 4, 5)] + obsticals[obsticalArray[loopint(i, 4, 5)]].width;
                        if (stage != GAME && stage != PAUSE)
                            obsticalArray[i] = 0;
                        else
                            // Change it's type
                            obsticalArray[i] = rando(0, obsticals.size());

                        obsticalPos = loopint(i,1,5);


                    }
                }

                SDL_Rect obpos;
                obpos.x = obsticalPosA[i];
                obpos.y = 0;
                obpos.w = 690;
                obpos.h = 480;
                SDL_RenderCopy(ren, textures[obsticals[obsticalArray[i]].texture], NULL, &obpos);

            }

            switch(stage) {
            case GAME: {
                score++;
                SDL_Color color= {0,0,0};
                SDL_Surface *scoreMsg;
                std::string scoreString = "SCORE: ";
                std::string scoreNumString = num2str(score);
                scoreString.append(scoreNumString);
                scoreMsg = TTF_RenderText_Blended(scoreFont, scoreString.c_str(), color);
                SDL_Texture* Message = NULL;
                Message = SDL_CreateTextureFromSurface(ren, scoreMsg);
                scorePos.x = 0;
                scorePos.y = 0;
                scorePos.w = scoreMsg->w;
                scorePos.h = scoreMsg->h;
                SDL_RenderCopy(ren, Message, NULL, &scorePos);
                scoreMsg = NULL;
                SDL_FreeSurface(scoreMsg);
                break;
            }
            case DEATH: {
                unsigned long int highscore = getHighScore();
                if (highscore < score) {
                    setHighScore(score);
                    highscore = score;
                }
                SDL_Texture * ded;
                SDL_Texture * playerScore;
                SDL_Texture * highscoreMessage;
                SDL_Color color= {255,255,255};
                SDL_Color red = {255,0,0};
                std::string playerScoreStr = "Your Score: ";
                std::string scoreString = "High Score: ";
                std::string dedstr = "DED";
                playerScoreStr.append(num2str(score));
                scoreString.append(num2str(highscore));

                SDL_SetRenderDrawColor(ren, 0, 0, 0, 100);
                SDL_Rect box = makeRect(640 / 20, 480 / 4, 640 / 20 * 18, 480 / 1.75);
                SDL_RenderFillRect(ren, &box);

                ded = wordTexture(ren, titleFont, dedstr.c_str(), red);
                scorePos = makeRect(640/2 - (getTextureW(ded)/2), 480 / 3, getTextureW(ded), getTextureH(ded));
                SDL_RenderCopy(ren, ded, NULL, &scorePos);

                playerScore = wordTexture(ren, scoreFont, playerScoreStr.c_str(), color);
                scorePos = makeRect( (640 / 2 - (getTextureW(playerScore) /2)), 480 / 2, getTextureW(playerScore), getTextureH(playerScore));
                SDL_RenderCopy(ren, playerScore, NULL, &scorePos);

                highscoreMessage = wordTexture(ren, scoreFont, scoreString.c_str(), color);
                scorePos = makeRect( (640 / 2 - (getTextureW(highscoreMessage) / 2)) , (480 / 3 * 2) , getTextureW(highscoreMessage), getTextureH(highscoreMessage));

                SDL_RenderCopy(ren, highscoreMessage, NULL, &scorePos);
                SDL_DestroyTexture(ded);
                SDL_DestroyTexture(playerScore);
                SDL_DestroyTexture(highscoreMessage);


                break;
            }
            case TITLE: {
                unsigned long int highscore = getHighScore();

                SDL_Color color= {0,0,0};
                SDL_Surface *scoreMsg;
                std::string scoreString = "High Score: ";
                std::string scoreNumString = num2str(highscore);
                scoreString.append(scoreNumString);
                scoreMsg = TTF_RenderText_Blended(scoreFont, scoreString.c_str(), color);
                SDL_Texture* Message = NULL;
                Message = SDL_CreateTextureFromSurface(ren, scoreMsg);
                scorePos.x = 0;
                scorePos.y = 0;
                scorePos.w = scoreMsg->w;
                scorePos.h = scoreMsg->h;
                SDL_RenderCopy(ren, Message, NULL, &scorePos);
                SDL_FreeSurface(scoreMsg);
                SDL_DestroyTexture(Message);
                score = 0;


                SDL_Texture * ded;
                SDL_Color red = {255,0,0};
                SDL_Color white = {255,255,255};
                std::string dedstr = "A.P.P.L.E.R.";

                SDL_SetRenderDrawColor(ren, 0, 0, 0, 100);
                SDL_Rect box = makeRect(640 / 6, 480 / 4, 640 / 6 * 4, 480 / 3.30);
                SDL_RenderFillRect(ren, &box);

                ded = wordTexture(ren, huge, dedstr.c_str(), red);
                scorePos = makeRect(640/2 - (getTextureW(ded)/2), 480 / 3, getTextureW(ded), getTextureH(ded));
                SDL_RenderCopy(ren, ded, NULL, &scorePos);
                SDL_DestroyTexture(ded);

                dedstr = "With Micro-Transactions!!!";
                ded = wordTexture(ren, scoreFont, dedstr.c_str(), white);
                float flashx = 640/7;
                float flashy = 480 / 3 * 2 ;
                float flashw = getTextureW(ded)/4 *3;
                float flashh = getTextureH(ded)/4 * 3;
                if(SDL_GetTicks() % 1000 > 500 ) {
                    flashsize = (flashsize <= 0 ) ? 0 : flashsize - 1;
                } else {
                    flashsize = flashsize ++;
                }
                scorePos = makeRect(flashx + flashsize *flashsize /4 , flashy + flashsize, flashw - flashsize*flashsize /2, flashh- flashsize);
                SDL_RenderCopyEx(ren, ded, NULL, &scorePos, -10, NULL, SDL_FLIP_NONE);
                SDL_DestroyTexture(ded);

                //SDL_RenderCopy(ren, textures["title"],NULL,NULL);
                break;
            }
            case PAUSE: {
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 100);
                SDL_Rect box = makeRect(640 / 20, 480 / 4, 640 / 20 * 18, 480 / 1.75);
                SDL_RenderFillRect(ren, &box);
                break;
            }
            }

            if (stage != DEATH) {
                // Create the box for where our player is
                SDL_Rect playerRec;
                playerRec.x = playerForward;
                playerRec.y = (int)groundLevel - 50 - playerY;
                playerRec.w = playerRec.h = 50;
                SDL_RenderCopy(ren, textures["player"], NULL, &playerRec);
            }




            SDL_RenderPresent(ren);

        }
    }

    // The end
    TTF_CloseFont(scoreFont);
    TTF_Quit();
    Mix_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


/*-------------------------------------*/
/* Functions                           */
/*-------------------------------------*/
SDL_Window* initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window;
    window = SDL_CreateWindow(
                              "A.P.P.L.E.R. Another Painfully Pathetically Lame Endless Runner",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640,
                              480,
                              SDL_WINDOW_OPENGL);
    return window;
}

bool checkWindow(SDL_Window* window) {
    if(window == NULL) {
        //printf("Could not create window: %s\n", SDL_GetError());
        return false;
    } else
        return true;
}


SDL_Texture * loadImg(SDL_Renderer *ren, std::string imagePath) {
    SDL_Surface *bmp = SDL_LoadBMP(imagePath.c_str());
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, 0, 0xFF, 0xFF));
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_FreeSurface(bmp);

    return tex;
}


void scrollLayer(SDL_Renderer* ren, std::string tex, int speed, unsigned int& counter) {
    SDL_Rect block1;
    SDL_Rect block2;
    counter -= speed;
    block1.x = counter % 680;
    block2.x = block1.x - 680;
    block1.y = block2.y = 0;
    block1.w = block2.w = 680;
    block1.h = block2.h = 480;
    // This needs to error if texture does not exist
    SDL_RenderCopy(ren, textures[tex], NULL, &block1);
    SDL_RenderCopy(ren, textures[tex], NULL, &block2);
}

obstical obsticalLoad(std::string filename) {

    std::ifstream in(filename.c_str());
    int numOfDeath;
    int numOfLife;
    int x1, y1, x2, y2;
    obstical tmpObstical;
    segment death;
    segment life;
    if (in.is_open()){
        //read line for filename
        in >> tmpObstical.texture;
        //std::cout << texkey << std::endl;

        //read line for width
        in >> tmpObstical.width;

        //read line for # of death
        in >> numOfDeath;

        //read death lines for death
        for (int i = 0; i < numOfDeath; i++) {
            in >> x1 >> y1 >> x2 >> y2;
            death.x1 = x1;
            death.y1 = y1;
            death.x2 = x2;
            death.y2 = y2;
            tmpObstical.death.push_back(death);
        }

        //read line for # of life
        in >> numOfLife;

        //read life lines for life
        for (int i = 0; i < numOfLife; i++) {
            in >> x1 >> y1 >> x2 >> y2;
            life.x1 = x1;
            life.y1 = y1;
            life.x2 = x2;
            life.y2 = y2;
            tmpObstical.life.push_back(life);
        }
        in.close();
        return tmpObstical;
    } else {
        tmpObstical.texture = "";
        tmpObstical.width = 0;
        return tmpObstical;
    }
}

int loopint(int what, int howmuch, int howbig) {
        return ((what+howmuch) % howbig);

}


int rando(int start, int endnum) {
    return (rand() % (endnum - start)) + start;
}

segment offsetX(segment thing, int offset) {
    thing.x1 += offset;
    thing.x2 += offset;
    return thing;
}
float intersects(segment one, segment two) {
    float m1 = (one.x2 - one.x1 == 0 || one.y2 - one.y1 == 0) ? 0 : (one.y2 - one.y1) / (one.x2 - one.x1);
    float m2 = (two.x2 - two.x1 == 0 || two.y2 - two.y1 == 0) ? 0 : (two.y2 - two.y1) / (two.x2 - two.x1);

    float intersect = ((m2 * two.x1) - two.y1 - (m1 * one.x1) + one.y1) / (m2 - m1);
    //std::cout << intersect << std::endl;
    if (((one.x1 >= intersect && one.x2 <= intersect) || (one.x2 >= intersect && one.x1 <= intersect)) &&
        ((two.x1 >= intersect && two.x2 <= intersect) || (two.x2 >= intersect && two.x1 <= intersect))) {
        return intersect;
    }
    else
        return 0;
}

unsigned long int getHighScore() {
	std::ifstream in("./res/score");
	unsigned long int highscore;
	if (in.is_open()) {
		in >> highscore;
		return highscore;
	}
	else {
		return 0;
	}
}

void setHighScore(unsigned long int score) {
	std::ofstream scorefile("./res/score");
	if (scorefile.is_open()) {
		std::ostringstream convert;
		convert << score;
		scorefile <<  convert.str();
	}

}

void changeState(gameState& oldState,gameState newState) {
    oldState = newState;
}

std::string num2str(int num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}

std::string num2str(unsigned long int num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}

SDL_Rect makeRect(int x, int y, int w, int h) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    return rect;
}

SDL_Texture * wordTexture(SDL_Renderer* ren, TTF_Font * font, std::string thing, SDL_Color color) {
    SDL_Surface *tmp = NULL;
    SDL_Texture * message = NULL;
    tmp = TTF_RenderText_Blended(font, thing.c_str(), color);
    message = SDL_CreateTextureFromSurface(ren, tmp);
    SDL_FreeSurface(tmp);
    return message;
}

int getTextureW(SDL_Texture* tex) {
    int w,h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    return w;
}

int getTextureH(SDL_Texture* tex) {
    int w,h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    return h;
}
