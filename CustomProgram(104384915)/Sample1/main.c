#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_ENEMIES 25
#define MAX_BULLETS 20
#define MAX_ENEMY_BULLETS 100
#define MAX_STARS 100

typedef enum{
    EASY,
    MEDIUM,
    HARD
}  Difficulty;

typedef enum {
    SHIP_IDLE = 0,
    SHIP_RIGHT = 1,
    SHIP_LEFT = 2,
} ShipAnimationState;

typedef struct {
    Texture2D textures[4];
    Texture2D leftTexture;
    Texture2D rightTexture;  // Array to hold 4 different ship textures
    int currentFrame;       // Current texture being displayed
    int framesCounter;      // Counter for timing
    int framesSpeed;        // Speed of animation
    ShipAnimationState state;
} AnimatedSprite;

typedef struct {
    Vector2 position;
    float speed;
    float radius;
    float brightness;
} Star;

// Add animation structure for enemies
typedef struct {
    Texture2D textures[4];  // 4 frames of animation
    int currentFrame;
    int framesCounter;
    int framesSpeed;
} EnemySprite;

void DrawSpaceship(Rectangle rec, Color mainColor);
void InitSpaceshipSprite(AnimatedSprite *sprite);
void DrawAnimatedSpaceship(Rectangle destRec, AnimatedSprite *sprite);


typedef struct {
    int stageIndex;
    Rectangle rec;
    int speed;
    float fireRate;
    float HP;
    float maxHP; 
    float shield;
    float maxShield;
    float maxEnergy;
    float energy;
    float energyRechargeRate;
    float energyCost;
    bool canUseSpecialAttack;
    float stamina;
    int staminaReg;
    Difficulty difficulty;
    AnimatedSprite sprite;
} Player;

Player InitializePlayer();

typedef struct 
{
    Rectangle rec;
    bool active;
    int speed;
    float fireRate;
    float hp;
    int direction;
    float minX;
    float maxX;
    EnemySprite sprite;
} Enemy;


typedef enum GameState{
    MENU,
    DIFFICULTY,
    GAMEPLAY,
    SELECTBUFF,
    GAMEOVER,
    VICTORY,
} GameState;

typedef struct Stage
{
    int enemyCount;
    int eliteEnemyCount;
    int bossCount;
    float hpMultiplier;
    float dmgMultiplier;
    float spdMultiplier;
    float fireRateMultiplier;
} Stage;

// Define a structure to hold all stage configurations
typedef struct GameStages {
    Stage easy[4];      // Size matches your current easy stages
    Stage medium[6];    // Size matches your current medium stages
    Stage hard[7];      // Size matches your current hard stages
    int easyCount;      // Number of easy stages
    int mediumCount;    // Number of medium stages
    int hardCount;      // Number of hard stages
} GameStages;

GameStages InitializeStages() {
    GameStages stages = {0};
    FILE *file = fopen("resources/stages.txt", "r");
    if (!file) {
        printf("Error: Could not open stages.txt\n");
        return stages;
    }

    // Read easy stages
    fscanf(file, "EASY %d\n", &stages.easyCount);
    for (int i = 0; i < stages.easyCount; i++) {
        fscanf(file, "%d %d %d %f %f %f %f\n",
            &stages.easy[i].enemyCount,
            &stages.easy[i].eliteEnemyCount,
            &stages.easy[i].bossCount,
            &stages.easy[i].hpMultiplier,
            &stages.easy[i].dmgMultiplier,
            &stages.easy[i].spdMultiplier,
            &stages.easy[i].fireRateMultiplier);
    }

    // Read medium stages
    fscanf(file, "MEDIUM %d\n", &stages.mediumCount);
    for (int i = 0; i < stages.mediumCount; i++) {
        fscanf(file, "%d %d %d %f %f %f %f\n",
            &stages.medium[i].enemyCount,
            &stages.medium[i].eliteEnemyCount,
            &stages.medium[i].bossCount,
            &stages.medium[i].hpMultiplier,
            &stages.medium[i].dmgMultiplier,
            &stages.medium[i].spdMultiplier,
            &stages.medium[i].fireRateMultiplier);
    }

    // Read hard stages
    fscanf(file, "HARD %d\n", &stages.hardCount);
    for (int i = 0; i < stages.hardCount; i++) {
        fscanf(file, "%d %d %d %f %f %f %f\n",
            &stages.hard[i].enemyCount,
            &stages.hard[i].eliteEnemyCount,
            &stages.hard[i].bossCount,
            &stages.hard[i].hpMultiplier,
            &stages.hard[i].dmgMultiplier,
            &stages.hard[i].spdMultiplier,
            &stages.hard[i].fireRateMultiplier);
    }

    fclose(file);
    return stages;
}


typedef struct Bullet{
    Rectangle rec;
    bool active;
    float spd;
    float dmg;
    Texture2D texture;
} Bullet;


typedef struct GameContext {
    GameStages stages;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Enemy eliteEnemies[MAX_ENEMIES];
    Enemy Boss[MAX_ENEMIES];
    Bullet bullets[MAX_BULLETS];
    Bullet spBullets[MAX_BULLETS];
    Bullet enemyBullets[MAX_ENEMY_BULLETS];
    Bullet eliteEnemyBullets[MAX_ENEMY_BULLETS];
    float maxStageHealth;  // Add this new field
    double gameStartTime;
    bool isGameTimerRunning;
    double elapsedTime;
    Star stars[MAX_STARS];
} GameContext;

GameContext InitializeGameContext() {
    GameContext context = {0};
    context.stages = InitializeStages();
    context.player = InitializePlayer();
    return context;
}

void InitializeEnemyBullets(Bullet enemyBullets[], int maxEnemyBullets){
    Texture2D enemyBulletTexture = LoadTexture("resources/enemybullet.png");
    if (enemyBulletTexture.id == 0) {
        printf("Failed to load enemybullet.png\n");
        return;
    }     

    for (int i = 0; i < maxEnemyBullets; i++) {
        enemyBullets[i].rec = (Rectangle){0, 0, 5, 10};
        enemyBullets[i].active = false;
        enemyBullets[i].spd = 5.0f;
        enemyBullets[i].dmg = 100.0f;
        enemyBullets[i].texture = enemyBulletTexture;
    }
}

Player InitializePlayer(){
    Player player = 
    {
        0, 
        {40, 750, 64, 64}, 
        1, 
        1.0f, 
        100.0f, 
        100.0f, 
        30.0f, 
        30.0f, 
        100.0f, 
        100.0f, 
        5.0f, 
        22.0f, 
        false, 
        1.0f, 
        3, 
        EASY
    };
    return player;
}

typedef enum { BOT, MID, TOP } BuffTier;

typedef struct {
    char* name;
    BuffTier tier;
    void (*apply)(GameContext*);
    char* description;
    Color color;
} Buff;

void SmallHealthBoost(GameContext *context) {
    context->player.HP += 10; // Increase HP by a small amount
    context->player.maxHP +=  10;
}

void MinorSpeedBoost(GameContext *context) {
    context->player.speed += 1; // Increase speed by a small amount
}

void SmallDamageBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].dmg += 5.0f;
    }
}

void MediumHealthBoost(GameContext *context) {
    context->player.HP += 20; // Increase HP by a medium amount
    context->player.maxHP +=  20;
}

void MediumSpeedBoost(GameContext *context) {
    context->player.speed += 2; // Increase speed by a medium amount
}

void MediumDamageBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].dmg += 20.0f;
}
}

void HighHealthBoost(GameContext *context) {
    context->player.HP += 30; // Increase HP by a large amount
    context->player.maxHP +=  30;
}

void HighSpeedBoost(GameContext *context) {
    context->player.speed += 3; // Increase speed by a large amount
}

void HighDamageBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].dmg += 30.0f;
    }
}

void SmallSpecialAttackBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->spBullets[i].dmg += 30.0f;
    }
}

void MediumSpecialAttackBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->spBullets[i].dmg += 45.0f;
    }
}

void HighSpecialAttackBoost(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->spBullets[i].dmg += 60.0f;
    }
}

void SmallStaminaRegenBoost(GameContext *context) {
    context->player.staminaReg += 1;
}

void MediumStaminaRegenBoost(GameContext *context) {
    context->player.staminaReg += 1.5;
}

void HighStaminaRegenBoost(GameContext *context) {
    context->player.staminaReg += 2;
}

void SmallEnergyMaxBoost(GameContext *context) {
    context->player.maxEnergy += 20.0f;
    context->player.energy += 20.0f;  // Also increase current energy
}

void MediumEnergyMaxBoost(GameContext *context) {
    context->player.maxEnergy += 40.0f;
    context->player.energy += 40.0f;
}

void HighEnergyMaxBoost(GameContext *context) {
    context->player.maxEnergy += 60.0f;
    context->player.energy += 60.0f;
}

void SmallEnergyRegenBoost(GameContext *context) {
    context->player.energyRechargeRate += 1.0f;
}

void MediumEnergyRegenBoost(GameContext *context) {
    context->player.energyRechargeRate += 2.0f;
}

void HighEnergyRegenBoost(GameContext *context) {
    context->player.energyRechargeRate += 3.0f;
}

void SmallShieldMaxBoost(GameContext *context) {
    context->player.maxShield += 10.0f;
    context->player.shield += 10.0f;  // Also increase current shield
}

void MediumShieldMaxBoost(GameContext *context) {
    context->player.maxShield += 20.0f;
    context->player.shield += 20.0f;
}

void HighShieldMaxBoost(GameContext *context) {
    context->player.maxShield += 30.0f;
    context->player.shield += 30.0f;
}

