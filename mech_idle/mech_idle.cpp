#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

void radar_screen(void);
void contact_menu(const char* callsign, const char* mech);
void start_battle(const char* target_callsign);

void wait_for_space(void)
{
    int ch;
    printf("\nPress SPACE to continue...");

    while (1)
    {
        ch = _getch();   // waits for a key press
        if (ch == ' ')
            break;
    }
}

int main(void)
{
    int choice = 0;

    while (1)
    {
     
        printf("\n=== BATTLEMECH IDLE SCREEN PROTOTYPE v0.3.5===\n");
        printf("1) RADAR Screen\n");
        printf("2) BattleMech Status\n");
        printf("3) Pilot Status\n");
        printf("4) Options\n");
        printf("5) Exit\n");
        printf("Select option: ");

        if (scanf_s("%d", &choice) != 1)
        {
            // Clear invalid input
            while (getchar() != '\n');
            continue;
        }

        switch (choice)
        {
        case 1:
            system("cls");
            radar_screen();
            break;

        case 2:
            printf("\n[BATTLEMECH STATUS]\n");
            printf("SUMMONER PRIME\n");
            printf("REACTOR: ONLINE\n");
            printf("SENSORS: ONLINE\n");
            printf("WEAPONS: ONLINE\n");
            printf("ALL SYSTEMS: NOMINAL\n");
            wait_for_space();
            break;

        case 3:
            printf("\n[PILOT STATUS]\n");
            printf("Name: StCpt Jordan | Callsign: Wildcat\n");
            printf("Piloting Skill: 1  | Gunnery Skill: 1\n");
            printf("Homeworld: Sudeten | Faction: Clan Jade Falcon\n");
            printf("Bloodname: Pryde   | XP: 69/420\n");
            wait_for_space();
            break;

        case 4:
            printf("\nOptions.\n");
            printf("Options Go Here...\n");
            wait_for_space();
            break;

        case 5:
            printf("\nExiting program.\n");
            return 0;

        default:
            printf("\nInvalid selection.\n");
            break;
        }
    }

    return 0;
}

void radar_screen(void)
{
    int choice;
    printf("\n=== RADAR CONTACTS ===\n");
    printf("1) FOOFER - BUSHWACKER [BSW-X1] (-62 dBm)\n");
    printf("2) HEXW0LF - LOKI [PRIME] (-71 dBm)\n");
    printf("3) Back\n");
    printf("Select contact: ");

    if (scanf_s("%d", &choice) != 1)
    {
        while (getchar() != '\n');
        return;
    }

    switch (choice)
    {
    case 1:
        contact_menu("FOOFER", "BUSHWACKER [BSW-X1]");
        break;

    case 2:
        contact_menu("HEXW0LF", "LOKI [PRIME]");
        break;

    case 3:
        return;

    default:
        printf("Invalid selection.\n");
        break;
    }
}

void contact_menu(const char* callsign, const char* mech)
{
    int choice;
    system("cls");
    printf("\n=== CONTACT SELECTED ===\n");
    printf("%s - %s\n", callsign, mech);

    printf("\n1) Engage\n");
    printf("2) Communicate\n");
    printf("3) Back\n");
    printf("Select option: ");

    if (scanf_s("%d", &choice) != 1)
    {
        while (getchar() != '\n');
        return;
    }

    switch (choice)
    {
    case 1:
        system("cls");
        start_battle(callsign);
        wait_for_space();
        break;

    case 2:
        system("cls");
        printf("\n[COMMS]\n");
        printf("Opening COMMS channel to %s...\n", callsign);
        wait_for_space();
        break;

    case 3:
        return;

    default:
        printf("Invalid selection.\n");
        break;
    }
}
// ===================== BATTLE ENGINE (C) ===================================================================================================================

typedef enum { RANGE_MELEE = 0, RANGE_SHORT = 1, RANGE_MED = 2, RANGE_LONG = 3 } RangeBand;
typedef enum { ACT_CLOSE = 0, ACT_HOLD = 1, ACT_OPEN = 2 } RangeAction;

typedef enum {
    LOC_HEAD = 0, LOC_CT = 1, LOC_RT = 2, LOC_LT = 3, LOC_RA = 4, LOC_LA = 5, LOC_RL = 6, LOC_LL = 7, LOC_COUNT = 8
} Loc;

