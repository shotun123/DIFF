//
//  util.h
//  diff
//
//  Created by William McCarthy on 5/9/19.
//  Copyright © 2019 William McCarthy. All rights reserved.
//

#ifndef util_h
#define util_h

#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#define BUFLEN 256

char* yesorno(int condition);
FILE* openfile(const char* filename, const char* openflags);

void printleft(const char* left);
void printright(const char* right);
void printright3(const char* right);
void printboth(const char* left_right);
void printleft2(const char* left);
void printleft3(const char* left);

void printline(void);


#endif /* util_h */