void SmallEnergyCostReduction(GameContext *context) {
    context->player.energyCost *= 0.9f;  // Reduce energy cost by 10%
}

void MediumEnergyCostReduction(GameContext *context) {
    context->player.energyCost *= 0.8f;  // Reduce energy cost by 20%
}

void HighEnergyCostReduction(GameContext *context) {
    context->player.energyCost *= 0.7f;  // Reduce energy cost by 30%
}

Buff lowTierBuffs[] = {
    {"Health Boost I", BOT, SmallHealthBoost, "HP +10, Max HP +10", BROWN},
    {"Speed Boost I", BOT, MinorSpeedBoost, "Speed +1", BROWN},
    {"Damage Boost I", BOT, SmallDamageBoost, "Bullet Damage +5", BROWN},
    {"Attack Speed I", BOT, SmallStaminaRegenBoost, "Stamina Regen +1", BROWN},
    {"Energy Max I", BOT, SmallEnergyMaxBoost, "Energy Max +20", BROWN},
    {"Energy Regen I", BOT, SmallEnergyRegenBoost, "Energy Regen +2", BROWN},
    {"Shield Max I", BOT, SmallShieldMaxBoost, "Shield Max +10", BROWN},
    {"Energy Cost I", BOT, SmallEnergyCostReduction, "Energy Cost -10%", BROWN}
};

Buff mediumTierBuffs[] = {
    {"Health Boost II", MID, MediumHealthBoost, "HP +20, Max HP +20", GRAY},
    {"Speed Boost II", MID, MediumSpeedBoost, "Speed +2", GRAY},
    {"Damage Boost II", MID, MediumDamageBoost, "Bullet Damage +10", GRAY},
    {"Attack Speed II", MID, MediumStaminaRegenBoost, "Stamina Regen +1.5", GRAY},
    {"Energy Max II", MID, MediumEnergyMaxBoost, "Energy Max +40", GRAY},
    {"Energy Regen II", MID, MediumEnergyRegenBoost, "Energy Regen +3", GRAY},
    {"Shield Max II", MID, MediumShieldMaxBoost, "Shield Max +20", GRAY},
    {"Energy Cost II", MID, MediumEnergyCostReduction, "Energy Cost -20%", GRAY}
};

Buff highTierBuffs[] = {
    {"Health Boost III", TOP, HighHealthBoost, "HP +30, Max HP +30", GOLD},
    {"Speed Boost III", TOP, HighSpeedBoost, "Speed +3", GOLD},
    {"Damage Boost III", TOP, HighDamageBoost, "Bullet Damage +15", GOLD},
    {"Attack Speed III", TOP, HighStaminaRegenBoost, "Stamina Regen +2", GOLD},
    {"Energy Max III", TOP, HighEnergyMaxBoost, "Energy Max +60", GOLD},
    {"Energy Regen III", TOP, HighEnergyRegenBoost, "Energy Regen +4", GOLD},
    {"Shield Max III", TOP, HighShieldMaxBoost, "Shield Max +30", GOLD},
    {"Energy Cost III", TOP, HighEnergyCostReduction, "Energy Cost -30%", GOLD}
};

BuffTier GetRandomTier() {
    int randomValue = rand() % 100;  // Generate a random number between 0 and 99

    if (randomValue < 60) {
        return BOT;  // 60% chance
    } else if (randomValue < 90) {
        return MID;  // 30% chance
    } else {
        return TOP;  // 10% chance
    }
}

Buff SelectFromTier(BuffTier tier) {
    Buff selectedBuff;

    switch (tier) {
        case BOT: {
            int numBuffs = sizeof(lowTierBuffs) / sizeof(lowTierBuffs[0]);
            int randomIndex = rand() % numBuffs;
            selectedBuff = lowTierBuffs[randomIndex];
            break;
        }
        case MID: {
            int numBuffs = sizeof(mediumTierBuffs) / sizeof(mediumTierBuffs[0]);
            int randomIndex = rand() % numBuffs;
            selectedBuff = mediumTierBuffs[randomIndex];
            break;
        }
        case TOP: {
            int numBuffs = sizeof(highTierBuffs) / sizeof(highTierBuffs[0]);
            int randomIndex = rand() % numBuffs;
            selectedBuff = highTierBuffs[randomIndex];
            break;
        }
    }

    return selectedBuff;
}

Buff GetRandomBuff() {
    BuffTier chosenTier = GetRandomTier();   // Choose a random tier based on weighted probability
    return SelectFromTier(chosenTier);       // Select a random buff from the chosen tier
}

void TakeDamage(Player *player, int damage) {
    if (player->shield > 0) {
        // Shield absorbs damage
        player->shield -= damage;
        if (player->shield < 0) {
            // If shield is depleted, apply remaining damage to HP
            player->HP += player->shield;  // player->shield is negative here
            player->shield = 0;
        }
    } else {
        // No shield, apply damage to HP
        player->HP -= damage;
    }

    if (player->HP <= 0) {
        // Handle player death
        printf("Player is dead!\n");
    }
}

void GenerateNewBuffs(Buff *buff1, Buff *buff2, Buff *buff3) {
    *buff1 = GetRandomBuff();
    *buff2 = GetRandomBuff();
    *buff3 = GetRandomBuff();
    printf("New Buff Assigned");
}

bool AllEnemiesDestroyed(GameContext *context) {
    // Check if any regular enemies are still active
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->enemies[i].active) {
            return false; // Found an active regular enemy
        }
    }

    // Check if any elite enemies are still active
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->eliteEnemies[i].active) {
            return false; // Found an active elite enemy
        }
    }

    // Check if any bosses are still active
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->Boss[i].active) {
            return false; // Found an active boss
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].active = false;
        context->spBullets[i].active = false;
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        context->enemyBullets[i].active = false;
        context->eliteEnemyBullets[i].active = false;
    }

    printf("The Stage is Cleared");
    return true; // All enemies and elite enemies are inactive
}

void SelectBuff(Buff selectedBuff, GameContext *context) {
    selectedBuff.apply(context); // Pass the player to the buff function
    printf("\nApplying buff: %s", selectedBuff.name);
}

void DrawPlayerStats(Player *player) {
    
    DrawRectangle(0, 0, 600, 70, DARKGRAY);
    char stageText[32];
    sprintf(stageText, "Stage: %d", player->stageIndex);
    DrawText(stageText, 240, 20, 30, WHITE);

    char difficultyText[32];
    char difficultyString[10];
    switch (player->difficulty) {
        case EASY:
            strcpy(difficultyString, "Easy");
            break;
        case MEDIUM:
            strcpy(difficultyString, "Medium");
            break;
        case HARD:
            strcpy(difficultyString, "Hard");
            break;
    }
    sprintf(difficultyText, "Difficulty: %s", difficultyString);
    DrawText(difficultyText, 400, 40, 20, WHITE);

    //Draw a border around the screen
    DrawRectangleLinesEx((Rectangle){0, 0, 600, 950}, 5, DARKGRAY);

    // Draw HP bar
    DrawRectangle(10, 10, 200, 20, GRAY);
    DrawRectangle(10, 10, (player->HP/player->maxHP) * 200, 20, RED);
    // Add HP text below the bar
    char hpText[32];
    sprintf(hpText, "HP: %.0f/%.0f", player->HP, player->maxHP);
    DrawText(hpText, 11, 11, 20, WHITE);  // Position text below HP bar

    // Draw Shield bar
    DrawRectangle(10, 40, 200, 20, GRAY);
    DrawRectangle(10, 40, (player->shield/player->maxShield) * 200, 20, BLUE);
    // Add Shield text below the bar
    char shieldText[32];
    sprintf(shieldText, "AP: %.0f/%.0f", player->shield, player->maxShield);
    DrawText(shieldText, 11, 41, 20, WHITE);  // Position text below shield bar

    // Draw Energy bar
    DrawRectangle(390, 10, 200, 20, GRAY);
    DrawRectangle(390, 10, (player->energy/player->maxEnergy) * 200, 20, YELLOW);
    // Add Energy text below the bar
    char energyText[32];
    sprintf(energyText, "Energy: %.0f/%.0f", player->energy, player->maxEnergy);
    DrawText(energyText, 391, 11, 20, BLACK);  // Position text below energy bar

    // Draw Stamina bar if you have one
    if (player->stamina < 1.0f) {
        DrawRectangle(player->rec.x, player->rec.y - 20, 64, 10, GRAY);
        DrawRectangle(player->rec.x, player->rec.y - 20, player->stamina * 64, 10, ORANGE);
    }
}

void RecoverShield(Player *player) {
    player->shield = player->maxShield;  // Fully restore shield after each stage
}

void InitEnemySprite(EnemySprite *sprite) {
    // Load all 4 frames
    sprite->textures[0] = LoadTexture("resources/Enemy-1.png");
    sprite->textures[1] = LoadTexture("resources/Enemy-1.png");
    sprite->textures[2] = LoadTexture("resources/Enemy-1.png");
    sprite->textures[3] = LoadTexture("resources/Enemy-1.png");
    
    // Check if all textures loaded successfully
    for(int i = 0; i < 4; i++) {
        if (sprite->textures[i].id == 0) {
            printf("Failed to load enemy%d.png\n", i+1);
            return;
        }
    }
    
    sprite->currentFrame = 0;
    sprite->framesCounter = 0;
    sprite->framesSpeed = 8;  // Adjust for animation speed
}

