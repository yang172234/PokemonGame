#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SKILLS 4
#define NAME_LENGTH 32
#define MAX_STATUS_TURNS 5

// ========== 颜色定义 ==========
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_CYAN "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_BOLD "\033[1m"

// ========== 属性类型 ==========
typedef enum {
    TYPE_NORMAL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC,
    TYPE_ICE, TYPE_FIGHTING, TYPE_POISON, TYPE_GROUND, TYPE_FLYING,
    TYPE_PSYCHIC, TYPE_BUG, TYPE_ROCK, TYPE_GHOST, TYPE_DRAGON,
    TYPE_DARK, TYPE_STEEL, TYPE_FAIRY, TYPE_COUNT
} PokemonType;

// ========== 状态异常 ==========
typedef enum {
    STATUS_NONE, STATUS_SLEEP, STATUS_PARALYZE, STATUS_POISON, STATUS_BURN
} StatusCondition;

// ========== 技能结构 ==========
typedef struct {
    char name[NAME_LENGTH];
    int power;
    int accuracy;
    int pp;
    int max_pp;
    PokemonType type;
    int status_chance;        // 附加状态异常的概率 (0-100)
    StatusCondition status;   // 可能附加的状态
    int crit_rate;            // 暴击率 (0-100)
} Skill;

// ========== 宝可梦结构 ==========
typedef struct {
    char name[NAME_LENGTH];
    int max_hp;
    int current_hp;
    int attack;
    int defense;
    int speed;
    Skill skills[MAX_SKILLS];
    int skill_count;
    StatusCondition status;
    int status_turns;
    int level;
    int exp;
    int exp_to_next;
} Pokemon;

// ========== 战斗状态 ==========
typedef enum { BATTLE_SELECT, BATTLE_END } BattleState;
typedef enum { RESULT_P1_WIN, RESULT_P2_WIN, RESULT_DRAW } BattleResult;

typedef struct {
    Pokemon p1;
    Pokemon p2;
    int turn;
    BattleState state;
    BattleResult result;
} Battle;

// ========== 属性克制表 ==========
float type_chart[TYPE_COUNT][TYPE_COUNT] = {0};

void init_type_chart() {
    // 初始化所有为 1.0
    for (int i = 0; i < TYPE_COUNT; i++)
        for (int j = 0; j < TYPE_COUNT; j++)
            type_chart[i][j] = 1.0f;
    
    // 火克草、冰、虫、钢
    type_chart[TYPE_FIRE][TYPE_GRASS] = 2.0f;
    type_chart[TYPE_FIRE][TYPE_ICE] = 2.0f;
    type_chart[TYPE_FIRE][TYPE_BUG] = 2.0f;
    type_chart[TYPE_FIRE][TYPE_STEEL] = 2.0f;
    type_chart[TYPE_FIRE][TYPE_WATER] = 0.5f;
    type_chart[TYPE_FIRE][TYPE_ROCK] = 0.5f;
    type_chart[TYPE_FIRE][TYPE_DRAGON] = 0.5f;
    
    // 水克火、地面、岩石
    type_chart[TYPE_WATER][TYPE_FIRE] = 2.0f;
    type_chart[TYPE_WATER][TYPE_GROUND] = 2.0f;
    type_chart[TYPE_WATER][TYPE_ROCK] = 2.0f;
    type_chart[TYPE_WATER][TYPE_GRASS] = 0.5f;
    type_chart[TYPE_WATER][TYPE_DRAGON] = 0.5f;
    
    // 草克水、地面、岩石
    type_chart[TYPE_GRASS][TYPE_WATER] = 2.0f;
    type_chart[TYPE_GRASS][TYPE_GROUND] = 2.0f;
    type_chart[TYPE_GRASS][TYPE_ROCK] = 2.0f;
    type_chart[TYPE_GRASS][TYPE_FIRE] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_GRASS] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_POISON] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_FLYING] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_BUG] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_DRAGON] = 0.5f;
    type_chart[TYPE_GRASS][TYPE_STEEL] = 0.5f;
    
    // 电克水、飞行
    type_chart[TYPE_ELECTRIC][TYPE_WATER] = 2.0f;
    type_chart[TYPE_ELECTRIC][TYPE_FLYING] = 2.0f;
    type_chart[TYPE_ELECTRIC][TYPE_GRASS] = 0.5f;
    type_chart[TYPE_ELECTRIC][TYPE_ELECTRIC] = 0.5f;
    type_chart[TYPE_ELECTRIC][TYPE_DRAGON] = 0.5f;
    type_chart[TYPE_ELECTRIC][TYPE_GROUND] = 0.0f;
    
    // 冰克草、地面、飞行、龙
    type_chart[TYPE_ICE][TYPE_GRASS] = 2.0f;
    type_chart[TYPE_ICE][TYPE_GROUND] = 2.0f;
    type_chart[TYPE_ICE][TYPE_FLYING] = 2.0f;
    type_chart[TYPE_ICE][TYPE_DRAGON] = 2.0f;
    type_chart[TYPE_ICE][TYPE_FIRE] = 0.5f;
    type_chart[TYPE_ICE][TYPE_ICE] = 0.5f;
    type_chart[TYPE_ICE][TYPE_STEEL] = 0.5f;
    
    // 格斗克一般、冰、岩石、恶、钢
    type_chart[TYPE_FIGHTING][TYPE_NORMAL] = 2.0f;
    type_chart[TYPE_FIGHTING][TYPE_ICE] = 2.0f;
    type_chart[TYPE_FIGHTING][TYPE_ROCK] = 2.0f;
    type_chart[TYPE_FIGHTING][TYPE_DARK] = 2.0f;
    type_chart[TYPE_FIGHTING][TYPE_STEEL] = 2.0f;
    type_chart[TYPE_FIGHTING][TYPE_POISON] = 0.5f;
    type_chart[TYPE_FIGHTING][TYPE_FLYING] = 0.5f;
    type_chart[TYPE_FIGHTING][TYPE_PSYCHIC] = 0.5f;
    type_chart[TYPE_FIGHTING][TYPE_BUG] = 0.5f;
    type_chart[TYPE_FIGHTING][TYPE_GHOST] = 0.0f;
    
    // 毒克草、妖精
    type_chart[TYPE_POISON][TYPE_GRASS] = 2.0f;
    type_chart[TYPE_POISON][TYPE_FAIRY] = 2.0f;
    type_chart[TYPE_POISON][TYPE_POISON] = 0.5f;
    type_chart[TYPE_POISON][TYPE_GROUND] = 0.5f;
    type_chart[TYPE_POISON][TYPE_ROCK] = 0.5f;
    type_chart[TYPE_POISON][TYPE_GHOST] = 0.5f;
    type_chart[TYPE_POISON][TYPE_STEEL] = 0.0f;
    
    // 地面克火、电、毒、岩石、钢
    type_chart[TYPE_GROUND][TYPE_FIRE] = 2.0f;
    type_chart[TYPE_GROUND][TYPE_ELECTRIC] = 2.0f;
    type_chart[TYPE_GROUND][TYPE_POISON] = 2.0f;
    type_chart[TYPE_GROUND][TYPE_ROCK] = 2.0f;
    type_chart[TYPE_GROUND][TYPE_STEEL] = 2.0f;
    type_chart[TYPE_GROUND][TYPE_GRASS] = 0.5f;
    type_chart[TYPE_GROUND][TYPE_BUG] = 0.5f;
    type_chart[TYPE_GROUND][TYPE_FLYING] = 0.0f;
    
    // 飞行克草、格斗、虫
    type_chart[TYPE_FLYING][TYPE_GRASS] = 2.0f;
    type_chart[TYPE_FLYING][TYPE_FIGHTING] = 2.0f;
    type_chart[TYPE_FLYING][TYPE_BUG] = 2.0f;
    type_chart[TYPE_FLYING][TYPE_ELECTRIC] = 0.5f;
    type_chart[TYPE_FLYING][TYPE_ROCK] = 0.5f;
    type_chart[TYPE_FLYING][TYPE_STEEL] = 0.5f;
    
    // 超能力克格斗、毒
    type_chart[TYPE_PSYCHIC][TYPE_FIGHTING] = 2.0f;
    type_chart[TYPE_PSYCHIC][TYPE_POISON] = 2.0f;
    type_chart[TYPE_PSYCHIC][TYPE_PSYCHIC] = 0.5f;
    type_chart[TYPE_PSYCHIC][TYPE_STEEL] = 0.5f;
    type_chart[TYPE_PSYCHIC][TYPE_DARK] = 0.0f;
    
    // 虫克草、超能力、恶
    type_chart[TYPE_BUG][TYPE_GRASS] = 2.0f;
    type_chart[TYPE_BUG][TYPE_PSYCHIC] = 2.0f;
    type_chart[TYPE_BUG][TYPE_DARK] = 2.0f;
    type_chart[TYPE_BUG][TYPE_FIRE] = 0.5f;
    type_chart[TYPE_BUG][TYPE_FIGHTING] = 0.5f;
    type_chart[TYPE_BUG][TYPE_POISON] = 0.5f;
    type_chart[TYPE_BUG][TYPE_FLYING] = 0.5f;
    type_chart[TYPE_BUG][TYPE_GHOST] = 0.5f;
    type_chart[TYPE_BUG][TYPE_STEEL] = 0.5f;
    type_chart[TYPE_BUG][TYPE_FAIRY] = 0.5f;
    
    // 岩石克火、冰、飞行、虫
    type_chart[TYPE_ROCK][TYPE_FIRE] = 2.0f;
    type_chart[TYPE_ROCK][TYPE_ICE] = 2.0f;
    type_chart[TYPE_ROCK][TYPE_FLYING] = 2.0f;
    type_chart[TYPE_ROCK][TYPE_BUG] = 2.0f;
    type_chart[TYPE_ROCK][TYPE_FIGHTING] = 0.5f;
    type_chart[TYPE_ROCK][TYPE_GROUND] = 0.5f;
    type_chart[TYPE_ROCK][TYPE_STEEL] = 0.5f;
    
    // 幽灵克超能力、幽灵
    type_chart[TYPE_GHOST][TYPE_PSYCHIC] = 2.0f;
    type_chart[TYPE_GHOST][TYPE_GHOST] = 2.0f;
    type_chart[TYPE_GHOST][TYPE_DARK] = 0.5f;
    type_chart[TYPE_GHOST][TYPE_NORMAL] = 0.0f;
    
    // 龙克龙
    type_chart[TYPE_DRAGON][TYPE_DRAGON] = 2.0f;
    type_chart[TYPE_DRAGON][TYPE_STEEL] = 0.5f;
    type_chart[TYPE_DRAGON][TYPE_FAIRY] = 0.0f;
    
    // 恶克超能力、幽灵
    type_chart[TYPE_DARK][TYPE_PSYCHIC] = 2.0f;
    type_chart[TYPE_DARK][TYPE_GHOST] = 2.0f;
    type_chart[TYPE_DARK][TYPE_FIGHTING] = 0.5f;
    type_chart[TYPE_DARK][TYPE_DARK] = 0.5f;
    type_chart[TYPE_DARK][TYPE_FAIRY] = 0.5f;
    
    // 钢克冰、岩石、妖精
    type_chart[TYPE_STEEL][TYPE_ICE] = 2.0f;
    type_chart[TYPE_STEEL][TYPE_ROCK] = 2.0f;
    type_chart[TYPE_STEEL][TYPE_FAIRY] = 2.0f;
    type_chart[TYPE_STEEL][TYPE_FIRE] = 0.5f;
    type_chart[TYPE_STEEL][TYPE_WATER] = 0.5f;
    type_chart[TYPE_STEEL][TYPE_ELECTRIC] = 0.5f;
    type_chart[TYPE_STEEL][TYPE_STEEL] = 0.5f;
    
    // 妖精克格斗、龙、恶
    type_chart[TYPE_FAIRY][TYPE_FIGHTING] = 2.0f;
    type_chart[TYPE_FAIRY][TYPE_DRAGON] = 2.0f;
    type_chart[TYPE_FAIRY][TYPE_DARK] = 2.0f;
    type_chart[TYPE_FAIRY][TYPE_FIRE] = 0.5f;
    type_chart[TYPE_FAIRY][TYPE_POISON] = 0.5f;
    type_chart[TYPE_FAIRY][TYPE_STEEL] = 0.5f;
}

