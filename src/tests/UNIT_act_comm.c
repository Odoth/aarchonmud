#include <stddef.h>
#include <time.h>
#include <string.h>
#include "CuTest.h"
#include "../merc.h"


void Test_parse_walk(CuTest *tc)
{
    {
        char argument[] = "";
        char cmdbuf[MAX_PROTOCOL_BUFFER];
        char errbuf[256];
        
        int rc = parse_walk(argument, cmdbuf, sizeof(cmdbuf), errbuf, sizeof(errbuf));

        const char * const exp = "";

        CuAssertIntEquals(tc, strlen(exp), rc);
        CuAssertStrEquals(tc, exp, cmdbuf);
    }

    {
        char argument[] = "hello,world";
        char cmdbuf[MAX_PROTOCOL_BUFFER];
        char errbuf[256];
        
        int rc = parse_walk(argument, cmdbuf, sizeof(cmdbuf), errbuf, sizeof(errbuf));

        const char * const exp = "hello\nworld\n";

        CuAssertIntEquals(tc, strlen(exp), rc);
        CuAssertStrEquals(tc, exp, cmdbuf);
    }

    {
        char argument[] = "hello , world";
        char cmdbuf[MAX_PROTOCOL_BUFFER];
        char errbuf[256];
        
        int rc = parse_walk(argument, cmdbuf, sizeof(cmdbuf), errbuf, sizeof(errbuf));

        const char * const exp = "hello\nworld\n";

        CuAssertIntEquals(tc, strlen(exp), rc);
        CuAssertStrEquals(tc, exp, cmdbuf);
    }

    {
        char argument[] = "tell_self_El_Cid_to_Ctuoac_Warrens_have_a_boat_with_you, 2w, 11s, 2e, 2s, e, d, tell_self_Crevice_Goblin_Cave_with_Kaarvik_then_Goblin_Lair, nw, 4n, nw, n, ne, e, se, s, 3e, se, say_at_setna, ne, e, say_dark_river, e, ne, say_Cave_entrance";
        char cmdbuf[MAX_PROTOCOL_BUFFER];
        char errbuf[256];
        
        int rc = parse_walk(argument, cmdbuf, sizeof(cmdbuf), errbuf, sizeof(errbuf));

        const char * const exp = "tell self El Cid to Ctuoac Warrens have a boat with you\n"
            "w\nw\n"
            "s\ns\ns\ns\ns\ns\ns\ns\ns\ns\ns\n"
            "e\ne\n" "s\ns\n" "e\n" "d\n"
            "tell self Crevice Goblin Cave with Kaarvik then Goblin Lair\n"
            "nw\nn\nn\nn\nn\nnw\nn\nne\ne\nse\ns\ne\ne\ne\nse\n"
            "say at setna\n" "ne\ne\n" "say dark river\n" "e\nne\n"
            "say Cave entrance\n";

        CuAssertIntEquals(tc, strlen(exp), rc);
        CuAssertStrEquals(tc, exp, cmdbuf);
    }

    {
        // overflow
        char argument[] = "2hello , 4world";
        char cmdbuf[10];
        char errbuf[256];
        
        int rc = parse_walk(argument, cmdbuf, sizeof(cmdbuf), errbuf, sizeof(errbuf));

        CuAssertIntEquals(tc, -1, rc);
        CuAssertStrEquals(tc, "", cmdbuf);
    }
}