void InitEliteEnemySprite(EnemySprite *sprite) {
    // Load all 4 frames
    sprite->textures[0] = LoadTexture("resources/eliteenemies.png");
    sprite->textures[1] = LoadTexture("resources/eliteenemies1.png");
    sprite->textures[2] = LoadTexture("resources/eliteenemies2.png");
    sprite->textures[3] = LoadTexture("resources/eliteenemies3.png");

    for(int i = 0; i < 4; i++) {
        if (sprite->textures[i].id == 0) {
            printf("Failed to load enemy%d.png\n", i+1);
            return;
        }
    }

    sprite->currentFrame = 0;
    sprite->framesCounter = 0;
    sprite->framesSpeed = 8;  // Adjust for animation speed
}

void InitBossSprite(EnemySprite *sprite) {
    // Load all 4 frames
    sprite->textures[0] = LoadTexture("resources/Boss.png");
    sprite->textures[1] = LoadTexture("resources/Boss.png");
    sprite->textures[2] = LoadTexture("resources/Boss.png");
    sprite->textures[3] = LoadTexture("resources/Boss.png");

    for(int i = 0; i < 4; i++) {
        if (sprite->textures[i].id == 0) {
            printf("Failed to load enemy%d.png\n", i+1);
            return;
        }
    }

    sprite->currentFrame = 0;
    sprite->framesCounter = 0;
    sprite->framesSpeed = 8;  // Adjust for animation speed
}

// Update enemy animation
void UpdateEnemyAnimation(EnemySprite *sprite) {
    sprite->framesCounter++;
    
    if (sprite->framesCounter >= sprite->framesSpeed) {
        sprite->framesCounter = 0;
        sprite->currentFrame++;
        
        if (sprite->currentFrame >= 4) {
            sprite->currentFrame = 0;
        }
    }
}

// Draw animated enemy
void DrawAnimatedEnemy(Rectangle destRec, EnemySprite *sprite) {
    Rectangle scaledRec = {
        destRec.x,
        destRec.y,
        destRec.width * 1.5f,  // Scale width by 1.5 (adjust this value as needed)
        destRec.height * 1.5f  // Scale height by 1.5 (adjust this value as needed)
    };
    
    DrawTexturePro(
        sprite->textures[sprite->currentFrame],
        (Rectangle){ 0, 0, 
                    sprite->textures[sprite->currentFrame].width, 
                    sprite->textures[sprite->currentFrame].height },
        scaledRec,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void DrawAnimatedEliteEnemy(Rectangle destRec, EnemySprite *sprite) {
    Rectangle scaledRec = {
        destRec.x,
        destRec.y,
        destRec.width * 1.5f,  // Scale width by 1.5 (adjust this value as needed)
        destRec.height * 1.5f  // Scale height by 1.5 (adjust this value as needed)
    };
    
    DrawTexturePro(
        sprite->textures[sprite->currentFrame],
        (Rectangle){ 0, 0, 
                    sprite->textures[sprite->currentFrame].width, 
                    sprite->textures[sprite->currentFrame].height },
        scaledRec,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void DrawAnimatedBoss(Rectangle destRec, EnemySprite *sprite) {
    Rectangle scaledRec = {
        destRec.x,
        destRec.y,
        destRec.width * 1.0f,  // Scale width by 1.5 (adjust this value as needed)
        destRec.height * 1.0f  // Scale height by 1.5 (adjust this value as needed)
    };
    
    DrawTexturePro(
        sprite->textures[sprite->currentFrame],
        (Rectangle){ 0, 0, 
                    sprite->textures[sprite->currentFrame].width, 
                    sprite->textures[sprite->currentFrame].height },
        scaledRec,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void InitMenuStars(GameContext *context) {
    for (int i = 0; i < MAX_STARS; i++) {
        context->stars[i].position.x = GetRandomValue(0, GetScreenWidth());
        context->stars[i].position.y = GetRandomValue(0, GetScreenHeight());
        context->stars[i].speed = GetRandomValue(100, 400) / 100.0f;
        context->stars[i].radius = GetRandomValue(1, 3);
        context->stars[i].brightness = GetRandomValue(50, 255) / 255.0f;
    }
}

void UpdateStars(GameContext *context) {
    for (int i = 0; i < MAX_STARS; i++) {
        context->stars[i].position.y += context->stars[i].speed;
        if (context->stars[i].position.y > GetScreenHeight()) {
            context->stars[i].position.y = 0;
            context->stars[i].position.x = GetRandomValue(0, GetScreenWidth());
        }
    }
}

void DrawStars(GameContext *context) {
    for (int i = 0; i < MAX_STARS; i++) {
        Color starColor = { 
            255, 
            255, 
            255, 
            (unsigned char)(context->stars[i].brightness * 255)
        };
        DrawCircleV(context->stars[i].position, context->stars[i].radius, starColor);
    }
}

void InitSpaceshipSprite(AnimatedSprite *sprite) {
    // Load all 4 textures
    sprite->textures[0] = LoadTexture("resources/spaceship-1.png");
    sprite->textures[1] = LoadTexture("resources/spaceship-2.png");
    sprite->textures[2] = LoadTexture("resources/spaceship-3.png");
    sprite->textures[3] = LoadTexture("resources/spaceship-4.png");

    // Load movement textures
    sprite->leftTexture = LoadTexture("resources/spaceship-left.png");
    sprite->rightTexture = LoadTexture("resources/spaceship-right.png");
    
    // Check if all textures loaded successfully
    for(int i = 0; i < 4; i++) {
        if (sprite->textures[i].id == 0) {
            printf("Failed to load spaceship%d.png\n", i+1);
            return;
        }
    }
    if (sprite->leftTexture.id == 0 || sprite->rightTexture.id == 0) {
        printf("Failed to load movement textures\n");
        return;
    }
    
    sprite->currentFrame = 0;
    sprite->framesCounter = 0;
    sprite->framesSpeed = 8;  // Adjust this to change animation speed (lower = faster)
}

void UpdateSpaceshipAnimation(AnimatedSprite *sprite) {
    sprite->framesCounter++;
    
    if (sprite->state == SHIP_IDLE) {
        sprite->framesCounter++;
        
        if (sprite->framesCounter >= sprite->framesSpeed) {
            sprite->framesCounter = 0;
            sprite->currentFrame++;
            
            if (sprite->currentFrame >= 4) {
                sprite->currentFrame = 0;
            }
        }
    }
}

void DrawAnimatedSpaceship(Rectangle destRec, AnimatedSprite *sprite) {
    if (sprite->state == SHIP_IDLE) {
        // Draw current frame of idle animation
        DrawTexturePro(
            sprite->textures[sprite->currentFrame],
            (Rectangle){ 0, 0, 
                        sprite->textures[sprite->currentFrame].width, 
                        sprite->textures[sprite->currentFrame].height },
            destRec,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
    } else if (sprite->state == SHIP_LEFT) {
        // Draw left movement sprite
        DrawTexturePro(
            sprite->leftTexture,
            (Rectangle){ 0, 0, sprite->leftTexture.width, sprite->leftTexture.height },
            destRec,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
    } else if (sprite->state == SHIP_RIGHT) {
        // Draw right movement sprite
        DrawTexturePro(
            sprite->rightTexture,
            (Rectangle){ 0, 0, sprite->rightTexture.width, sprite->rightTexture.height },
            destRec,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
    }
}

void UnloadSpaceshipSprites(AnimatedSprite *sprite) {
    for(int i = 0; i < 4; i++) {
        UnloadTexture(sprite->textures[i]);
    }
}

void UnloadEnemySprites(Enemy *enemies) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            for (int j = 0; j < 4; j++) {
                UnloadTexture(enemies[i].sprite.textures[j]);
            }
        }
    }
}

void InitializeEnemies(GameContext *context) {
    Stage currentStage;
    int stageIndex = context->player.stageIndex;
    float hpMultiplier;
    
    switch(context->player.difficulty) {
        case EASY:
            if (stageIndex < context->stages.easyCount) {
                currentStage = context->stages.easy[stageIndex];
                hpMultiplier = context->stages.easy[stageIndex].hpMultiplier;
            }
            break;
        case MEDIUM:
            if (stageIndex < context->stages.mediumCount) {
                currentStage = context->stages.medium[stageIndex];
                hpMultiplier = context->stages.medium[stageIndex].hpMultiplier;
            }
            break;
        case HARD:
            if (stageIndex < context->stages.hardCount) {
                currentStage = context->stages.hard[stageIndex];
                hpMultiplier = context->stages.hard[stageIndex].hpMultiplier;
            }
            break;
    }

    // Calculate total stage health before initializing enemies
    context->maxStageHealth = (currentStage.enemyCount * 100.0f + 
                              currentStage.eliteEnemyCount * 500.0f + 
                              currentStage.bossCount * 3000.0f) * hpMultiplier;

    for (int i = 0; i < currentStage.enemyCount; i++) {

        float initialX = GetRandomValue(25, 575);
        float initialY = GetRandomValue(100, 500);
        float movementRange = GetRandomValue(70, 200);

        context->enemies[i].rec = (Rectangle){initialX, initialY, 70, 40};
        context->enemies[i].active = true;
        context->enemies[i].speed = 2.5 + currentStage.spdMultiplier; // Increase speed with dmgMultiplier
        context->enemies[i].fireRate = 3.0f * currentStage.fireRateMultiplier;
        context->enemies[i].hp = 100.0f * currentStage.hpMultiplier; // Increase enemy HP with the stage's multiplier
        context->enemies[i].direction = 1;
        context->enemies[i].minX = fmax(0, initialX - movementRange);
        context->enemies[i].maxX = fmin(600 - context->enemies[i].rec.width, initialX + movementRange);
            
            // Initialize sprite for this enemy
        InitEnemySprite(&context->enemies[i].sprite);

    }
    for (int i = 0; i < currentStage.eliteEnemyCount; i++)
    {
        float initialX = GetRandomValue(25, 575);
        float initialY = GetRandomValue(100, 500);
        float movementRange = GetRandomValue(70, 150);

        context->eliteEnemies[i].rec = (Rectangle){initialX, initialY, 70, 40};
        context->eliteEnemies[i].active = true;
        context->eliteEnemies[i].speed = 2.5 + currentStage.spdMultiplier; // Increase speed with dmgMultiplier
        context->eliteEnemies[i].fireRate = 3.0f * currentStage.fireRateMultiplier;
        context->eliteEnemies[i].hp = 500.0f * currentStage.hpMultiplier; // Increase enemy HP with the stage's multiplier
        context->eliteEnemies[i].direction = 1;
        context->eliteEnemies[i].minX = fmax(0, initialX - movementRange);
        context->eliteEnemies[i].maxX = fmin(600 - context->eliteEnemies[i].rec.width, initialX + movementRange);

        // Initialize sprite for this enemy
        InitEliteEnemySprite(&context->eliteEnemies[i].sprite);
    }

    for (int i = 0; i < currentStage.bossCount; i++)
    {   

        float initialX = GetRandomValue(100, 250);
        float initialY = GetRandomValue(100, 250);
        float movementRange = GetRandomValue(50, 100);

        context->Boss[i].rec = (Rectangle){initialX, initialY, 220, 150};
        context->Boss[i].active = true;
        context->Boss[i].speed = 1.0 + currentStage.spdMultiplier; // Increase speed with dmgMultiplier
        context->Boss[i].fireRate = 3.0f * currentStage.fireRateMultiplier;
        context->Boss[i].hp = 3000.0f * currentStage.hpMultiplier; // Increase enemy HP with the stage's multiplier
        context->Boss[i].direction = 1;
        context->Boss[i].minX = fmax(0, initialX - movementRange);
        context->Boss[i].maxX = fmin(600 - context->Boss[i].rec.width, initialX + movementRange);
        // Initialize sprite for this enemy
        InitBossSprite(&context->Boss[i].sprite);
    }

    for (int i = currentStage.enemyCount; i < MAX_ENEMIES; i++)
    {
        context->enemies[i].active = false;
    }

    for (int i = currentStage.eliteEnemyCount; i < MAX_ENEMIES; i++)
    {
        context->eliteEnemies[i].active = false;
    }

    for (int i = currentStage.bossCount; i < MAX_ENEMIES; i++)
    {
        context->Boss[i].active = false;
    }
    
}

void ReinitializeGame(GameContext *context) {
    context->player.rec = (Rectangle){40, 750, 64, 64};
    context->player.HP = 100.0;
    context->player.maxHP = 100.0;
    context->player.shield = 30.0f;
    context->player.maxShield = 30.0f;
    context->player.fireRate = 0.3f;
    context->player.speed = 5;
    context->player.stageIndex = 0;
    context->player.energy = 100.0f;
    context->player.stamina = 1.0f;
    context->player.difficulty = context->player.difficulty;  // Set the difficulty
    context->gameStartTime = 0;
    context->isGameTimerRunning = false;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        context->enemies[i].active = false;
        context->eliteEnemies[i].active = false;
        context->Boss[i].active = false;
    }

    // Reset all bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].active = false;
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        context->enemyBullets[i].active = false;
        context->eliteEnemyBullets[i].active = false;
    }

    InitSpaceshipSprite(&context->player.sprite);
}

void DrawEnemies(GameContext *context) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->enemies[i].active) DrawAnimatedEnemy(context->enemies[i].rec, &context->enemies[i].sprite);
        if (context->eliteEnemies[i].active) DrawAnimatedEliteEnemy(context->eliteEnemies[i].rec, &context->eliteEnemies[i].sprite); // Elite enemies appear in a different color
        if (context->Boss[i].active) DrawAnimatedBoss(context->Boss[i].rec, &context->Boss[i].sprite);
    }
}

void InitializeBullets(GameContext *context){
    Texture2D enemyBulletTexture = LoadTexture("resources/EnemyBullet.png");
    Texture2D BulletTexture = LoadTexture("resources/playerbullet.png");
    Texture2D spBulletTexture = LoadTexture("resources/spbullet.png");
    Texture2D eliteEnemyBulletTexture = LoadTexture("resources/elitebullet.png");

    
    // Set all bullets as inactive initially
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->bullets[i].rec = (Rectangle){0, 0, 5, 10}; // Example bullet size
        context->bullets[i].active = false;
        context->bullets[i].spd = 5.0f;
        context->bullets[i].dmg = 100.0f;
        context->bullets[i].texture = BulletTexture;
    }
    for (int i = 0; i < MAX_BULLETS; i++) {
        context->spBullets[i].rec = (Rectangle){0, 0, 40, 20}; // Example bullet size
        context->spBullets[i].active = false;
        context->spBullets[i].spd = 15.0f;
        context->spBullets[i].dmg = 250.0f;
        context->spBullets[i].texture = spBulletTexture;
    }
    // Enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        context->enemyBullets[i].rec = (Rectangle){0, 0, 16, 32};  // Adjust size to match your sprite
        context->enemyBullets[i].active = false;
        context->enemyBullets[i].spd = 5.0f;
        context->enemyBullets[i].dmg = 10.0f;
        context->enemyBullets[i].texture = enemyBulletTexture;
    }

    // Elite enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        context->eliteEnemyBullets[i].rec = (Rectangle){0, 0, 12, 24};  // Larger size for elite bullets
        context->eliteEnemyBullets[i].active = false;
        context->eliteEnemyBullets[i].spd = 7.0f;
        context->eliteEnemyBullets[i].dmg = 20.0f;
        context->eliteEnemyBullets[i].texture = eliteEnemyBulletTexture; 
    }
}