const char* get_type_name(PokemonType type) {
    switch (type) {
        case TYPE_NORMAL: return "普通";
        case TYPE_FIRE: return "火";
        case TYPE_WATER: return "水";
        case TYPE_GRASS: return "草";
        case TYPE_ELECTRIC: return "电";
        case TYPE_ICE: return "冰";
        case TYPE_FIGHTING: return "武";
        case TYPE_POISON: return "毒";
        case TYPE_GROUND: return "地";
        case TYPE_FLYING: return "翼";
        case TYPE_PSYCHIC: return "超能";
        case TYPE_BUG: return "虫";
        case TYPE_ROCK: return "岩";
        case TYPE_GHOST: return "幽";
        case TYPE_DRAGON: return "龙";
        case TYPE_DARK: return "恶";
        case TYPE_STEEL: return "钢";
        case TYPE_FAIRY: return "妖";
        default: return "未知";
    }
}

const char* get_status_name(StatusCondition status) {
    switch (status) {
        case STATUS_NONE: return "";
        case STATUS_SLEEP: return "睡眠";
        case STATUS_PARALYZE: return "麻痹";
        case STATUS_POISON: return "中毒";
        case STATUS_BURN: return "烧伤";
        default: return "";
    }
}

float get_type_effectiveness(PokemonType attack_type, PokemonType defend_type) {
    return type_chart[attack_type][defend_type];
}

