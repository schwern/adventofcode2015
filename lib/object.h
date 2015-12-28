#ifndef _object_h
#define _object_h

#define __(o, m, ...) o->proto->m(o, ##__VA_ARGS__)

#endif