void FireBullet(GameContext *context) {
    // Find an inactive bullet to shoot
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!context->bullets[i].active) {
            context->bullets[i].rec.x = context->player.rec.x + context->player.rec.width / 2 - context->bullets[i].rec.width / 2;
            context->bullets[i].rec.y = context->player.rec.y;
            context->bullets[i].active = true;
            break; 
        }
    }
}

void FireSpBullet(GameContext *context) {
    // Find an inactive bullet to shoot
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!context->spBullets[i].active) {
            context->spBullets[i].rec.x = context->player.rec.x + context->player.rec.width / 2 - context->spBullets[i].rec.width / 2;
            context->spBullets[i].rec.y = context->player.rec.y;
            context->spBullets[i].active = true;
            break; 
        }
    }
}

void RechargeEnergy(float deltaTime, GameContext *context) {
    context->player.energy += context->player.energyRechargeRate * deltaTime;
    if (context->player.energy > context->player.maxEnergy) context->player.energy = context->player.maxEnergy; 

    context->player.stamina  += deltaTime*context->player.staminaReg;
    if (context->player.stamina > 1.0f) context->player.stamina = 1.0f;
}

void FireEliteEnemyBullet(GameContext *context) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!context->eliteEnemyBullets[i].active) {
            context->eliteEnemyBullets[i].rec = (Rectangle){context->eliteEnemies[i].rec.x + context->eliteEnemies[i].rec.width / 2 - 2.5f, context->eliteEnemies[i].rec.y + context->eliteEnemies[i].rec.height, 5, 10};
            context->eliteEnemyBullets[i].active = true;
            context->eliteEnemyBullets[i].spd = 5.0f; // Faster speed for elite enemy bullets
            break;
        }
    }
}

void FireEnemyBullet(GameContext *context, int enemyIndex) {
    // Find an inactive bullet to shoot
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!context->enemyBullets[i].active) {
            context->enemyBullets[i].rec = (Rectangle){
                context->enemies[enemyIndex].rec.x + context->enemies[enemyIndex].rec.width / 2 - 2.5f, // Center bullet horizontally relative to enemy
                context->enemies[enemyIndex].rec.y + context->enemies[enemyIndex].rec.height, // Start just below the enemy
                5, 10 // Width and height of the bullet
            };
            context->enemyBullets[i].active = true;
            context->enemyBullets[i].spd = 3.0f; // Set bullet speed
            break;
        }
    }
}

void UpdateEnemies(GameContext *context) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->enemies[i].active) {
            // Move the enemy based on its speed and direction
            context->enemies[i].rec.x += context->enemies[i].direction * context->enemies[i].speed;

            // Check if the enemy reaches its movement bounds and change direction
            if (context->enemies[i].rec.x > context->enemies[i].maxX) {
                context->enemies[i].direction = -1; // Move left
            } else if (context->enemies[i].rec.x < context->enemies[i].minX) {
                context->enemies[i].direction = 1; // Move right
            }
            
            UpdateEnemyAnimation(&context->enemies[i].sprite);

            // Manage each enemy's firing cooldown independently
            context->enemies[i].fireRate -= GetFrameTime(); // Decrease the timer for each enemy

            if (context->enemies[i].fireRate <= 0.0f) {
                FireEnemyBullet(context, i);
                context->enemies[i].fireRate = 2.0f; // Reset the fire rate cooldown after shooting
            }
        }

        if (context->eliteEnemies[i].active) {
            context->eliteEnemies[i].rec.x += context->eliteEnemies[i].direction * context->eliteEnemies[i].speed;
            if (context->eliteEnemies[i].rec.x > context->eliteEnemies[i].maxX) context->eliteEnemies[i].direction = -1;
            else if (context->eliteEnemies[i].rec.x < context->eliteEnemies[i].minX) context->eliteEnemies[i].direction = 1;

            context->eliteEnemies[i].fireRate -= GetFrameTime();
            if (context->eliteEnemies[i].fireRate <= 0.0f) {
                FireEliteEnemyBullet(context);
                context->eliteEnemies[i].fireRate = 1.5f;
            }

            UpdateEnemyAnimation(&context->eliteEnemies[i].sprite);
        }

        if (context->Boss[i].active) {
            context->Boss[i].rec.x += context->Boss[i].direction * context->Boss[i].speed;
            if (context->Boss[i].rec.x > context->Boss[i].maxX) context->Boss[i].direction = -1;
            else if (context->Boss[i].rec.x < context->Boss[i].minX) context->Boss[i].direction = 1;

            context->Boss[i].fireRate -= GetFrameTime();
            if (context->Boss[i].fireRate <= 0.0f) {
                FireEliteEnemyBullet(context);
                context->Boss[i].fireRate = 1.5f;
            }

            UpdateEnemyAnimation(&context->Boss[i].sprite);
        }
    }
}   