// ========== 宝可梦初始化 ==========
void init_pokemon(Pokemon* p, const char* name, int hp, int atk, int def, int spd, int level) {
    strcpy(p->name, name);
    p->max_hp = hp;
    p->current_hp = hp;
    p->attack = atk;
    p->defense = def;
    p->speed = spd;
    p->skill_count = 0;
    p->status = STATUS_NONE;
    p->status_turns = 0;
    p->level = level;
    p->exp = 0;
    p->exp_to_next = 100;
}

void add_skill(Pokemon* p, const char* name, int power, int accuracy, int pp, 
               PokemonType type, StatusCondition status, int status_chance, int crit_rate) {
    if (p->skill_count < MAX_SKILLS) {
        strcpy(p->skills[p->skill_count].name, name);
        p->skills[p->skill_count].power = power;
        p->skills[p->skill_count].accuracy = accuracy;
        p->skills[p->skill_count].pp = pp;
        p->skills[p->skill_count].max_pp = pp;
        p->skills[p->skill_count].type = type;
        p->skills[p->skill_count].status = status;
        p->skills[p->skill_count].status_chance = status_chance;
        p->skills[p->skill_count].crit_rate = crit_rate;
        p->skill_count++;
    }
}

// ========== 显示功能 ==========
void show_hp_bar(Pokemon* p, int bar_length) {
    int filled = (p->current_hp * bar_length) / p->max_hp;
    float hp_percent = (float)p->current_hp / p->max_hp;
    
    const char* color;
    if (hp_percent > 0.5f) color = COLOR_GREEN;
    else if (hp_percent > 0.2f) color = COLOR_YELLOW;
    else color = COLOR_RED;
    
    printf(color);
    printf("[");
    for (int i = 0; i < bar_length; i++) {
        printf(i < filled ? "█" : "-");
    }
    printf("]");
    printf(COLOR_RESET);
}

