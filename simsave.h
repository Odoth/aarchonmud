/*
  Tools to handle buffered simultanious player saves
  by Henning Koehler <koehlerh@in.tum.de>
*/

#ifndef SIMSAVE_H
#define SIMSAVE_H

/* player files kept in memory
 */
extern MEMFILE *player_quit_list;
extern MEMFILE *player_save_list;
extern MEMFILE *box_mf_list;
extern MEMFILE *remort_memfile;

void handle_player_save();
void force_full_save();
void final_player_save();
void sim_save_to_mem();
bool load_char_obj( DESCRIPTOR_DATA *d, const char *name );
void quit_save_char_obj( CHAR_DATA *ch );
bool remove_from_quit_list( const char *name );
bool remove_from_save_list( const char *name );
bool remove_from_box_list( const char *name );
bool memfile_in_list( const char *filename, MEMFILE *list );
void mem_sim_save_other();
void sim_save_other();
int unlink_pfile( const char *filename );

#endif