typedef struct {
    uint64_t state;
} Rng;

static uint64_t splitmix64(uint64_t* x)
{
    uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static void rng_seed(Rng* r, uint64_t seed) { r->state = seed; }

static uint32_t rng_u32(Rng* r)
{
    uint64_t x = r->state;
    uint64_t z = splitmix64(&x);
    r->state = x;
    return (uint32_t)(z & 0xFFFFFFFFu);
}

static int rng_int(Rng* r, int lo, int hi) // inclusive
{
    if (lo > hi) { int t = lo; lo = hi; hi = t; }
    uint32_t span = (uint32_t)(hi - lo + 1);
    return lo + (int)(rng_u32(r) % span);
}

static int roll_d6(Rng* r) { return rng_int(r, 1, 6); }
static int roll_2d6(Rng* r) { return roll_d6(r) + roll_d6(r); }

// Deterministic seed from two ids + session salt (order-independent)
static uint64_t make_seed(uint32_t idA, uint32_t idB, uint32_t salt)
{
    uint64_t lo = (idA < idB) ? idA : idB;
    uint64_t hi = (idA < idB) ? idB : idA;
    uint64_t x = (hi << 32) ^ lo ^ ((uint64_t)salt << 1);
    uint64_t tmp = x;
    return splitmix64(&tmp);
}

typedef struct {
    char name[32];
    int gunnery;   // lower is better
    int piloting;  // lower is better
} Pilot;

typedef struct {
    char name[32];
    int damage;
    int heat;
    RangeBand minRange;
    RangeBand maxRange;
} Weapon;

typedef struct {
    char chassis[32];
    int speed;      // 1..5 (base)
    int heatSinks;  // heat removed per turn
    int heat;

    // Heat status
    int shutdown;        // 0/1 : if 1, you can't move or fire this turn
    int movePenalty;     // 0.. : speed penalty from heat (recomputed each turn)

    int armor[LOC_COUNT];
    int internal[LOC_COUNT];

    Weapon hardpoints[3]; // 3 SAOs
} Mech;


typedef struct {
    RangeAction rangeAction;
    int fire[3]; // 0/1
} PlayerChoice;

// True if weapon can fire at the current range band
static int weapon_in_range(const Weapon* w, RangeBand range)
{
    return ((int)range >= (int)w->minRange) && ((int)range <= (int)w->maxRange);
}

static const char* range_name(RangeBand r)
{
    switch (r) {
    case RANGE_MELEE: return "MELEE";
    case RANGE_SHORT: return "SHORT";
    case RANGE_MED:   return "MEDIUM";
    case RANGE_LONG:  return "LONG";
    default: return "?";
    }
}

static const char* loc_name(Loc l)
{
    switch (l) {
    case LOC_HEAD: return "Head";
    case LOC_CT: return "Center Torso";
    case LOC_RT: return "Right Torso";
    case LOC_LT: return "Left Torso";
    case LOC_RA: return "Right Arm";
    case LOC_LA: return "Left Arm";
    case LOC_RL: return "Right Leg";
    case LOC_LL: return "Left Leg";
    default: return "?";
    }
}

static int range_mod(RangeBand r)
{
    switch (r) {
    case RANGE_LONG:  return 4;
    case RANGE_MED:   return 2;
    case RANGE_SHORT: return 0;
    case RANGE_MELEE: return -1;
    default: return 0;
    }
}


static int heat_to_hit_mod(int heat)
{
    if (heat <= 4) return 0;
    if (heat <= 8) return 1;
    if (heat <= 12) return 2;
    return 3;
}


static int mech_alive(const Mech* m)
{
    return (m->internal[LOC_CT] > 0) && (m->internal[LOC_HEAD] > 0);
}

static RangeBand shift_range(RangeBand cur, int delta)
{
    int r = (int)cur + delta;
    if (r < (int)RANGE_MELEE) r = (int)RANGE_MELEE;
    if (r > (int)RANGE_LONG)  r = (int)RANGE_LONG;
    return (RangeBand)r;
}


// Simple 2d6 hit location (front-ish). Replace with full BT tables later.
static Loc roll_hit_location(Rng* rng)
{
    int roll = roll_2d6(rng);
    switch (roll) {
    case 2: return LOC_HEAD;
    case 3:
    case 4: return LOC_RA;
    case 5: return LOC_RL;
    case 6: return LOC_RT;
    case 7: return LOC_CT;
    case 8: return LOC_LT;
    case 9: return LOC_LL;
    case 10:
    case 11: return LOC_LA;
    case 12: return LOC_CT;
    default: return LOC_CT;
    }
}

// Returns:
//  +1 = you got behind enemy this turn (you have rear-arc advantage)
//   0 = no rear advantage
//  -1 = enemy got behind you this turn (enemy has rear-arc advantage)
//
// Uses the margin between movement contest rolls and a small random check
// so it feels "lucky" but still skill-based.
static int determine_rear_advantage(
    int yourMoveRoll,
    int enemyMoveRoll,
    RangeAction yourAction,
    RangeAction enemyAction,
    Rng* rng)
{
    int margin = yourMoveRoll - enemyMoveRoll;

    // Only allow "behind" if the player is actually maneuvering (Close/Open)
    int youManeuver = (yourAction == ACT_CLOSE || yourAction == ACT_OPEN);
    int enManeuver = (enemyAction == ACT_CLOSE || enemyAction == ACT_OPEN);

    // Thresholds you can tune:
    // 5+ margin = possible rear shot
    // 8+ margin = more likely rear shot
    if (youManeuver && margin >= 5) {
        int d6 = roll_d6(rng);
        int needed = (margin >= 8) ? 3 : 4;  // margin 8+: 3-6 succeeds, else 4-6 succeeds
        if (d6 >= needed) return +1;
    }

    if (enManeuver && margin <= -5) {
        int d6 = roll_d6(rng);
        int needed = (margin <= -8) ? 3 : 4;
        if (d6 >= needed) return -1;
    }

    return 0;
}


static void print_mech_brief(const Pilot* p, const Mech* m)
{
    printf("%s - %s | Heat %d | HS %d | Speed %d\n",
        p->name, m->chassis, m->heat, m->heatSinks, m->speed);
    printf("  Armor:  H%d CT%d RT%d LT%d RA%d LA%d RL%d LL%d\n",
        m->armor[LOC_HEAD], m->armor[LOC_CT], m->armor[LOC_RT], m->armor[LOC_LT],
        m->armor[LOC_RA], m->armor[LOC_LA], m->armor[LOC_RL], m->armor[LOC_LL]);
    printf("  Intern: H%d CT%d RT%d LT%d RA%d LA%d RL%d LL%d\n",
        m->internal[LOC_HEAD], m->internal[LOC_CT], m->internal[LOC_RT], m->internal[LOC_LT],
        m->internal[LOC_RA], m->internal[LOC_LA], m->internal[LOC_RL], m->internal[LOC_LL]);
}

static void apply_damage(Rng* rng, Mech* def, Loc loc, int dmg)
{
    int idx = (int)loc;

    // armor
    if (def->armor[idx] > 0) {
        int take = (def->armor[idx] < dmg) ? def->armor[idx] : dmg;
        def->armor[idx] -= take;
        dmg -= take;
    }

    // internal
    if (dmg > 0) {
        def->internal[idx] -= dmg;

        // placeholder crit hook: 8+ triggers something small
        int critRoll = roll_2d6(rng);
        if (critRoll >= 8) {
            // very lightweight effects (replace with real crit tables later)
            if (loc == LOC_CT || loc == LOC_RT || loc == LOC_LT) {
                if (def->heatSinks > 0) def->heatSinks -= 1;
            }
            else if (loc == LOC_RL || loc == LOC_LL) {
                if (def->speed > 1) def->speed -= 1;
            }
            else if (loc == LOC_HEAD) {
                def->internal[LOC_HEAD] -= 1; // “pilot hit” vibe
            }
        }
    }
}

static void sink_heat(Mech* m)
{
    m->heat -= m->heatSinks;
    if (m->heat < 0) m->heat = 0;
}

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Effective speed after heat penalties and shutdown
static int mech_effective_speed(const Mech* m)
{
    if (m->shutdown) return 0;
    int eff = m->speed - m->movePenalty;
    if (eff < 1) eff = 1;
    return eff;
}

// Heat phase: apply penalties and possible shutdown/startup.
// Call this AFTER sink_heat().
static void heat_phase(Mech* m, const Pilot* p, Rng* rng)
{
    // If shutdown, attempt startup FIRST (so you can recover next turn)
    if (m->shutdown) {
        // Simple "startup" roll: 2d6 + (6 - piloting) >= 8 succeeds
        int roll = roll_2d6(rng) + (6 - p->piloting);
        if (roll >= 8) {
            m->shutdown = 0;
            printf("%s STARTUP SUCCESS (roll %d)\n", m->chassis, roll);
        }
        else {
            printf("%s STILL SHUTDOWN (roll %d)\n", m->chassis, roll);
            // While shutdown, keep move penalty based on current heat (still matters after restart)
        }
    }

    // Recompute movement penalty from current heat (tune as desired)
    // This is "badge-BT": simple tiers.
    if (m->heat >= 13)      m->movePenalty = 2;
    else if (m->heat >= 10) m->movePenalty = 1;
    else                    m->movePenalty = 0;

    // Shutdown checks (very simplified, but feels BT-ish)
    // 19+ = auto shutdown
    if (!m->shutdown) {
        if (m->heat >= 19) {
            m->shutdown = 1;
            printf("%s AUTO SHUTDOWN (heat %d)\n", m->chassis, m->heat);
        }
        else if (m->heat >= 14) {
            // chance-based shutdown: higher heat = higher chance
            // target: roll 2d6 <= (heat - 12) shuts down
            int target = clamp_int(m->heat - 12, 2, 12); // heat 14 => 2, heat 18 => 6, etc.
            int roll = roll_2d6(rng);
            if (roll <= target) {
                m->shutdown = 1;
                printf("%s SHUTDOWN! (heat %d, roll %d <= %d)\n", m->chassis, m->heat, roll, target);
            }
            else {
                printf("%s avoids shutdown (heat %d, roll %d > %d)\n", m->chassis, m->heat, roll, target);
            }
        }
    }

    // Optional: small "overheat damage" if you want consequences beyond shutdown.
    // Uncomment for extra crunch:
    /*
    if (m->heat >= 22) {
        m->internal[LOC_CT] -= 1;
        printf("%s OVERHEAT DAMAGE! CT internal -1\n", m->chassis);
    }
    */
}


static PlayerChoice read_player_choice(const Mech* m)
{
    if (m->shutdown) {
        PlayerChoice c;
        c.rangeAction = ACT_HOLD;
        c.fire[0] = c.fire[1] = c.fire[2] = 0;
        printf("\n*** YOU ARE SHUTDOWN: cannot move or fire this turn. ***\n");
        return c;
    }

    PlayerChoice c;
    int ra = 2;
    int i;

    c.rangeAction = ACT_HOLD;
    for (i = 0; i < 3; i++) c.fire[i] = 0;

    printf("\nRange Action: 1) Close  2) Hold  3) Open  > ");
    if (scanf_s("%d", &ra) != 1) {
        while (getchar() != '\n');
        ra = 2;
    }
    if (ra == 1) c.rangeAction = ACT_CLOSE;
    else if (ra == 3) c.rangeAction = ACT_OPEN;
    else c.rangeAction = ACT_HOLD;

    printf("\nSelect weapons to fire (toggle 1-3). Enter 9 for Alpha, 0 for none.\n");
    for (i = 0; i < 3; i++) {
        const Weapon* w = &m->hardpoints[i];
        printf("  %d) %s  dmg %d  heat %d  range %s-%s\n",
            i + 1, w->name, w->damage, w->heat, range_name(w->minRange), range_name(w->maxRange));
    }
    printf("  > ");

    int wsel = -1;
    if (scanf_s("%d", &wsel) != 1) {
        while (getchar() != '\n');
        wsel = 0;
    }

    if (wsel == 9) {
        c.fire[0] = c.fire[1] = c.fire[2] = 1;
    }
    else if (wsel == 0) {
        // none
    }
    else {
        // allow a single selection for now (easy UI). You can expand to multi-select later.
        if (wsel >= 1 && wsel <= 3) c.fire[wsel - 1] = 1;
    }

    return c;
}

// Simple deterministic enemy AI (replace with remote choices later)
static PlayerChoice enemy_ai_choice(const Mech* enemy, RangeBand range)
{
    if (enemy->shutdown) {
        PlayerChoice c;
        int i;
        c.rangeAction = ACT_HOLD;
        for (i = 0; i < 3; i++) c.fire[i] = 0;
        return c;
    }

    PlayerChoice c;
    int i;
    c.rangeAction = ACT_HOLD;
    for (i = 0; i < 3; i++) c.fire[i] = 0;

    // If enemy has short weapons and we're far, close; if hot, hold/open.
    if (enemy->heat >= 9) c.rangeAction = ACT_OPEN;
    else if (range == RANGE_LONG) c.rangeAction = ACT_CLOSE;
    else c.rangeAction = ACT_HOLD;

    // Fire any in-range weapons; avoid huge heat if already hot.
    for (i = 0; i < 3; i++) {
        const Weapon* w = &enemy->hardpoints[i];
        int inRange = ((int)range >= (int)w->minRange && (int)range <= (int)w->maxRange);
        int tooHot = (enemy->heat >= 8 && w->heat >= 6);
        if (inRange && !tooHot) c.fire[i] = 1;
    }

    // If none selected, pick first in-range
    if (!c.fire[0] && !c.fire[1] && !c.fire[2]) {
        for (i = 0; i < 3; i++) {
            const Weapon* w = &enemy->hardpoints[i];
            int inRange = ((int)range >= (int)w->minRange && (int)range <= (int)w->maxRange);
            if (inRange) { c.fire[i] = 1; break; }
        }
    }

    return c;
}

static Weapon W_MED_LASER(void) { Weapon w; strcpy_s(w.name, 32, "Medium Laser"); w.damage = 5; w.heat = 3; w.minRange = RANGE_SHORT; w.maxRange = RANGE_MED; return w; }
static Weapon W_PPC(void) { Weapon w; strcpy_s(w.name, 32, "PPC"); w.damage = 10; w.heat = 10; w.minRange = RANGE_MED; w.maxRange = RANGE_LONG; return w; }
static Weapon W_AC5(void) { Weapon w; strcpy_s(w.name, 32, "AC/5"); w.damage = 5; w.heat = 1; w.minRange = RANGE_SHORT; w.maxRange = RANGE_LONG; return w; }
static Weapon W_LRM10(void) { Weapon w; strcpy_s(w.name, 32, "LRM-10"); w.damage = 10; w.heat = 4; w.minRange = RANGE_MED; w.maxRange = RANGE_LONG; return w; }
static Weapon W_SRM4(void) { Weapon w; strcpy_s(w.name, 32, "SRM-4"); w.damage = 8; w.heat = 3; w.minRange = RANGE_MELEE; w.maxRange = RANGE_SHORT; return w; }

static Mech make_loki(void)
{
    Mech m;
    strcpy_s(m.chassis, 32, "SUMMONER [PRIME]");
    m.speed = 3;
    m.heatSinks = 6;
    m.heat = 0;
    m.shutdown = 0;
    m.movePenalty = 0;


    // Armor:  H CT RT LT RA LA RL LL
    int armor[LOC_COUNT] = { 3, 18, 12, 12, 8, 8, 10, 10 };
    int internal[LOC_COUNT] = { 3, 12,  8,  8, 6, 6,  7,  7 };
    memcpy(m.armor, armor, sizeof(armor));
    memcpy(m.internal, internal, sizeof(internal));

    m.hardpoints[0] = W_PPC();
    m.hardpoints[1] = W_MED_LASER();
    m.hardpoints[2] = W_SRM4();
    return m;
}

static Mech make_bushwacker(void)
{
    Mech m;
    strcpy_s(m.chassis, 32, "BUSHWACKER [BSW-X1]");
    m.speed = 2;
    m.heatSinks = 5;
    m.heat = 0;
    m.shutdown = 0;
    m.movePenalty = 0;


    int armor[LOC_COUNT] = { 3, 20, 13, 13, 9, 9, 11, 11 };
    int internal[LOC_COUNT] = { 3, 13,  9,  9, 6, 6,  8,  8 };
    memcpy(m.armor, armor, sizeof(armor));
    memcpy(m.internal, internal, sizeof(internal));

    m.hardpoints[0] = W_AC5();
    m.hardpoints[1] = W_LRM10();
    m.hardpoints[2] = W_MED_LASER();
    return m;
}

// Core duel loop entry point
void start_battle(const char* target_callsign)
{
    // In the real badge: badgeId is your unique ID; opponentId comes from BLE advertisement.
    // For now, just hash callsign into a deterministic "opponent id".
    uint32_t badgeId = 0xA11CE001u;
    uint32_t oppId = 0;
    for (size_t i = 0; i < strlen(target_callsign); i++) {
        oppId = (oppId * 131u) + (uint8_t)target_callsign[i];
    }

    uint32_t sessionSalt = 12345; // later: set at handshake
    uint64_t seed = make_seed(badgeId, oppId, sessionSalt);

    Rng rng;
    rng_seed(&rng, seed);

    Pilot you;   strcpy_s(you.name, 32, "Jordan");   you.gunnery = 2; you.piloting = 5;
    Pilot enemy; strcpy_s(enemy.name, 32, "Enemy");  enemy.gunnery = 3; enemy.piloting = 5;

    // Choose opponent chassis based on callsign (for now)
    Mech yourMech = make_loki();
    Mech enemyMech = make_bushwacker();
    if (strstr(target_callsign, "HEX") != NULL) {
        enemyMech = make_loki();
        strcpy_s(enemy.name, 32, "HEXW0LF");
    }
    else if (strstr(target_callsign, "FOO") != NULL) {
        enemyMech = make_bushwacker();
        strcpy_s(enemy.name, 32, "FOOFER");
    }

    RangeBand range = RANGE_LONG; // per your design: start at max distance
    int turn = 0;

    printf("\n=== DUEL START ===\n");
    printf("Seed: %llu\n", (unsigned long long)seed);
    printf("Start Range: %s\n", range_name(range));
    wait_for_space();

    while (mech_alive(&yourMech) && mech_alive(&enemyMech) && turn < 30) {
        turn++;

        system("cls");
        printf("=== BATTLE MODE (TURN %d) ===\n", turn);
        printf("Range: %s\n\n", range_name(range));
        print_mech_brief(&you, &yourMech);
        printf("\n");
        print_mech_brief(&enemy, &enemyMech);

        // 1) Initiative (computer rolled)
        int initYou = roll_2d6(&rng) + mech_effective_speed(&yourMech);
        int initEnemy = roll_2d6(&rng) + mech_effective_speed(&enemyMech);

        // 2) Range choices
        printf("\n--- INITIATIVE ROLLED ---\n");
        printf("You: %d   Enemy: %d\n", initYou, initEnemy);

        PlayerChoice cYou = read_player_choice(&yourMech);
        PlayerChoice cEn = enemy_ai_choice(&enemyMech, range);

        // Resolve range contest
        int desireYou = (cYou.rangeAction == ACT_CLOSE) ? -1 : (cYou.rangeAction == ACT_OPEN) ? +1 : 0;
        int desireEn = (cEn.rangeAction == ACT_CLOSE) ? -1 : (cEn.rangeAction == ACT_OPEN) ? +1 : 0;

        int rollYou = roll_2d6(&rng) + mech_effective_speed(&yourMech) + (6 - you.piloting);
        int rollEn = roll_2d6(&rng) + mech_effective_speed(&enemyMech) + (6 - enemy.piloting);


        // NEW: rear advantage check
        int rearAdv = determine_rear_advantage(rollYou, rollEn, cYou.rangeAction, cEn.rangeAction, &rng);
        int youRearShot = (rearAdv == +1);
        int enemyRearShot = (rearAdv == -1);

        if (rollYou > rollEn && desireYou != 0) range = shift_range(range, desireYou);
        else if (rollEn > rollYou && desireEn != 0) range = shift_range(range, desireEn);

        // Print rear result (optional)
        if (youRearShot) printf("MANEUVER! You slip behind the enemy (REAR ARC ADVANTAGE)\n");
        else if (enemyRearShot) printf("MANEUVER! Enemy slips behind you (ENEMY REAR ARC ADVANTAGE)\n");

        // 3) Attack resolution order
        int youFirst = (initYou >= initEnemy);

        // We'll hide dice (you can print debug later if you want)
        printf("\n--- RANGE RESOLVED ---\n");
        printf("Enemy chose: %s\n", (cEn.rangeAction == ACT_CLOSE) ? "Close" : (cEn.rangeAction == ACT_OPEN) ? "Open" : "Hold");
        printf("New Range: %s\n", range_name(range));

        // helper lambda-style via function pointer would be overkill in C; do inline blocks.

        // A function-like macro to resolve one side firing at the other:
#define RESOLVE_ATTACK(ATT_PILOT, ATT_MECH, DEF_MECH, CHOICE, ATT_NAME, DEF_NAME) \
    do { \
        int i; \
        int rMod = range_mod(range); \
        int hMod = heat_to_hit_mod((ATT_MECH).heat); \
        int defEffSpeed = mech_effective_speed(&(DEF_MECH)); \
        int targetMoveMod = (defEffSpeed >= 4) ? 2 : ((defEffSpeed >= 3) ? 1 : 0); \
        for (i = 0; i < 3; i++) { \
            if (!(CHOICE).fire[i]) continue; \
            Weapon *w = &(ATT_MECH).hardpoints[i]; \
            int inRange = weapon_in_range(w, range); \
            (ATT_MECH).heat += w->heat; \
            if (!inRange) { \
                printf("\n%s fires %s: OUT OF RANGE\n", (ATT_NAME), w->name); \
                continue; \
            } \
            /* tuned TN so LONG isn't impossible */ \
            int tn = 2 + (ATT_PILOT).gunnery + rMod + hMod + targetMoveMod; \
            int roll = roll_2d6(&rng); \
            if (roll >= tn) { \
                Loc loc = roll_hit_location(&rng); \
                printf("\n%s fires %s: HIT %s for %d\n", (ATT_NAME), w->name, loc_name(loc), w->damage); \
                apply_damage(&rng, &(DEF_MECH), loc, w->damage); \
            } else { \
                printf("\n%s fires %s: MISS\n", (ATT_NAME), w->name); \
            } \
            if (!mech_alive(&(DEF_MECH))) break; \
        } \
    } while (0)


        printf("\n--- ATTACKS ---\n");

        if (youFirst) {
            RESOLVE_ATTACK(you, yourMech, enemyMech, cYou, you.name, enemy.name);
            if (mech_alive(&enemyMech)) RESOLVE_ATTACK(enemy, enemyMech, yourMech, cEn, enemy.name, you.name);
        }
        else {
            RESOLVE_ATTACK(enemy, enemyMech, yourMech, cEn, enemy.name, you.name);
            if (mech_alive(&yourMech)) RESOLVE_ATTACK(you, yourMech, enemyMech, cYou, you.name, enemy.name);
        }

#undef RESOLVE_ATTACK

        // 4) Heat sinks
        sink_heat(&yourMech);
        sink_heat(&enemyMech);

        printf("\n--- HEAT PHASE ---\n");
        heat_phase(&yourMech, &you, &rng);
        heat_phase(&enemyMech, &enemy, &rng);

        printf("You Heat: %d | MovePenalty: %d | Shutdown: %s\n",
            yourMech.heat, yourMech.movePenalty, yourMech.shutdown ? "YES" : "NO");
        printf("Enemy Heat: %d | MovePenalty: %d | Shutdown: %s\n",
            enemyMech.heat, enemyMech.movePenalty, enemyMech.shutdown ? "YES" : "NO");

        // End turn
        wait_for_space();
    }

    system("cls");
    printf("=== DUEL END ===\n");
    if (mech_alive(&yourMech) && !mech_alive(&enemyMech)) {
        printf("VICTORY!\n");
    }
    else if (!mech_alive(&yourMech) && mech_alive(&enemyMech)) {
        printf("DEFEAT.\n");
    }
    else {
        printf("DRAW / TIMEOUT.\n");
    }
    wait_for_space();
}