void show_pokemon_info(Pokemon* p) {
    printf("\n" COLOR_BOLD "【%s】" COLOR_RESET " (Lv.%d)\n", p->name, p->level);
    printf("  HP: ");
    show_hp_bar(p, 20);
    printf(" (%d/%d)\n", p->current_hp, p->max_hp);
    printf("  攻击：%d | 防御：%d | 速度：%d\n", p->attack, p->defense, p->speed);
    if (p->status != STATUS_NONE) {
        printf("  状态：" COLOR_RED "%s" COLOR_RESET " (%d 回合)\n", get_status_name(p->status), p->status_turns);
    }
    printf("  技能:\n");
    for (int i = 0; i < p->skill_count; i++) {
        const char* color = "";
        switch (p->skills[i].type) {
            case TYPE_FIRE: color = COLOR_RED; break;
            case TYPE_WATER: color = COLOR_CYAN; break;
            case TYPE_GRASS: color = COLOR_GREEN; break;
            case TYPE_ELECTRIC: color = COLOR_YELLOW; break;
            case TYPE_ICE: color = COLOR_CYAN; break;
            case TYPE_PSYCHIC: color = COLOR_MAGENTA; break;
            case TYPE_DRAGON: color = COLOR_MAGENTA; break;
            case TYPE_GHOST: color = COLOR_MAGENTA; break;
            case TYPE_DARK: color = COLOR_MAGENTA; break;
            case TYPE_STEEL: color = COLOR_YELLOW; break;
            case TYPE_FLYING: color = COLOR_CYAN; break;
            case TYPE_POISON: color = COLOR_MAGENTA; break;
            default: color = COLOR_RESET;
        }
        printf("    %s%d. %s" COLOR_RESET " (威力:%d, 命中:%d%%, PP:%d/%d)\n", 
               color, i+1, p->skills[i].name, p->skills[i].power, 
               p->skills[i].accuracy, p->skills[i].pp, p->skills[i].max_pp);
    }
}

// ========== 创建宝可梦 ==========
void create_pikachu(Pokemon* p, int level) {
    init_pokemon(p, "皮卡丘", 100, 55, 40, 90, level);
    add_skill(p, "十万伏特", 40, 90, 15, TYPE_ELECTRIC, STATUS_PARALYZE, 30, 10);
    add_skill(p, "电光一闪", 30, 100, 30, TYPE_NORMAL, STATUS_NONE, 0, 25);
    add_skill(p, "铁尾", 35, 75, 15, TYPE_STEEL, STATUS_NONE, 0, 15);
    add_skill(p, "电球", 45, 85, 20, TYPE_ELECTRIC, STATUS_NONE, 0, 20);
}

void create_charizard(Pokemon* p, int level) {
    init_pokemon(p, "喷火龙", 120, 65, 55, 60, level);
    add_skill(p, "喷射火焰", 45, 90, 15, TYPE_FIRE, STATUS_BURN, 20, 10);
    add_skill(p, "龙爪", 40, 85, 15, TYPE_DRAGON, STATUS_NONE, 0, 15);
    add_skill(p, "空气斩", 35, 80, 20, TYPE_FLYING, STATUS_NONE, 0, 20);
    add_skill(p, "闪焰冲锋", 55, 85, 10, TYPE_FIRE, STATUS_NONE, 0, 25);
}

void create_blastoise(Pokemon* p, int level) {
    init_pokemon(p, "水箭龟", 130, 50, 70, 45, level);
    add_skill(p, "水炮", 50, 80, 10, TYPE_WATER, STATUS_NONE, 0, 10);
    add_skill(p, "咬住", 30, 100, 25, TYPE_DARK, STATUS_NONE, 0, 15);
    add_skill(p, "火箭头槌", 35, 95, 15, TYPE_NORMAL, STATUS_NONE, 0, 20);
    add_skill(p, "冰冻光束", 45, 85, 12, TYPE_ICE, STATUS_NONE, 0, 10);
}

void create_venusaur(Pokemon* p, int level) {
    init_pokemon(p, "妙蛙花", 140, 45, 65, 40, level);
    add_skill(p, "飞叶快刀", 40, 95, 15, TYPE_GRASS, STATUS_NONE, 0, 15);
    add_skill(p, "日光束", 55, 75, 10, TYPE_GRASS, STATUS_NONE, 0, 10);
    add_skill(p, "睡眠粉", 25, 85, 15, TYPE_GRASS, STATUS_SLEEP, 75, 5);
    add_skill(p, "寄生种子", 30, 90, 15, TYPE_GRASS, STATUS_POISON, 40, 10);
}

void create_zapdos(Pokemon* p, int level) {
    init_pokemon(p, "雷电鸟", 110, 60, 50, 80, level);
    add_skill(p, "打雷", 50, 75, 10, TYPE_ELECTRIC, STATUS_PARALYZE, 40, 15);
    add_skill(p, "啄钻", 35, 95, 20, TYPE_FLYING, STATUS_NONE, 0, 20);
    add_skill(p, "电磁波", 20, 90, 20, TYPE_ELECTRIC, STATUS_PARALYZE, 80, 5);
    add_skill(p, "热风", 45, 85, 12, TYPE_FIRE, STATUS_BURN, 25, 10);
}

void create_mewtwo(Pokemon* p, int level) {
    init_pokemon(p, "超梦", 115, 75, 45, 95, level);
    add_skill(p, "精神强念", 50, 90, 15, TYPE_PSYCHIC, STATUS_NONE, 0, 10);
    add_skill(p, "暗影球", 45, 85, 15, TYPE_GHOST, STATUS_NONE, 0, 15);
    add_skill(p, "波导弹", 40, 95, 20, TYPE_FIGHTING, STATUS_NONE, 0, 20);
    add_skill(p, "自我再生", 0, 100, 10, TYPE_PSYCHIC, STATUS_NONE, 0, 5);
}

