#ifndef LUA_MAIN_H
#define LUA_MAIN_H

#include <stddef.h>

#define ENV_TABLE_NAME "envtbl"
#define INTERP_TABLE_NAME "interptbl"

/* Names of some functions declared on the lua side */
#define GETSCRIPT_FUNCTION "GetScript"
#define SAVETABLE_FUNCTION "SaveTable"
#define LOADTABLE_FUNCTION "LoadTable"
#define TPRINTSTR_FUNCTION "glob_tprintstr"

#define RUNDELAY_VNUM -1
#define LOADSCRIPT_VNUM 0
#define MAX_LUA_SECURITY 9
#define SEC_NOSCRIPT 99

struct lua_State;
struct descriptor_data;
struct char_data;

struct luaref;
typedef struct luaref LUAREF;

void open_lua(void);
void check_lua_stack( void );
void update_bossachv_table( void );
void load_mudconfig( void );
const char* save_luaconfig( struct char_data *ch );
void load_luaconfig( struct char_data *ch, const char *text );
const char* save_ptitles( struct char_data *ch );
void load_ptitles( struct char_data *ch, const char *text );
void do_achievements_boss( struct char_data *ch, struct char_data *vic );
void do_achievements_boss_reward( struct char_data *ch );
void lua_con_handler( struct descriptor_data *d, const char *argument );
void load_changelog( void );
void confirm_yes_no( struct descriptor_data *d,
        void (*yes_callback)(struct char_data *, const char *),
        const char *yes_argument,
        void (*no_callback)(struct char_data *, const char *),
        const char *no_argument);

double genrand(void);

int L_delay( lua_State *LS);
int L_cancel( lua_State *LS);
const char *check_fstring( lua_State *LS, int index, size_t size);
const char *check_string( lua_State *LS, int index, size_t size);
int CallLuaWithTraceBack (lua_State *LS, const int iArguments, const int iReturn);
void stackDump (lua_State *LS);
void dump_prog(struct char_data *ch, const char *prog, bool numberlines);

int GetLuaMemoryUsage( void );
int GetLuaGameObjectCount( void );
int GetLuaEnvironmentCount( void );


extern lua_State *g_mud_LS;
extern bool       g_LuaScriptInProgress;
extern int        g_ScriptSecurity;
extern int        g_LoopCheckCounter;

LUAREF *new_luaref( void );
void free_luaref( lua_State *LS, LUAREF *ref );

void save_luaref( lua_State *LS, int index, LUAREF *ref );
void release_luaref( lua_State *LS, LUAREF *ref );
void push_luaref( lua_State *LS, LUAREF *ref );
bool is_set_luaref( LUAREF *ref );

void quest_buy_ptitle( struct char_data *ch, const char *argument);
void fix_ptitles( struct char_data *ch );

void update_lboard( int lboard_type, struct char_data *ch, int current, int increment );
void save_lboards( void );
void load_lboards( void );
void check_lboard_reset( void );

void lua_unregister_desc (struct descriptor_data *d);
bool run_lua_interpret( struct descriptor_data *d);

extern LUAREF REF_TRACEBACK;
extern LUAREF REF_TABLE_INSERT;
extern LUAREF REF_TABLE_MAXN;
extern LUAREF REF_TABLE_CONCAT;
extern LUAREF REF_STRING_FORMAT;
extern LUAREF REF_UNPACK;

void handle_arclua_assert(const char *cond, const char *func, const char *file, unsigned line);
#define ARCLUA_ASSERT(cond) \
    do { \
        if ( !(cond) ) \
        { \
            handle_arclua_assert( #cond, __func__, __FILE__, __LINE__); \
        } \
    } while(0)

#endif
