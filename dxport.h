typedef enum eDXPORT_rc
{
    eDXPORT_SUCCESS=0,
    /* statuses */
    eDXPORT_OPENED,
    eDXPORT_CLOSED,
} eDXPORT_rc;

eDXPORT_rc DXPORT_init();
eDXPORT_rc DXPORT_close();
eDXPORT_rc DXPORT_player_connect(const char *player_name, const char *ip, time_t timestamp);
eDXPORT_rc DXPORT_mob_kill(const char *player_name, int mob_vnum, int mob_room, time_t timestamp);
eDXPORT_rc DXPORT_status();