void create_gengar(Pokemon* p, int level) {
    init_pokemon(p, "耿鬼", 95, 70, 35, 100, level);
    add_skill(p, "影子球", 45, 90, 15, TYPE_GHOST, STATUS_NONE, 0, 15);
    add_skill(p, "污泥炸弹", 40, 85, 12, TYPE_POISON, STATUS_POISON, 35, 10);
    add_skill(p, "噩梦", 30, 80, 15, TYPE_GHOST, STATUS_SLEEP, 50, 5);
    add_skill(p, "暗影拳", 35, 100, 20, TYPE_GHOST, STATUS_NONE, 0, 20);
}

void create_dragonite(Pokemon* p, int level) {
    init_pokemon(p, "快龙", 135, 70, 55, 55, level);
    add_skill(p, "龙息", 40, 90, 15, TYPE_DRAGON, STATUS_PARALYZE, 30, 10);
    add_skill(p, "破坏光线", 60, 75, 8, TYPE_NORMAL, STATUS_NONE, 0, 15);
    add_skill(p, "翅膀攻击", 35, 95, 25, TYPE_FLYING, STATUS_NONE, 0, 20);
    add_skill(p, "水流尾", 45, 85, 12, TYPE_WATER, STATUS_NONE, 0, 10);
}

void create_gyarados(Pokemon* p, int level) {
    init_pokemon(p, "暴鲤龙", 125, 70, 50, 65, level);
    add_skill(p, "水炮", 50, 80, 10, TYPE_WATER, STATUS_NONE, 0, 10);
    add_skill(p, "咬碎", 40, 90, 15, TYPE_DARK, STATUS_NONE, 0, 15);
    add_skill(p, "龙尾", 35, 95, 20, TYPE_DRAGON, STATUS_NONE, 0, 20);
    add_skill(p, "冰冻牙", 35, 90, 15, TYPE_ICE, STATUS_NONE, 0, 15);
}

// ========== 选择功能 ==========
int select_pokemon() {
    int choice;
    printf("\n" COLOR_BOLD "========== 宝可梦选择 ==========" COLOR_RESET "\n");
    printf("  1. " COLOR_YELLOW "皮卡丘" COLOR_RESET " (电系 - 速度快)\n");
    printf("  2. " COLOR_RED "喷火龙" COLOR_RESET " (火/飞行 - 能力均衡)\n");
    printf("  3. " COLOR_CYAN "水箭龟" COLOR_RESET " (水系 - 防御高)\n");
    printf("  4. " COLOR_GREEN "妙蛙花" COLOR_RESET " (草系 - HP 高)\n");
    printf("  5. " COLOR_YELLOW "雷电鸟" COLOR_RESET " (电/飞行 - 攻击高)\n");
    printf("  6. " COLOR_MAGENTA "超梦" COLOR_RESET " (超能力 - 特攻型)\n");
    printf("  7. " COLOR_MAGENTA "耿鬼" COLOR_RESET " (幽灵/毒 - 速度最快)\n");
    printf("  8. " COLOR_MAGENTA "快龙" COLOR_RESET " (龙/飞行 - 能力强大)\n");
    printf("  9. " COLOR_CYAN "暴鲤龙" COLOR_RESET " (水/飞行 - 攻击型)\n");
    printf("\n");
    while (1) {
        printf(COLOR_BOLD "选择你的宝可梦 (1-9): " COLOR_RESET);
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf(COLOR_RED "无效输入!" COLOR_RESET "\n");
            continue;
        }
        if (choice >= 1 && choice <= 9) return choice;
        printf(COLOR_RED "无效输入!" COLOR_RESET "\n");
    }
}

void show_battle(Battle* b) {
    printf("\n" COLOR_BOLD "=== 第 %d 回合 ===" COLOR_RESET "\n", b->turn);
    printf(COLOR_BOLD "%s" COLOR_RESET " HP: ", b->p1.name);
    show_hp_bar(&b->p1, 25);
    printf(" (%d/%d)", b->p1.current_hp, b->p1.max_hp);
    if (b->p1.status != STATUS_NONE) printf(" [%s]", get_status_name(b->p1.status));
    printf("\n");
    
    printf(COLOR_BOLD "%s" COLOR_RESET " HP: ", b->p2.name);
    show_hp_bar(&b->p2, 25);
    printf(" (%d/%d)", b->p2.current_hp, b->p2.max_hp);
    if (b->p2.status != STATUS_NONE) printf(" [%s]", get_status_name(b->p2.status));
    printf("\n");
}

int select_skill(Pokemon* p) {
    int choice;
    printf("\n%s 的技能:\n", p->name);
    for (int i = 0; i < p->skill_count; i++) {
        const char* status_str = "";
        if (p->skills[i].status != STATUS_NONE && p->skills[i].status_chance > 0) {
            status_str = " [异常]";
        }
        printf("  %d. %s%s (威力:%d, 命中:%d%%, PP:%d/%d)\n", 
               i+1, p->skills[i].name, status_str, p->skills[i].power, 
               p->skills[i].accuracy, p->skills[i].pp, p->skills[i].max_pp);
    }
    while (1) {
        printf("选择 (1-%d): ", p->skill_count);
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf(COLOR_RED "无效输入!" COLOR_RESET "\n");
            continue;
        }
        if (choice >= 1 && choice <= p->skill_count) {
            if (p->skills[choice-1].pp <= 0) {
                printf(COLOR_RED "这个技能的 PP 已经用完了!" COLOR_RESET "\n");
                continue;
            }
            return choice - 1;
        }
        printf(COLOR_RED "无效输入!" COLOR_RESET "\n");
    }
}

