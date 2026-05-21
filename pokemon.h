#ifndef POKEMON_H  
#define POKEMON_H  
  
#define MAX_SKILLS 4  
#define NAME_LENGTH 32  
  
typedef struct {  
    char name[NAME_LENGTH];  
    int power;  
    int accuracy;  
    char type[16];  
} Skill; 
  
typedef struct {  
    char name[NAME_LENGTH];  
    int max_hp;  
    int current_hp;  
    int attack;  
    int defense;  
    int speed;  
    Skill skills[MAX_SKILLS];  
    int skill_count;  
    int is_alive;  
} Pokemon; 
  
void init_pokemon(Pokemon* p, const char* name, int hp, int atk, int def, int spd);  
void add_skill(Pokemon* p, const char* name, int power, int accuracy, const char* type);  
void show_pokemon_info(const Pokemon* p);  
void show_hp_bar(const Pokemon* p, int bar_length);  
  
#endif 