void UpdateBullets(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->bullets[i].active) {
            context->bullets[i].rec.y -= context->bullets[i].spd; // Move the bullet upwards

            // Deactivate the bullet if it goes off-screen
            if (context->bullets[i].rec.y + context->bullets[i].rec.height < 0) {
                context->bullets[i].active = false;
            }

            // Check for collisions with enemies
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (context->enemies[j].active && CheckCollisionRecs(context->bullets[i].rec, context->enemies[j].rec)) 
                {
                    context->bullets[i].active = false; // Deactivate the bullet
                    context->enemies[j].hp = context->enemies[j].hp - context->bullets[i].dmg;
                    if (context->enemies[j].hp <= 0.01f)
                    {
                        context->enemies[j].active = false;
                    }
                    break; // Exit the inner loop since the bullet is now inactive
                }
                if (context->eliteEnemies[j].active && CheckCollisionRecs(context->bullets[i].rec, context->eliteEnemies[j].rec)) 
                {
                    context->bullets[i].active = false; // Deactivate the bullet
                    context->eliteEnemies[j].hp = context->eliteEnemies[j].hp - context->bullets[i].dmg;
                    if (context->eliteEnemies[j].hp <= 0.01f)
                    {
                        context->eliteEnemies[j].active = false;
                    }
                    break; // Exit the inner loop since the bullet is now inactive
                }
                if (context->Boss[j].active && CheckCollisionRecs(context->bullets[i].rec, context->Boss[j].rec)) 
                {
                    context->bullets[i].active = false; // Deactivate the bullet
                    context->Boss[j].hp = context->Boss[j].hp - context->bullets[i].dmg;
                    break; // Exit the inner loop since the bullet is now inactive
                }
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->spBullets[i].active) {
            context->spBullets[i].rec.y -= context->spBullets[i].spd; // Move the bullet upwards

            // Deactivate the bullet if it goes off-screen
            if (context->spBullets[i].rec.y + context->spBullets[i].rec.height < 0) {
                context->spBullets[i].active = false;
            }

            for (int j = 0; j < MAX_ENEMY_BULLETS; j++) {
                // Regular enemy bullets
                if (context->enemyBullets[j].active) {
                    if (CheckCollisionRecs(context->spBullets[i].rec, context->enemyBullets[j].rec)) {
                        printf("SP Bullet hit enemy bullet! Deactivating enemy bullet %d\n", j);
                        context->enemyBullets[j].active = false;
                    }
                }
                
                // Elite enemy bullets
                if (context->eliteEnemyBullets[j].active) {
                    if (CheckCollisionRecs(context->spBullets[i].rec, context->eliteEnemyBullets[j].rec)) {
                        printf("SP Bullet hit elite bullet! Deactivating elite bullet %d\n", j);
                        context->eliteEnemyBullets[j].active = false;
                    }
                }
            }

            // Check for collisions with enemies
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (context->enemies[j].active && CheckCollisionRecs(context->spBullets[i].rec, context->enemies[j].rec)) 
                {
                    context->spBullets[i].active = false;
                    context->enemies[j].hp = context->enemies[j].hp - context->spBullets[i].dmg;
                    if (context->enemies[j].hp <= 0.01f)
                    {
                        context->enemies[j].active = false;
                    }
                    break; // Exit the inner loop since the bullet is now inactive
                }
                if (context->eliteEnemies[j].active && CheckCollisionRecs(context->spBullets[i].rec, context->eliteEnemies[j].rec)) 
                {
                    context->spBullets[i].active = false;
                    context->eliteEnemies[j].hp = context->eliteEnemies[j].hp - context->spBullets[i].dmg;
                    if (context->eliteEnemies[j].hp <= 0.01f)
                    {
                        context->eliteEnemies[j].active = false;
                    }
                    break; // Exit the inner loop since the bullet is now inactive
                }
                if (context->Boss[j].active && CheckCollisionRecs(context->spBullets[i].rec, context->Boss[j].rec)) 
                {
                    context->spBullets[i].active = false;
                    context->Boss[j].hp = context->Boss[j].hp - context->spBullets[i].dmg;
                    if (context->Boss[j].hp <= 0.01f)
                    {
                        context->Boss[j].active = false;
                    }
                    break; // Exit the inner loop since the bullet is now inactive
                }
            }
        }
    }
}

void UpdateEnemyBullets(GameContext *context) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (context->enemyBullets[i].active) {
            context->enemyBullets[i].rec.y += context->enemyBullets[i].spd; // Move the bullet downwards

            // Deactivate the bullet if it goes off-screen
            if (context->enemyBullets[i].rec.y > 950) {
                context->enemyBullets[i].active = false;
            }

            // Check for collisions with the player
            if (CheckCollisionRecs(context->enemyBullets[i].rec, context->player.rec)) {
                context->enemyBullets[i].active = false; // Deactivate the bullet
                TakeDamage(&context->player, context->enemyBullets[i].dmg);
            }
        }
    }
}

void UpdateEliteEnemyBullets(GameContext *context) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (context->eliteEnemyBullets[i].active) {
            context->eliteEnemyBullets[i].rec.y += context->eliteEnemyBullets[i].spd;
            if (context->eliteEnemyBullets[i].rec.y > 950) context->eliteEnemyBullets[i].active = false;

            if (CheckCollisionRecs(context->player.rec, context->eliteEnemyBullets[i].rec)) {
                TakeDamage(&context->player, 25);
                context->eliteEnemyBullets[i].active = false;
            }
        }
    }
}

void DrawBullets(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->bullets[i].active) {
            // Calculate scaled dimensions
            float scaledWidth = context->bullets[i].texture.width * 3.0f;
            float scaledHeight = context->bullets[i].texture.height * 3.0f;
            
            // Center the scaled sprite on the bullet's position
            Rectangle destRec = {
                context->bullets[i].rec.x - (scaledWidth - context->bullets[i].rec.width) / 2,
                context->bullets[i].rec.y - (scaledHeight - context->bullets[i].rec.height) / 2,
                scaledWidth,
                scaledHeight
            };

            DrawTexturePro(
                context->bullets[i].texture,
                (Rectangle){ 0, 0, 
                            context->bullets[i].texture.width,
                            context->bullets[i].texture.height },
                destRec,
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }
        if (context->spBullets[i].active) {
            float scaledWidth = context->spBullets[i].texture.width * 3.0f;
            float scaledHeight = context->spBullets[i].texture.height * 3.0f;
            
            // Center the scaled sprite on the bullet's position
            Rectangle destRec = {
                context->spBullets[i].rec.x - (scaledWidth - context->spBullets[i].rec.width) / 2,
                context->spBullets[i].rec.y - (scaledHeight - context->spBullets[i].rec.height) / 2,
                scaledWidth,
                scaledHeight
            };

            DrawTexturePro(
                context->spBullets[i].texture,
                (Rectangle){ 0, 0, 
                            context->spBullets[i].texture.width,
                            context->spBullets[i].texture.height },
                destRec,
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }
    }
}

void DrawEnemyBullets(GameContext *context) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->enemyBullets[i].active) {
            // Calculate scaled dimensions
            float scaledWidth = context->enemyBullets[i].texture.width * 3.0f;
            float scaledHeight = context->enemyBullets[i].texture.height * 3.0f;
            
            // Center the scaled sprite on the bullet's position
            Rectangle destRec = {
                context->enemyBullets[i].rec.x - (scaledWidth - context->enemyBullets[i].rec.width) / 2,
                context->enemyBullets[i].rec.y - (scaledHeight - context->enemyBullets[i].rec.height) / 2,
                scaledWidth,
                scaledHeight
            };

            DrawTexturePro(
                context->enemyBullets[i].texture,
                (Rectangle){ 0, 0, 
                            context->enemyBullets[i].texture.width,
                            context->enemyBullets[i].texture.height },
                destRec,
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }
    }
    for (int i = 0; i < MAX_BULLETS; i++) {

        if (context->eliteEnemyBullets[i].active)
        {
            // Calculate scaled dimensions
            float scaledWidth = context->eliteEnemyBullets[i].texture.width * 3.0f;
            float scaledHeight = context->eliteEnemyBullets[i].texture.height * 3.0f;
                
            // Center the scaled sprite on the bullet's position
            Rectangle destRec = {
                context->eliteEnemyBullets[i].rec.x - (scaledWidth - context->eliteEnemyBullets[i].rec.width) / 2,
                context->eliteEnemyBullets[i].rec.y - (scaledHeight - context->eliteEnemyBullets[i].rec.height) / 2,
                scaledWidth,
                scaledHeight
            };

            if (context->eliteEnemyBullets[i].active) {
                DrawTexturePro(
                    context->eliteEnemyBullets[i].texture,
                    (Rectangle){ 0, 0, 
                                context->eliteEnemyBullets[i].texture.width,
                                context->eliteEnemyBullets[i].texture.height },
                    destRec,
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );
            }
        }
    }
}

