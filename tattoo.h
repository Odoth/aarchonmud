#ifndef TATTOO_H
#define TATTOO_H

/* merc.h
#define TATTOO_NONE -1
*/

/***************************** tattoo_list ***************************/

/* merc.h
typedef int tattoo_list[MAX_WEAR];
*/

void add_tattoo( tattoo_list tl, int loc, int ID );
void remove_tattoo( tattoo_list tl, int loc );
int get_tattoo( tattoo_list tl, int loc );
bool is_tattoo_list_empty( tattoo_list tl );

void clear_tattoos( tattoo_list tl );
char* print_tattoos( tattoo_list tl );
void bread_tattoos( RBUFFER *rbuf, tattoo_list tl );

/***************************** tattoo_data ***************************/

bool is_tattoo_loc( int loc );
char* tattoo_desc( int ID );
int tattoo_cost( int ID );
int tattoo_id( char *name );

/***************************** general *******************************/ 

void tattoo_modify_equip( CHAR_DATA *ch, int loc, bool fAdd, bool drop, bool basic );
void tattoo_modify_level( CHAR_DATA *ch, int old_level, int new_level );
void tattoo_modify_reset( CHAR_DATA *ch );
int get_tattoo_ch( CHAR_DATA *ch, int loc );
float get_tattoo_level( CHAR_DATA *ch, int loc, int level );

/***************************** do_functions **************************/

void do_tattoo( CHAR_DATA *ch, const char *argument);

#endif