// ========== 战斗计算 ==========
int check_hit(int accuracy) {
    return (rand() % 100 + 1) <= accuracy;
}

int check_crit(int crit_rate) {
    return (rand() % 100 + 1) <= crit_rate;
}

float calc_type_effectiveness(Skill* skill, Pokemon* defender) {
    // 简化：假设宝可梦有一个主属性
    PokemonType defend_type = TYPE_NORMAL;
    
    // 根据技能类型和防御方判断
    // 这里简化处理，可以根据宝可梦名字判断
    if (strstr(defender->name, "火") || strstr(defender->name, "焰")) defend_type = TYPE_FIRE;
    else if (strstr(defender->name, "水") || strstr(defender->name, "海")) defend_type = TYPE_WATER;
    else if (strstr(defender->name, "草") || strstr(defender->name, "花") || strstr(defender->name, "蛙")) defend_type = TYPE_GRASS;
    else if (strstr(defender->name, "电") || strstr(defender->name, "雷") || strstr(defender->name, " pikachu")) defend_type = TYPE_ELECTRIC;
    else if (strstr(defender->name, "冰")) defend_type = TYPE_ICE;
    else if (strstr(defender->name, "龙")) defend_type = TYPE_DRAGON;
    else if (strstr(defender->name, "飞行") || strstr(defender->name, "鸟")) defend_type = TYPE_FLYING;
    else if (strstr(defender->name, "幽灵") || strstr(defender->name, "鬼")) defend_type = TYPE_GHOST;
    else if (strstr(defender->name, "超能力") || strstr(defender->name, "梦")) defend_type = TYPE_PSYCHIC;
    else if (strstr(defender->name, "毒")) defend_type = TYPE_POISON;
    else if (strstr(defender->name, "钢") || strstr(defender->name, "铁")) defend_type = TYPE_STEEL;
    else if (strstr(defender->name, "恶")) defend_type = TYPE_DARK;
    else if (strstr(defender->name, "妖精")) defend_type = TYPE_FAIRY;
    else if (strstr(defender->name, "格斗")) defend_type = TYPE_FIGHTING;
    else if (strstr(defender->name, "地面") || strstr(defender->name, "岩")) defend_type = TYPE_GROUND;
    else if (strstr(defender->name, "虫")) defend_type = TYPE_BUG;
    
    return get_type_effectiveness(skill->type, defend_type);
}

int calc_dmg(Pokemon* atk, Pokemon* def, Skill* skill, int* is_crit, float* effectiveness) {
    *is_crit = 0;
    *effectiveness = 1.0f;

    if (!check_hit(skill->accuracy)) return 0;

    // 基础伤害计算
    float base = (float)atk->attack * skill->power / def->defense;
    
    // 属性克制
    *effectiveness = calc_type_effectiveness(skill, def);
    
    // 暴击判定
    int crit = check_crit(skill->crit_rate);
    if (crit) {
        *is_crit = 1;
        base *= 1.5f;
    }
    
    // 随机波动
    float rnd = 0.85f + (float)rand() / RAND_MAX * 0.15f;
    
    // 状态影响
    if (atk->status == STATUS_BURN) base *= 0.5f;
    
    return (int)(base * *effectiveness * rnd);
}

void apply_status(Pokemon* target, StatusCondition status, int chance) {
    if (target->status != STATUS_NONE) return;  // 已有状态
    if (rand() % 100 < chance) {
        target->status = status;
        target->status_turns = 3 + rand() % 3;  // 3-5 回合
    }
}

void handle_status_damage(Pokemon* p) {
    if (p->status == STATUS_POISON || p->status == STATUS_BURN) {
        int dmg = p->max_hp / 16;
        if (dmg < 1) dmg = 1;
        p->current_hp -= dmg;
        if (p->current_hp < 0) p->current_hp = 0;
        printf(COLOR_MAGENTA "%s 受到%s伤害 (%d)!" COLOR_RESET "\n", 
               p->name, p->status == STATUS_POISON ? "中毒" : "烧伤", dmg);
    }
}

void handle_status_turn(Pokemon* p) {
    if (p->status != STATUS_NONE) {
        p->status_turns--;
        if (p->status_turns <= 0) {
            printf("%s 从%s状态恢复了!\n", p->name, get_status_name(p->status));
            p->status = STATUS_NONE;
        }
    }
}

int is_asleep(Pokemon* p) {
    if (p->status == STATUS_SLEEP) {
        printf("%s 正在睡觉，无法行动!\n", p->name);
        handle_status_turn(p);
        return 1;
    }
    return 0;
}