void UnloadBulletTextures(GameContext *context) {
    // We only need to unload one instance since all bullets share the same texture
    if (context->enemyBullets[0].texture.id != 0) {
        UnloadTexture(context->enemyBullets[0].texture);
    }
}

int GetHighScore(const char *filename, int difficulty) {
    FILE *file = fopen(filename, "r"); // Open the file in read mode
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return 0;
    }

    int easyScore = 0, mediumScore = 0, hardScore = 0;
    fscanf(file, "EASY %d\n", &easyScore);
    fscanf(file, "MEDIUM %d\n", &mediumScore);
    fscanf(file, "HARD %d\n", &hardScore);
    fclose(file);

    if (difficulty == 1) {
        return easyScore;
    } else if (difficulty == 2) {
        return mediumScore;
    } else if (difficulty == 3) {
        return hardScore;
    } else {
        return 0;
    }
}

GameState MainMenu(GameContext *context){

    if (IsKeyDown(KEY_LEFT) && context->player.rec.x > 0) context->player.rec.x -= 4;
    if (IsKeyDown(KEY_RIGHT) && context->player.rec.x < 600 - context->player.rec.width) context->player.rec.x += 4;

    if (IsKeyDown(KEY_RIGHT)) {
        context->player.sprite.state = SHIP_RIGHT;
    } else if (IsKeyDown(KEY_LEFT)) {
        context->player.sprite.state = SHIP_LEFT;
    } else {
        context->player.sprite.state = SHIP_IDLE;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        FireBullet(context);
    }

    UpdateBullets(context);
    DrawBullets(context);

     // Define circular buttons (planets)
    Vector2 startPlanetCenter = { 250, 350 };
    Vector2 quitPlanetCenter = { 450, 400 };
    float startPlanetRadius = 150;
    float quitPlanetRadius = 40;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->bullets[i].active) {
            Vector2 bulletCenter = {
                context->bullets[i].rec.x + context->bullets[i].rec.width/2,
                context->bullets[i].rec.y + context->bullets[i].rec.height/2
            };
            
            // Check collision with start planet
            if (CheckCollisionCircles(bulletCenter, 5, startPlanetCenter, startPlanetRadius)) {
                context->bullets[i].active = false;
                printf("CHOOSE DIFFICULTY");
                return DIFFICULTY;
            }
            
            // Check collision with quit planet
            if (CheckCollisionCircles(bulletCenter, 5, quitPlanetCenter, quitPlanetRadius)) {
                context->bullets[i].active = false;
                CloseWindow();
            }
        }
    }

    for (int i = 0; i < MAX_STARS; i++) {
        context->stars[i].position.y += context->stars[i].speed;
        
        // Reset star position when it goes off screen
        if (context->stars[i].position.y > GetScreenHeight()) {
            context->stars[i].position.y = 0;
            context->stars[i].position.x = GetRandomValue(0, GetScreenWidth());
        }
    }

    Vector2 mousePosition = GetMousePosition();
    Rectangle StartButton = { 200, 250, 200, 50 };
    Rectangle QuitButton = { 200, 350, 200, 50 };

    for (int i = 0; i < MAX_STARS; i++) {
        Color starColor = { 
            255, 
            255, 
            255, 
            (unsigned char)(context->stars[i].brightness * 255)
        };
        DrawCircleV(context->stars[i].position, context->stars[i].radius, starColor);
    }

    float titleScale = 2.0f;
    const char* title = "SPACE SHOOTER";
    int titleWidth = MeasureText(title, 30 * titleScale);
    Vector2 titlePos = { 
        (GetScreenWidth() - titleWidth) / 2,
        80
    };

    DrawText(title, titlePos.x + 2, titlePos.y + 2, 30 * titleScale, Fade(BLUE, 0.5f));
    DrawText(title, titlePos.x - 2, titlePos.y - 2, 30 * titleScale, Fade(BLUE, 0.5f));
    DrawText(title, titlePos.x, titlePos.y, 30 * titleScale, WHITE);

    DrawCircleGradient(startPlanetCenter.x, startPlanetCenter.y, startPlanetRadius, BLUE, DARKBLUE);
    DrawCircleLines(startPlanetCenter.x, startPlanetCenter.y, startPlanetRadius + 5, SKYBLUE);
    DrawCircleLines(startPlanetCenter.x, startPlanetCenter.y, startPlanetRadius + 10, Fade(SKYBLUE, 0.5f));

    DrawCircleGradient(quitPlanetCenter.x, quitPlanetCenter.y, quitPlanetRadius, RED, MAROON);
    DrawCircleLines(quitPlanetCenter.x, quitPlanetCenter.y, quitPlanetRadius + 5, PINK);
    DrawCircleLines(quitPlanetCenter.x, quitPlanetCenter.y, quitPlanetRadius + 10, Fade(PINK, 0.5f));

    const char* startText = "START";
    const char* quitText = "QUIT";
    int startTextWidth = MeasureText(startText, 50);
    int quitTextWidth = MeasureText(quitText, 20);
    
    DrawText(startText, 
        startPlanetCenter.x - startTextWidth/2, 
        startPlanetCenter.y - 20, 
        50, WHITE);
    
    DrawText(quitText, 
        quitPlanetCenter.x - quitTextWidth/2, 
        quitPlanetCenter.y - 10, 
        20, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, StartButton)) {
            printf("CHOOSE DIFFICULTY");
            return DIFFICULTY;
        } else if (CheckCollisionPointRec(mousePosition, QuitButton)) {
            CloseWindow();
        }
    }
    
    DrawAnimatedSpaceship(context->player.rec, &context->player.sprite);

    return MENU;
}

GameState ShowDifficulty(GameContext *context) {
    // Keep ship movement controls
    if (IsKeyDown(KEY_LEFT) && context->player.rec.x > 0) context->player.rec.x -= 4;
    if (IsKeyDown(KEY_RIGHT) && context->player.rec.x < 600 - context->player.rec.width) context->player.rec.x += 4;

    if (IsKeyDown(KEY_RIGHT)) {
        context->player.sprite.state = SHIP_RIGHT;
    } else if (IsKeyDown(KEY_LEFT)) {
        context->player.sprite.state = SHIP_LEFT;
    } else {
        context->player.sprite.state = SHIP_IDLE;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        FireBullet(context);
    }

    UpdateBullets(context);


    // Define planet positions and sizes (swapped medium and hard positions)
    Vector2 easyPlanetCenter = { 150, 300 };
    Vector2 hardPlanetCenter = { 450, 300 };    // Swapped with medium
    Vector2 mediumPlanetCenter = { 300, 300 };  // Swapped with hard
    float planetRadius = 50;

    // Retrieve high scores
    const char *filename = "scores.txt";
    int EasyHighScore = GetHighScore(filename, 1);
    int MediumHighScore = GetHighScore(filename, 2);
    int HardHighScore = GetHighScore(filename, 3);

    // Convert high scores to minutes and seconds
    int easyMinutes = EasyHighScore / 60;
    int easySeconds = EasyHighScore % 60;
    int mediumMinutes = MediumHighScore / 60;
    int mediumSeconds = MediumHighScore % 60;
    int hardMinutes = HardHighScore / 60;
    int hardSeconds = HardHighScore % 60;

    // Display high scores above the respective planets
    char easyScoreText[20];
    sprintf(easyScoreText, "Record: %02d:%02d", easyMinutes, easySeconds);
    DrawText(easyScoreText, easyPlanetCenter.x - MeasureText(easyScoreText, 20) / 2, easyPlanetCenter.y - planetRadius - 70, 20, WHITE);

    char mediumScoreText[20];
    sprintf(mediumScoreText, "Record: %02d:%02d", mediumMinutes, mediumSeconds);
    DrawText(mediumScoreText, mediumPlanetCenter.x - MeasureText(mediumScoreText, 20) / 2, mediumPlanetCenter.y - planetRadius - 30, 20, WHITE);

    char hardScoreText[20];
    sprintf(hardScoreText, "Record: %02d:%02d", hardMinutes, hardSeconds);
    DrawText(hardScoreText, hardPlanetCenter.x - MeasureText(hardScoreText, 20) / 2, hardPlanetCenter.y - planetRadius - 70, 20, WHITE);

    // Check bullet collisions with planets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (context->bullets[i].active) {
            Vector2 bulletCenter = {
                context->bullets[i].rec.x + context->bullets[i].rec.width/2,
                context->bullets[i].rec.y + context->bullets[i].rec.height/2
            };
            
            // Check collision with each planet
            if (CheckCollisionCircles(bulletCenter, 5, easyPlanetCenter, planetRadius)) {
                context->bullets[i].active = false;
                context->player.difficulty = EASY;
                ReinitializeGame(context);
                InitializeEnemies(context);
                return GAMEPLAY;
            }
            
            if (CheckCollisionCircles(bulletCenter, 5, mediumPlanetCenter, planetRadius)) {
                context->bullets[i].active = false;
                context->player.difficulty = MEDIUM;
                ReinitializeGame(context);
                InitializeEnemies(context);
                return GAMEPLAY;
            }

            if (CheckCollisionCircles(bulletCenter, 5, hardPlanetCenter, planetRadius)) {
                context->bullets[i].active = false;
                context->player.difficulty = HARD;
                ReinitializeGame(context);
                InitializeEnemies(context);
                return GAMEPLAY;
            }
        }
    }

    // Update stars
    for (int i = 0; i < MAX_STARS; i++) {
        context->stars[i].position.y += context->stars[i].speed;
        if (context->stars[i].position.y > GetScreenHeight()) {
            context->stars[i].position.y = 0;
            context->stars[i].position.x = GetRandomValue(0, GetScreenWidth());
        }
    }

    // Draw stars
    for (int i = 0; i < MAX_STARS; i++) {
        Color starColor = { 
            255, 
            255, 
            255, 
            (unsigned char)(context->stars[i].brightness * 255)
        };
        DrawCircleV(context->stars[i].position, context->stars[i].radius, starColor);
    }

    // Draw title
    const char* title = "SELECT DIFFICULTY";
    int titleWidth = MeasureText(title, 40);
    DrawText(title, (GetScreenWidth() - titleWidth)/2, 100, 40, WHITE);

    // Draw planets with different colors and effects
    // Easy planet (Green)
    DrawCircleGradient(easyPlanetCenter.x, easyPlanetCenter.y, planetRadius, GREEN, DARKGREEN);
    DrawCircleLines(easyPlanetCenter.x, easyPlanetCenter.y, planetRadius + 5, LIME);
    DrawCircleLines(easyPlanetCenter.x, easyPlanetCenter.y, planetRadius + 10, Fade(LIME, 0.5f));
    
    // Medium planet (Yellow) - now at bottom
    DrawCircleGradient(mediumPlanetCenter.x, mediumPlanetCenter.y, planetRadius, YELLOW, ORANGE);
    DrawCircleLines(mediumPlanetCenter.x, mediumPlanetCenter.y, planetRadius + 5, GOLD);
    DrawCircleLines(mediumPlanetCenter.x, mediumPlanetCenter.y, planetRadius + 10, Fade(GOLD, 0.5f));
    
    // Hard planet (Red) - now on right
    DrawCircleGradient(hardPlanetCenter.x, hardPlanetCenter.y, planetRadius, RED, MAROON);
    DrawCircleLines(hardPlanetCenter.x, hardPlanetCenter.y, planetRadius + 5, PINK);
    DrawCircleLines(hardPlanetCenter.x, hardPlanetCenter.y, planetRadius + 10, Fade(PINK, 0.5f));

    // Draw text labels
    DrawText("EASY", easyPlanetCenter.x - MeasureText("EASY", 20)/2, 
             easyPlanetCenter.y - 10, 20, WHITE);
    DrawText("MEDIUM", mediumPlanetCenter.x - MeasureText("MEDIUM", 20)/2, 
             mediumPlanetCenter.y - 10, 20, WHITE);
    DrawText("HARD", hardPlanetCenter.x - MeasureText("HARD", 20)/2, 
             hardPlanetCenter.y - 10, 20, WHITE);

    // Draw bullets and ship
    DrawBullets(context);
    DrawAnimatedSpaceship(context->player.rec, &context->player.sprite);

    return DIFFICULTY;
}

