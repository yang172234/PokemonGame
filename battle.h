#ifndef BATTLE_H  
#define BATTLE_H  
  
#include "pokemon.h\"  
  
typedef enum {  
    BATTLE_SELECT,  
    BATTLE_EXECUTE,  
    BATTLE_CHECK,  
    BATTLE_END  
} BattleState;  
  
typedef enum {  
    RESULT_PLAYER1_WIN,  
    RESULT_PLAYER2_WIN,  
    RESULT_DRAW  
} BattleResult;  
  
typedef struct {  
    Pokemon player1;  
    Pokemon player2;  
    int current_turn;  
    BattleState state;  
    BattleResult result;  
} Battle;  
  
void init_battle(Battle* b, Pokemon* p1, Pokemon* p2);  
void show_battle_info(const Battle* b);  
int select_skill(const Pokemon* p);  
void execute_attack(Battle* b, int attacker, int skill_index);  
int calculate_damage(const Pokemon* attacker, const Pokemon* defender, const Skill* skill);  
int check_hit(int accuracy);  
void check_winner(Battle* b);  
void run_battle(Battle* b);  
  
#endif 