void attack(Battle* b, int is_p1, int skill_idx) {
    Pokemon* atk = is_p1 ? &b->p1 : &b->p2;
    Pokemon* def = is_p1 ? &b->p2 : &b->p1;
    Skill* skill = &atk->skills[skill_idx];
    
    // PP 减少
    skill->pp--;
    
    printf("\n" COLOR_BOLD "%s 使用了 %s!" COLOR_RESET "\n", atk->name, skill->name);
    
    // 命中判定
    if (!check_hit(skill->accuracy)) {
        printf(COLOR_YELLOW "攻击未命中!" COLOR_RESET "\n");
        return;
    }
    
    // 伤害计算
    int is_crit = 0;
    float effectiveness = 1.0f;
    int dmg = calc_dmg(atk, def, skill, &is_crit, &effectiveness);
    
    if (dmg > 0) {
        // 暴击提示
        if (is_crit) {
            printf(COLOR_RED "=== 暴击! ===" COLOR_RESET "\n");
        }
        
        // 属性克制提示
        if (effectiveness > 1.5f) {
            printf(COLOR_GREEN "效果绝佳!" COLOR_RESET "\n");
        } else if (effectiveness < 0.5f && effectiveness > 0.0f) {
            printf(COLOR_YELLOW "效果不好..." COLOR_RESET "\n");
        } else if (effectiveness == 0.0f) {
            printf(COLOR_YELLOW "完全无效!" COLOR_RESET "\n");
            dmg = 0;
        }
        
        if (dmg > 0) {
            def->current_hp -= dmg;
            if (def->current_hp < 0) def->current_hp = 0;
            printf("造成 " COLOR_BOLD "%d" COLOR_RESET " 点伤害!\n", dmg);
        }
    }
    
    // 附加状态
    if (skill->status != STATUS_NONE && skill->status_chance > 0) {
        apply_status(def, skill->status, skill->status_chance);
        if (def->status != STATUS_NONE) {
            printf(COLOR_MAGENTA "%s 陷入了%s状态!" COLOR_RESET "\n", 
                   def->name, get_status_name(def->status));
        }
    }
}

BattleResult check_win(Battle* b) {
    if (b->p1.current_hp <= 0 && b->p2.current_hp <= 0) return RESULT_DRAW;
    if (b->p1.current_hp <= 0) return RESULT_P2_WIN;
    if (b->p2.current_hp <= 0) return RESULT_P1_WIN;
    return RESULT_DRAW;
}

void run_battle(Battle* b) {
    printf("\n" COLOR_BOLD "=== 战斗开始! ===" COLOR_RESET "\n");
    printf("%s 对 %s\n\n", b->p1.name, b->p2.name);
    
    show_pokemon_info(&b->p1);
    show_pokemon_info(&b->p2);
    
    while (1) {
        show_battle(b);
        
        // 玩家 1 回合
        printf("\n" COLOR_CYAN ">>> 玩家 1 回合" COLOR_RESET "\n");
        
        // 检查睡眠状态
        if (!is_asleep(&b->p1)) {
            int p1_skill = select_skill(&b->p1);
            
            // 玩家 2 回合（AI）
            printf("\n" COLOR_MAGENTA ">>> 玩家 2 回合" COLOR_RESET "\n");
            
            // AI 智能选择技能
            int p2_skill = 0;
            float best_mult = 0;
            for (int i = 0; i < b->p2.skill_count; i++) {
                if (b->p2.skills[i].pp > 0) {
                    float mult = calc_type_effectiveness(&b->p2.skills[i], &b->p1);
                    if (mult > best_mult) {
                        best_mult = mult;
                        p2_skill = i;
                    }
                }
            }
            // 如果没有克制技能，随机选择
            if (best_mult <= 1.0f) {
                p2_skill = rand() % b->p2.skill_count;
                while (b->p2.skills[p2_skill].pp <= 0) {
                    p2_skill = (p2_skill + 1) % b->p2.skill_count;
                    // 检查是否所有技能都用完
                    int all_empty = 1;
                    for (int i = 0; i < b->p2.skill_count; i++) {
                        if (b->p2.skills[i].pp > 0) {
                            all_empty = 0;
                            break;
                        }
                    }
                    if (all_empty) break;
                }
            }
            
            printf("玩家 2 选择了 %s!\n", b->p2.skills[p2_skill].name);
            
            // 攻击阶段
            printf("\n" COLOR_BOLD "--- 攻击阶段 ---" COLOR_RESET "\n");
            
            // 先手判定（考虑麻痹）
            int p1_speed = b->p1.status == STATUS_PARALYZE ? b->p1.speed / 2 : b->p1.speed;
            int p2_speed = b->p2.status == STATUS_PARALYZE ? b->p2.speed / 2 : b->p2.speed;
            
            if (p1_speed >= p2_speed) {
                // 检查玩家 2 是否睡眠
                if (!is_asleep(&b->p2)) {
                    attack(b, 1, p1_skill);
                }
                BattleResult r = check_win(b);
                if (r != RESULT_DRAW) { 
                    printf("\n" COLOR_BOLD "%s 获胜!" COLOR_RESET "\n", 
                           r == RESULT_P1_WIN ? b->p1.name : b->p2.name); 
                    return; 
                }
                if (!is_asleep(&b->p1)) {
                    attack(b, 0, p2_skill);
                }
                r = check_win(b);
                if (r != RESULT_DRAW) { 
                    printf("\n" COLOR_BOLD "%s 获胜!" COLOR_RESET "\n", 
                           r == RESULT_P1_WIN ? b->p1.name : b->p2.name); 
                    return; 
                }
            } else {
                // 检查玩家 1 是否睡眠
                if (!is_asleep(&b->p1)) {
                    attack(b, 0, p2_skill);
                }
                BattleResult r = check_win(b);
                if (r != RESULT_DRAW) { 
                    printf("\n" COLOR_BOLD "%s 获胜!" COLOR_RESET "\n", 
                           r == RESULT_P1_WIN ? b->p1.name : b->p2.name); 
                    return; 
                }
                if (!is_asleep(&b->p2)) {
                    attack(b, 1, p1_skill);
                }
                r = check_win(b);
                if (r != RESULT_DRAW) { 
                    printf("\n" COLOR_BOLD "%s 获胜!" COLOR_RESET "\n", 
                           r == RESULT_P1_WIN ? b->p1.name : b->p2.name); 
                    return; 
                }
            }
        }
        
        // 处理状态伤害
        handle_status_damage(&b->p1);
        handle_status_damage(&b->p2);
        
        // 检查状态结束
        handle_status_turn(&b->p1);
        handle_status_turn(&b->p2);
        
        // 检查状态伤害后的胜利
        BattleResult r = check_win(b);
        if (r != RESULT_DRAW) { 
            printf("\n" COLOR_BOLD "%s 获胜!" COLOR_RESET "\n", 
                   r == RESULT_P1_WIN ? b->p1.name : b->p2.name); 
            return; 
        }
        
        b->turn++;
    }
}