GameState GameOver(){
    Vector2 mousePosition = GetMousePosition();
    Rectangle RestartButton = { 200, 250, 200, 50 };
    Rectangle MenuButton = { 200, 350, 200, 50 };

    DrawText("GAME OVER", 185, 150, 30, RED);

    int RestartTextWidth = MeasureText("Restart", 20);
    int RestartTextX = RestartButton.x + (RestartButton.width - RestartTextWidth) / 2;
    int RestartTextY = RestartButton.y + (RestartButton.height - 20) / 2;

    DrawRectangleRec(RestartButton, LIGHTGRAY);
    DrawText("RESTART", RestartTextX, RestartTextY, 20, BLACK);

    // Calculate and draw Medium button
    int MenuTextWidth = MeasureText("Menu", 20);
    int MenuTextX = MenuButton.x + (MenuButton.width - MenuTextWidth) / 2;
    int MenuTextY = MenuButton.y + (MenuButton.height - 20) / 2;

    DrawRectangleRec(MenuButton, LIGHTGRAY);
    DrawText("MENU", MenuTextX, MenuTextY, 20, BLACK);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, RestartButton)) {
            printf("Restarting Game");
            return DIFFICULTY;
            
        } else if (CheckCollisionPointRec(mousePosition, MenuButton)) {
            printf("Returning To Menu");
            return MENU;
        }
    }
    return  GAMEOVER;
}

typedef struct {
    Buff buff1;
    Buff buff2;
    Buff buff3;
    bool buffsSelected;
} BuffSelection;

GameState ChooseBuff(Buff *buff1, Buff *buff2, Buff *buff3, GameContext *context) {

    // Check for 'R' key press to regenerate buffs
    if (IsKeyPressed(KEY_R)) {
        GenerateNewBuffs(buff1, buff2, buff3); // Update buffs
        printf("\nBuffs changed");
    }

    // Get mouse position for button interaction
    Vector2 mousePosition = GetMousePosition();
    Rectangle button1 = { 150, 200, 300, 150 };
    Rectangle button2 = { 150, 400, 300, 150 };
    Rectangle button3 = { 150, 600, 300, 150 };

    // Draw difficulty title
    DrawText("Select Buff", 155, 150, 30, DARKBLUE);

    // Define padding
    const int PADDING = 10;

    // Draw outer and inner rectangles for each button
    Rectangle button1Inner = { 
        button1.x + PADDING, 
        button1.y + PADDING + 30, 
        button1.width - (PADDING * 2), 
        button1.height - (PADDING * 2) - 30
    };

    Rectangle button2Inner = { 
        button2.x + PADDING, 
        button2.y + PADDING + 30, 
        button2.width - (PADDING * 2), 
        button2.height - (PADDING * 2) - 30
    };

    Rectangle button3Inner = { 
        button3.x + PADDING, 
        button3.y + PADDING + 30, 
        button3.width - (PADDING * 2), 
        button3.height - (PADDING * 2) - 30
    };

    // Draw the buttons with inner rectangles
    DrawRectangleRec(button1, buff1->color);
    DrawRectangleRec(button1Inner, LIGHTGRAY);

    DrawRectangleRec(button2, buff2->color);
    DrawRectangleRec(button2Inner, LIGHTGRAY);

    DrawRectangleRec(button3, buff3->color);
    DrawRectangleRec(button3Inner, LIGHTGRAY);

    // Draw the text on top of the inner rectangles
    int button1TextWidth = MeasureText(buff1->name, 20);
    int button1TextX = button1.x + (button1.width - button1TextWidth) / 2;
    int button1TextY = button1.y + (button1.height - 120) / 2;
    DrawText(buff1->name, button1TextX, button1TextY, 20, BLACK);
    int button1DescWidth = MeasureText(buff1->description, 20);
    int button1DescX = button1Inner.x + (button1Inner.width - button1DescWidth) / 2;
    DrawText(buff1->description, button1DescX, button1TextY + 60, 20, BLACK);

    int button2TextWidth = MeasureText(buff2->name, 20);
    int button2TextX = button2.x + (button2.width - button2TextWidth) / 2;
    int button2TextY = button2.y + (button2.height - 120) / 2;
    DrawText(buff2->name, button2TextX, button2TextY, 20, BLACK);
    int button2DescWidth = MeasureText(buff2->description, 20);
    int button2DescX = button2Inner.x + (button2Inner.width - button2DescWidth) / 2;
    DrawText(buff2->description, button2DescX, button2TextY + 60, 20, BLACK);

    int button3TextWidth = MeasureText(buff3->name, 20);
    int button3TextX = button3.x + (button3.width - button3TextWidth) / 2;
    int button3TextY = button3.y + (button3.height - 120) / 2;
    DrawText(buff3->name, button3TextX, button3TextY, 20, BLACK);
    int button3DescWidth = MeasureText(buff3->description, 20);
    int button3DescX = button3Inner.x + (button3Inner.width - button3DescWidth) / 2;
    DrawText(buff3->description, button3DescX, button3TextY + 60, 20, BLACK);

    // Check for mouse clicks
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, button1)) {
            printf("Buff Selected: %s\n", buff1->name);
            SelectBuff(*buff1, context);
            GenerateNewBuffs(buff1, buff2, buff3);
            return GAMEPLAY;

        } else if (CheckCollisionPointRec(mousePosition, button2)) {
            printf("Buff Selected: %s\n", buff2->name);
            SelectBuff(*buff2, context);
            GenerateNewBuffs(buff1, buff2, buff3);
            return GAMEPLAY;

        } else if (CheckCollisionPointRec(mousePosition, button3)) {
            printf("Buff Selected: %s\n", buff3->name);
            SelectBuff(*buff3, context);
            GenerateNewBuffs(buff1, buff2, buff3);
            return GAMEPLAY;
        }
    }

    return SELECTBUFF;
}

float CalculateTotalEnemyHealth(GameContext *context) {
    float totalHealth = 0.0f;
    
    // Sum up regular enemies health
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->enemies[i].active) {
            totalHealth += context->enemies[i].hp;
        }
    }
    
    // Sum up elite enemies health
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->eliteEnemies[i].active) {
            totalHealth += context->eliteEnemies[i].hp;
        }
    }
    
    // Sum up boss health
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (context->Boss[i].active) {
            totalHealth += context->Boss[i].hp;
        }
    }
    
    return (totalHealth / context->maxStageHealth) * 100.0f; // Return percentage
}

