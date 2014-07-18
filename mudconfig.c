#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "mudconfig.h"

bool cfg_show_exp_mult=FALSE;
bool cfg_enable_exp_mult=FALSE;
float cfg_exp_mult;
const float cfg_exp_mult_default=1;

bool cfg_show_qp_mult=FALSE;
bool cfg_enable_qp_mult=FALSE;
float cfg_qp_mult;
const float cfg_qp_mult_default=1;

char *cfg_word_of_day;
const char *cfg_word_of_day_default="bananahammock";

bool cfg_show_rolls=FALSE;

CFG_DATA_ENTRY mudconfig_table[] =
{
    { "enable_exp_mult",    CFG_BOOL,   &cfg_enable_exp_mult,   NULL }, 
    { "show_exp_mult",      CFG_BOOL,   &cfg_show_exp_mult,     NULL },
    { "exp_mult",           CFG_FLOAT,  &cfg_exp_mult,          &cfg_exp_mult_default },
    { "show_qp_mult",       CFG_BOOL,   &cfg_show_qp_mult,      NULL },
    { "enable_qp_mult",     CFG_BOOL,   &cfg_enable_qp_mult,    NULL },
    { "qp_mult",            CFG_FLOAT,  &cfg_qp_mult,           &cfg_qp_mult_default },
    { "show_rolls",         CFG_BOOL,   &cfg_show_rolls,        NULL },
    { "word_of_day",        CFG_STRING, &cfg_word_of_day,       &cfg_word_of_day_default},
    { NULL, NULL, NULL, NULL }
};
void mudconfig_init()
{
    /* set defaults, especially important for strings, others can
       really just be set in declaration, but can also be set with
       default_value fields */
    int i;
    CFG_DATA_ENTRY *en;

    for ( i=0; mudconfig_table[i].name ; i++ )
    {
        en=&mudconfig_table[i];

        if ( en->default_value )
        {
            switch(en->type)
            {
                case CFG_INT:
                {
                    *((int *)(en->value))=*((int *)(en->default_value));
                    break;
                }
                case CFG_FLOAT:
                {
                    *((float *)(en->value))=*((float *)(en->default_value));
                    break;
                }
                case CFG_BOOL:
                {
                    *((bool *)(en->value))=*((bool *)(en->default_value));
                    break;
                }
                case CFG_STRING:
                {
                    *((char **)(en->value))=str_dup( *((char **)(en->default_value)) );
                    break;
                }
            }
        }
    }

    /* double check that strings have defaults, otherwise give it one */
    for ( i=0; mudconfig_table[i].name ; i++ )
    {
        en=&mudconfig_table[i];

        if (en->type==CFG_STRING)
        {
            if ( *((char **)(en->value)) == NULL )
            {
                bugf("mudconfig_init: no default value for %s", en->name );
                *((char **)(en->value))=str_dup("");
            }
        }
    }

    return;
}    