// 根据ID初始化宝可梦
void init_pokemon_by_id(Pokemon* p, int id, int level) {
    switch(id) {
        case 1: create_pikachu(p, level); break;
        case 2: create_charizard(p, level); break;
        case 3: create_blastoise(p, level); break;
        case 4: create_venusaur(p, level); break;
        case 5: create_zapdos(p, level); break;
        case 6: create_mewtwo(p, level); break;
        case 7: create_gengar(p, level); break;
        case 8: create_dragonite(p, level); break;
        case 9: create_gyarados(p, level); break;
    }
}

// 计算伤害（用你原有的伤害公式即可，这里给一个通用模板）
int calculate_damage(Pokemon* attacker, Pokemon* defender, Skill* skill) {
    if (rand() % 100 >= skill->accuracy) {
        return 0; // 未命中
    }
    int damage = (skill->power * attacker->attack) / defender->defense;
    damage += rand() % 5; // 随机波动
    return damage > 0 ? damage : 1; // 至少1点伤害
}

int main(int argc, char *argv[]) {
   // SetConsoleOutputCP(65001);
    //SetConsoleCP(65001);
    srand((unsigned int)time(NULL));

    if (argc < 2) {
        printf("用法：\n");
        printf("初始化：pokemon.exe init [1-9]\n");
        printf("对战：pokemon.exe fight [精灵] [招式] [玩家血量] [敌方血量]\n");
        return 1;
    }

    // ===================== 初始化（英文格式，前端能正常解析） =====================
    if (strcmp(argv[1], "init") == 0 && argc == 3) {
        int pid = atoi(argv[2]);
        Pokemon p, e;

        init_pokemon_by_id(&p, pid, 5);

        int eid;
        do { eid = rand() % 9 + 1; } while (eid == pid);
        init_pokemon_by_id(&e, eid, 5);

        printf("PNAME=%s\n", p.name);
        printf("PHP=%d\n", p.max_hp);
        printf("PATK=%d\n", p.attack);
        printf("PDEF=%d\n", p.defense);
        printf("PSPD=%d\n", p.speed);

        printf("EMYNAME=%s\n", e.name);
        printf("EMYHP=%d\n", e.max_hp);
        printf("EMYATK=%d\n", e.attack);
        printf("EMYDEF=%d\n", e.defense);
        printf("EMYSPD=%d\n", e.speed);

        for (int i = 0; i < 4; i++) {
            printf("SKILL%d=%s|威力:%d|命中:%d%%|PP:%d/%d\n",
                i+1,
                p.skills[i].name,
                p.skills[i].power,
                p.skills[i].accuracy,
                p.skills[i].pp,
                p.skills[i].max_pp);
        }
        return 0;
    }

    // ===================== 对战（英文格式，前端绝对正常） =====================
    if (strcmp(argv[1], "fight") == 0 && argc == 6) {
        int pid = atoi(argv[2]);
        int skill = atoi(argv[3]) - 1;
        int php = atoi(argv[4]);
        int ehp = atoi(argv[5]);

        Pokemon p, e;
        init_pokemon_by_id(&p, pid, 5);

        int eid;
        do { eid = rand() % 9 + 1; } while (eid == pid);
        init_pokemon_by_id(&e, eid, 5);

        // 玩家攻击
        int dmg_e = calculate_damage(&p, &e, &p.skills[skill]);
        ehp -= dmg_e;
        if (ehp < 0) ehp = 0;

        // 电脑攻击
        int ai_skill = rand() % 4;
        int dmg_p = calculate_damage(&e, &p, &e.skills[ai_skill]);
        php -= dmg_p;
        if (php < 0) php = 0;

        int over = 0;
        char win[20] = "";

        if (php <= 0 && ehp <= 0) {
            over = 1;
            strcpy(win, "平局");
        } else if (php <= 0) {
            over = 1;
            strcpy(win, "敌方胜利");
        } else if (ehp <= 0) {
            over = 1;
            strcpy(win, "玩家胜利");
        }

        // 输出前端能解析的英文格式
        printf("PATTACK=%s\n", p.skills[skill].name);
        printf("EMY_DMG=%d\n", dmg_e);
        printf("EMYATTACK=%s\n", e.skills[ai_skill].name);
        printf("P_DMG=%d\n", dmg_p);
        printf("NEW_PHP=%d\n", php);
        printf("NEW_EMYHP=%d\n", ehp);
        printf("OVER=%d\n", over);
        printf("WIN=%s\n", win);

        return 0;
    }

    printf("参数错误\n");
    return 1;
}