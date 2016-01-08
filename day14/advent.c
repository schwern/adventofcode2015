#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

typedef struct {
    char *name;
    int flight_speed;
    int flight_time;
    int rest_time;
} Reindeer;

static Reindeer* Reindeer_new(char *name) {
    Reindeer *reindeer = calloc(1, sizeof(Reindeer));
    reindeer->name = strdup(name);

    return reindeer;
}

static void Reindeer_destroy(Reindeer *self) {
    free(self->name);
    free(self);
}

static int Reindeer_travel(Reindeer *self, int time) {
    int cycle = self->flight_time + self->rest_time;

    assert( time > 0 );
    
    time = time % cycle;

    /* If the time is right at the end of the rest cycle */
    if( time == 0 ) {
        return 0;
    }
    else if( time <= self->flight_time ) {
        return self->flight_speed;
    }
    else {
        return 0;
    }
}

static void Reindeer_print(Reindeer *self) {
    printf("name: %s, flight_speed: %d, flight_time: %d, rest_time: %d\n",
           self->name, self->flight_speed, self->flight_time, self->rest_time
    );
}

GRegex *Blank_Line_Re;
GRegex *Line_Re;

static void init_regexes() {
    if( !Blank_Line_Re )
        Blank_Line_Re = compile_regex(
            "^ \\s* $",
            G_REGEX_OPTIMIZE | G_REGEX_EXTENDED,
            0
        );

    if( !Line_Re )
        Line_Re = compile_regex(
            "^(?<NAME>[[:alpha:]]+) can fly (?<FLIGHT_SPEED>\\d+) km/s for (?<FLIGHT_TIME>\\d+) seconds, but then must rest for (?<REST_TIME>\\d+) seconds\\.$",
            G_REGEX_OPTIMIZE,
            0
        );
}

static void free_regexes() {
    g_regex_unref(Blank_Line_Re);
    g_regex_unref(Line_Re);
}

static void read_reindeer_line( char *line, Reindeer **reindeer_p ) {
    GMatchInfo *match;
    
    if( g_regex_match(Line_Re, line, 0, &match) ) {
        char *name              = g_match_info_fetch_named(match, "NAME");
        char *flight_speed      = g_match_info_fetch_named(match, "FLIGHT_SPEED");
        char *flight_time       = g_match_info_fetch_named(match, "FLIGHT_TIME");
        char *rest_time         = g_match_info_fetch_named(match, "REST_TIME");

        Reindeer *reindeer = Reindeer_new(name);
        reindeer->flight_speed = atoi(flight_speed);
        reindeer->flight_time  = atoi(flight_time);
        reindeer->rest_time    = atoi(rest_time);

        *reindeer_p = reindeer;

        g_match_info_free(match);
        free(name);
        free(flight_speed);
        free(flight_time);
        free(rest_time);
    }
    else if( !g_regex_match(Blank_Line_Re, line, 0, NULL) ) {
        die("Unknown line '%s'", line);
    }

    return;
}

static int max_of(int things[], int num_things) {
    int max = -INT_MAX;
    for( int i = 0; i < num_things; i++ ) {
        max = MAX( max, things[i] );
    }

    return max;
}

static int race_reindeer(Reindeer **reindeers, int num_reindeer, int race_length) {
    int distances[num_reindeer];
    for( int i = 0; i < num_reindeer; i++ ) {
        distances[i] = 0;
    }
    
    for( int time = 1; time <= race_length; time++ ) {
        for( int j = 0; j < num_reindeer; j++ ) {
            distances[j] += Reindeer_travel(reindeers[j], time);
        }
    }

    return max_of(distances, num_reindeer);
}

#define NUM_TEST_REINDEER 2
char *Test_Lines[NUM_TEST_REINDEER] = {
    "Comet can fly 14 km/s for 10 seconds, but then must rest for 127 seconds.\n",
    "Dancer can fly 16 km/s for 11 seconds, but then must rest for 162 seconds.\n"
};

static void test_reindeer() {
    printf("test_reindeer\n");
    
    Reindeer *reindeers[NUM_TEST_REINDEER];
    for( int i = 0; i < NUM_TEST_REINDEER; i++ ) {
        Reindeer *reindeer = NULL;
        read_reindeer_line( Test_Lines[i], &reindeer );

        reindeers[i] = reindeer;
    }

    assert( streq(reindeers[0]->name, "Comet") );
    assert( reindeers[0]->flight_speed == 14 );
    assert( reindeers[0]->flight_time  == 10 );
    assert( reindeers[0]->rest_time    == 127 );

    assert( streq(reindeers[1]->name, "Dancer") );
    assert( reindeers[1]->flight_speed == 16 );
    assert( reindeers[1]->flight_time  == 11 );
    assert( reindeers[1]->rest_time    == 162 );

    assert( Reindeer_travel(reindeers[1], 1) == 16 );
    assert( Reindeer_travel(reindeers[1], 11) == 16 );
    assert( Reindeer_travel(reindeers[1], 12) == 0 );
    assert( Reindeer_travel(reindeers[1], 11 + 162) == 0 );
    assert( Reindeer_travel(reindeers[1], 11 + 162 + 1) == 16 );

    for( int i = 0; i < NUM_TEST_REINDEER; i++ ) {
        Reindeer_destroy(reindeers[i]);
    }
}

static void test_race_reindeer() {
    printf("test_race_reindeer\n");
    
    Reindeer *reindeers[NUM_TEST_REINDEER];
    for( int i = 0; i < NUM_TEST_REINDEER; i++ ) {
        Reindeer *reindeer = NULL;
        read_reindeer_line( Test_Lines[i], &reindeer );
        reindeers[i] = reindeer;
    }

    assert( race_reindeer(reindeers, NUM_TEST_REINDEER, 1000) == 1120 );

    for( int i = 0; i < NUM_TEST_REINDEER; i++ ) {
        Reindeer_destroy(reindeers[i]);
    }
}

static void run_tests() {
    printf("Running tests\n");

    init_regexes();
    
    test_reindeer();
    test_race_reindeer();

    free_regexes();
    
    printf("OK\n");
}

int main(int argc, char *argv[]) {
    run_tests();
}