void DrawEnemyHealthBar(GameContext *context) {
    float percentage = CalculateTotalEnemyHealth(context);
    
    // Define health bar dimensions
    int barWidth = 500;
    int barHeight = 30;
    int xPos = (GetScreenWidth() - barWidth) / 2;
    int yPos = GetScreenHeight() - 50; // 50 pixels from bottom
    
    // Draw background (empty bar)
    DrawRectangle(xPos, yPos, barWidth, barHeight, DARKGRAY);
    
    // Draw filled portion
    int fillWidth = (int)((percentage / 100.0f) * barWidth);
    DrawRectangle(xPos, yPos, fillWidth, barHeight, RED);
    
    // Draw border
    DrawRectangleLinesEx((Rectangle){xPos, yPos, barWidth, barHeight}, 2, WHITE);
    
    // Draw percentage text
    char text[32];
    sprintf(text, "Enemy Health: %.1f%%", percentage);
    int textWidth = MeasureText(text, 20);
    DrawText(text, xPos + (barWidth - textWidth) / 2, yPos + 5, 20, WHITE);
}

int GetGameTimer(GameContext *context) {
    if (context->isGameTimerRunning) {
        context->elapsedTime = GetTime() - context->gameStartTime;
        int minutes = (int)(context->elapsedTime / 60);
        int seconds = (int)(context->elapsedTime) % 60;
        
        char timerText[20];
        sprintf(timerText, "Time: %02d:%02d", minutes, seconds);
        DrawText(timerText, 30, 100, 20, WHITE);
    }

    return context->elapsedTime;
}


GameState Gameplay(float deltaTime, GameContext *context) {
    
    const char *filename = "scores.txt";
    GetGameTimer(context);

    // Game logic here
    if (IsKeyDown(KEY_LEFT) && context->player.rec.x > 10) context->player.rec.x -= context->player.speed;
    if (IsKeyDown(KEY_RIGHT) && context->player.rec.x < 600 - context->player.rec.width) context->player.rec.x += context->player.speed;

    if (IsKeyDown(KEY_RIGHT)) {
        context->player.sprite.state = SHIP_RIGHT;
    } else if (IsKeyDown(KEY_LEFT)) {
        context->player.sprite.state = SHIP_LEFT;
    } else {
        context->player.sprite.state = SHIP_IDLE;
    }

    if (IsKeyPressed(KEY_SPACE) && context->player.stamina == 1.0f) {
        context->player.stamina -= 1.0f;
        FireBullet(context);
    }

    if (IsKeyPressed(KEY_LEFT_SHIFT) && context->player.energy >= context->player.energyCost) {
        context->player.energy -= context->player.energyCost;
        FireSpBullet(context);
    }

    //check if player died
    if (context->player.HP  <= 0) {
        return GAMEOVER;
    }

    // Update fire cooldown

    int EasyHighScore = GetHighScore(filename, 1);
    int MediumHighScore = GetHighScore(filename, 2);
    int HardHighScore = GetHighScore(filename, 3);
    if (IsKeyPressed(KEY_T)) {
        printf("Easy Highscore: %d\n", EasyHighScore);
        printf("Medium Highscore: %d\n", MediumHighScore);
        printf("Hard Highscore: %d\n", HardHighScore);
    }
    if (AllEnemiesDestroyed(context)) {
    context->player.stageIndex++; // Increment the stage index

        if (context->player.difficulty == EASY) {
            // Check if there are more stages in easy mode
            if (context->player.stageIndex < sizeof(context->stages.easy) / sizeof(context->stages.easy[0])) {
                RecoverShield(&context->player);
                InitializeEnemies(context);
                return SELECTBUFF; // Return to buff selection state
            } else {
                //Write in the file the elapsed time for easy difficulty

                int CurrentScore = GetGameTimer(context);
                if (CurrentScore < EasyHighScore) {
                    //Write in the file the new highscore
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "EASY %d\n", CurrentScore);
                    fprintf(file, "MEDIUM %d\n", MediumHighScore);
                    fprintf(file, "HARD %d\n", HardHighScore);
                    printf("New Highscore: %d\n", CurrentScore);
                    fclose(file);
                }
                return VICTORY; // Victory state
            }
        } else if (context->player.difficulty == MEDIUM) {
            // Check if there are more stages in medium mode
            if (context->player.stageIndex < 6) {
                RecoverShield(&context->player);
                InitializeEnemies(context);
                return SELECTBUFF; // Return to buff selection state
            } else {
                //Write in the file the elapsed time for easy difficulty
                int CurrentScore = GetGameTimer(context);
                if (CurrentScore < MediumHighScore) {
                    //Write in the file the new highscore
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "EASY %d\n", EasyHighScore);
                    fprintf(file, "MEDIUM %d\n", CurrentScore);
                    fprintf(file, "HARD %d\n", HardHighScore);
                    printf("New Highscore: %d\n", CurrentScore);
                    fclose(file);
                }
                return VICTORY; // Victory state
            }
        } else if (context->player.difficulty == HARD) {
            if (context->player.stageIndex < 10) {
                RecoverShield(&context->player);
                InitializeEnemies(context);
                return SELECTBUFF;
            } else {
                //Write in the file the elapsed time for easy difficulty
                int CurrentScore = GetGameTimer(context);
                if (CurrentScore < HardHighScore) {
                    //Write in the file the new highscore
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "EASY %d\n", EasyHighScore);
                    fprintf(file, "MEDIUM %d\n", MediumHighScore);
                    fprintf(file, "HARD %d\n", CurrentScore);
                    printf("New Highscore: %d\n", CurrentScore);
                    fclose(file);
                }
                return VICTORY;
            }
        }
    }
    
    UpdateStars(context);
    UpdateBullets(context);
    UpdateEnemies(context);
    UpdateEnemyBullets(context);
    UpdateEliteEnemyBullets(context);
    RechargeEnergy(deltaTime, context);
    UpdateSpaceshipAnimation(&context->player.sprite);

    BeginDrawing();
    ClearBackground(BLACK);
    

    
    // Then draw everything else
    DrawStars(context);
    DrawBullets(context);
    DrawEnemies(context);
    DrawEnemyBullets(context);
    DrawAnimatedSpaceship(context->player.rec, &context->player.sprite);
    DrawPlayerStats(&context->player);
    DrawEnemyHealthBar(context);

    EndDrawing();
    
    return GAMEPLAY;
}

GameState Victory() {
    Vector2 mousePosition = GetMousePosition();
    Rectangle RestartButton = { 200, 250, 200, 50 };
    Rectangle MenuButton = { 200, 350, 200, 50 };

    DrawText("Victory!", 185, 150, 30, GOLD);

    int RestartTextWidth = MeasureText("Restart", 20);
    int RestartTextX = RestartButton.x + (RestartButton.width - RestartTextWidth) / 2;
    int RestartTextY = RestartButton.y + (RestartButton.height - 20) / 2;

    DrawRectangleRec(RestartButton, LIGHTGRAY);
    DrawText("RESTART", RestartTextX, RestartTextY, 20, BLACK);

    // Calculate and draw Medium button
    int MenuTextWidth = MeasureText("Menu", 20);
    int MenuTextX = MenuButton.x + (MenuButton.width - MenuTextWidth) / 2;
    int MenuTextY = MenuButton.y + (MenuButton.height - 20) / 2;

    DrawRectangleRec(MenuButton, LIGHTGRAY);
    DrawText("MENU", MenuTextX, MenuTextY, 20, BLACK);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, RestartButton)) {
            printf("Restarting Game");
            return DIFFICULTY;
            
        } else if (CheckCollisionPointRec(mousePosition, MenuButton)) {
            printf("Returning To Menu");
            return MENU;
        }
    }
    return  VICTORY;
}

int main(void) {    
    InitWindow(600, 950, "SPACE SHOOTER");
    SetTargetFPS(60);
    srand(time(NULL));
    float deltaTime = GetFrameTime(); // Time between frames
    int enemyCount;
    
    GameContext context = InitializeGameContext();
    Buff buff1, buff2, buff3;
    float energy = 100.0f; // Example starting energy
    float stamina = 1.0f;

    InitSpaceshipSprite(&context.player.sprite);
    InitializeBullets(&context);
    InitMenuStars(&context); 
    GenerateNewBuffs(&buff1, &buff2, &buff3);



    GameState currentScreen =  MENU;

    while (!WindowShouldClose())
    {
        GameState previousScreen = currentScreen;
        if (IsKeyDown(KEY_I)) {
            printf("\nStage: %d", context.player.stageIndex);
        }
        switch (currentScreen)
        {
        case MENU:{
            BeginDrawing();
            ClearBackground(BLACK);
            currentScreen = MainMenu(&context);
            EndDrawing();
            break;
        }

        case DIFFICULTY:{
            BeginDrawing();
            ClearBackground(BLACK);
            if (previousScreen == GAMEOVER || previousScreen == VICTORY) {
            ReinitializeGame(&context);
            }
            currentScreen = ShowDifficulty(&context);
            EndDrawing();
            break;
        }

        case GAMEPLAY:{
            if (!context.isGameTimerRunning) {
                context.gameStartTime = GetTime();
                context.isGameTimerRunning = true;
            }
            deltaTime = GetFrameTime();
            currentScreen = Gameplay(deltaTime, &context);
            break;
        }

        case GAMEOVER:{
            BeginDrawing();
            ClearBackground(BLACK);
            currentScreen = GameOver();
            EndDrawing();
            break;
        }

        case SELECTBUFF:{
            BeginDrawing();
            ClearBackground(BLACK);
            currentScreen = ChooseBuff(&buff1, &buff2, &buff3, &context);
            EndDrawing();
            break;
        }

        case VICTORY:{
            BeginDrawing();
            ClearBackground(BLACK);
            currentScreen = Victory();
            EndDrawing();
            break;
        }

        default:
            break;
        }
    }
    // Add background cleanup
    UnloadBulletTextures(&context);
    UnloadSpaceshipSprites(&context.player.sprite);
    UnloadEnemySprites(context.enemies);
    CloseWindow();
    return 0;
}

