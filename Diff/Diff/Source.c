#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define ARGC_ERROR 1
#define TOOMANYFILES_ERROR 2
#define CONFLICTING_OUTPUT_OPTIONS 3

#define MAXSTRINGS 1024
#define MAXPARAS 4096

#define HASHLEN 200

#include "para.h"
#include "util.h"
#include "Header.h"

char buf[BUFLEN];
char* strings1[MAXSTRINGS], * strings2[MAXSTRINGS];
int showversion = 0, showbrief = 0, ignorecase = 0, report_identical = 0, showsidebyside = 0;
int showleftcolumn = 0, showunified = 0, showcontext = 0, suppresscommon = 0, diffnormal = 0;

int count1 = 0, count2 = 0, different = 0;


void loadfiles(const char* filename1, const char* filename2) {
    memset(buf, 0, sizeof(buf));
    memset(strings1, 0, sizeof(strings1));
    memset(strings2, 0, sizeof(strings2));

    FILE* fin1 = openfile(filename1, "r");
    FILE* fin2 = openfile(filename2, "r");

    while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = _strdup(buf); }  fclose(fin1);
    while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = _strdup(buf); }  fclose(fin2);

    if (count1 != count2) { different = 1;  return; }
    for (int i = 0, j = 0; i < count1 && j < count2; ++i, ++j) {
        if (strcmp(strings1[i], strings2[j]) != 0) { different = 1;  return; }
    }
}

void print_option(const char* name, int value) { printf("%17s: %s\n", name, yesorno(value)); }

void diff_output_conflict_error(void) {
    fprintf(stderr, "diff: conflicting output style options\n");
    fprintf(stderr, "diff: Try `diff --help' for more information.)\n");
    exit(CONFLICTING_OUTPUT_OPTIONS);
}

void setoption(const char* arg, const char* s, const char* t, int* value) {
    if ((strcmp(arg, s) == 0) || ((t != NULL && strcmp(arg, t) == 0))) {
        *value = 1;
    }
}

void showoptions(const char* file1, const char* file2) {
    printf("diff options...\n");
    print_option("diffnormal", diffnormal);
    print_option("show_version", showversion);
    print_option("show_brief", showbrief);
    print_option("ignorecase", ignorecase);
    print_option("report_identical", report_identical);
    print_option("show_sidebyside", showsidebyside);
    print_option("show_leftcolumn", showleftcolumn);
    print_option("suppresscommon", suppresscommon);
    print_option("showcontext", showcontext);
    print_option("show_unified", showunified);

    printf("file1: %s,  file2: %s\n\n\n", file1, file2);

    printline();
}


void init_options_files(int argc, const char* argv[]) {
    int cnt = 0;
    const char* files[2] = { NULL, NULL };

    while (argc-- > 0) {
        const char* arg = *argv;
        setoption(arg, "-v", "--version", &showversion);
        setoption(arg, "-q", "--brief", &showbrief);
        setoption(arg, "-i", "--ignore-case", &ignorecase);
        setoption(arg, "-s", "--report-identical-files", &report_identical);
        setoption(arg, "--normal", NULL, &diffnormal);
        setoption(arg, "-y", "--side-by-side", &showsidebyside);
        setoption(arg, "--left-column", NULL, &showleftcolumn);
        setoption(arg, "--suppress-common-lines", NULL, &suppresscommon);
        setoption(arg, "-c", "--context", &showcontext);
        setoption(arg, "-u", "showunified", &showunified);
        if (arg[0] != '-') {
            if (cnt == 2) {
                fprintf(stderr, "apologies, this version of diff only handles two files\n");
                fprintf(stderr, "Usage: ./diff [options] file1 file2\n");
                exit(TOOMANYFILES_ERROR);
            }
            else { files[cnt++] = arg; }
        }
       // ++argv;   // DEBUG only;  move increment up to top of switch at release

    }

    if (!showcontext && !showunified && !showsidebyside && !showleftcolumn) {
        diffnormal = 1;
    }

    if (showversion) { version();  exit(0); }

    if (((showsidebyside || showleftcolumn) && (diffnormal || showcontext || showunified)) ||
        (showcontext && showunified) || (diffnormal && (showcontext || showunified))) {

        diff_output_conflict_error();
    }

    showoptions(files[0], files[1]);
    loadfiles(files[0], files[1]);

    if (report_identical && !different) { printf("The files are identical.\n\n");   exit(0); }

    if (showbrief && different) { printf("The files are different.\n\n");   exit(0); }

    if (showbrief && !different) { exit(0); }

    if (showcontext == 1) {
        time_t timer;
        char buffer[26];
        struct tm* tm_info;
        timer = time(NULL);
        tm_info = localtime(&timer);
        printf("*** %s", files[0]);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("*** %s", files[1]);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        puts(buffer);
        printline();
    }

    if (showunified)
    {
        time_t timer;
        char buffer[26];
        struct tm* tm_info;
        timer = time(NULL);
        tm_info = localtime(&timer);
        printf("--- %s", files[0]);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("+++ %s", files[1]);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        puts(buffer);
        printline();
    }

}


int main(int argc, const char* argv[]) {
    init_options_files(--argc, ++argv);

   // para_printfile(strings1, count1, printleft);
   // para_printfile(strings2, count2, printright);

    if (diffnormal == 1 || showsidebyside == 1) { default_(); }

    if (showleftcolumn == 1) { leftcolumn(); }
       
    if (suppresscommon == 1) { suppress(); }

    if (showcontext == 1) { context(); }

    if (showunified == 1) { unify(); }

    if (ignorecase == 1) { ignore(); }

    return 0;
}

void version()
{
  printf("$ diff --version \ndiff(GNU diffutils) 3.5\n Packaged by Cygwin(3.5 - 2)\n Copyright(C) 2016 Free Software Foundation, Inc.\n License GPLv3 + : GNU GPL version 3 or later .\n This is free software : you are free to change and redistribute it.\n There is NO WARRANTY, to the extent permitted by law.");
}

void default_()
{
    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
    int foundmatch = 0;

    para* qlast = q;
    while (p != NULL) {
        qlast = q;
        foundmatch = 0;
        while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
            q = para_next(q);
        }
        q = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {
                para_print(q, printright);
                q = para_next(q);
                qlast = q;
            }
            para_print(q, printboth);
            p = para_next(p);
            q = para_next(q);
        }
        else {
            para_print(p, printleft);
            p = para_next(p);
        }
    }
    while (q != NULL) {
        para_print(q, printright);
        q = para_next(q);
    }

}

void leftcolumn()
{
    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
    int foundmatch = 0;

    para* qlast = q;
    while (p != NULL) {
        qlast = q;
        foundmatch = 0;
        while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
            q = para_next(q);
        }
        q = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {
                para_print(q, printright);
                q = para_next(q);
                qlast = q;
            }
            para_print(p, printleft2);
            p = para_next(p);
        }
        else {
            para_print(p, printleft);
            p = para_next(p);
        }
    }
    while (q != NULL) {
        para_print(q, printright);
        q = para_next(q);
    }
}

void suppress()
{
    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
    int foundmatch = 0;

    para* qlast = q;
    while (p != NULL) {
        qlast = q;
        foundmatch = 0;
        while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
            q = para_next(q);
        }
        q = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {
                para_print(q, printright);
                q = para_next(q);
                qlast = q;
            }
        }
        else {
            para_print(p, printleft);
            p = para_next(p);
        }
    }
    while (q != NULL) {
        para_print(q, printright);
        q = para_next(q);
    }

}


void context()
{
    printf("*** 1,%d ****", count1);
    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
    int foundmatch = 0;

    para* qlast = p;
    while (p != NULL) {
        foundmatch = 0;
        while (p != NULL && (foundmatch = para_equal(p, q)) == 0) {
            p = para_next(p);
        }
        p = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) { }
            para_print(p, printleft);
            p = para_next(p);
            qlast = p;
        }
        else {
            para_print(p, printleft3);
            p = para_next(p);
        }
    }
    printf("--- 1,%d ----", count2);
    while (p != NULL) {
        qlast = q;
        foundmatch = 0;
        while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
            q = para_next(q);
        }
        q = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {}
            para_print(q, printright);
            q = para_next(q);
        }
        else {
            para_print(q, printright3);
            q = para_next(q);
        }
    }
}

void unify()
{
    printf("@@ -1,%d +1,%d @@", count1, count2);
    para* p = para_first(strings1, count1);
    para* q = para_first(strings2, count2);
    int foundmatch = 0;

    para* qlast = p;
    while (p != NULL) {

        foundmatch = 0;
        while (p != NULL && (foundmatch = para_equal(p, q)) == 0) {
            p = para_next(p);
        }
        p = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {
                para_print(q, printleft);
                p = para_next(p);
                qlast = p;
            }
            para_print(p, printleft);
            p = para_next(p);
        }
        else {
            para_print(p, printleft3);
            p = para_next(p);
        }
    }
    while (q != NULL) {
        para_print(q, printright3);
        q = para_next(q);
    }

}

void ignore()
{
    para* p = tolower(para_first(strings1, count1));
    para* q = tolower(para_first(strings2, count2));
    int foundmatch = 0;

    para* qlast = q;
    while (p != NULL) {
        qlast = q;
        foundmatch = 0;
        while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
            q = para_next(q);
        }
        q = qlast;

        if (foundmatch) {
            while ((foundmatch = para_equal(p, q)) == 0) {
                para_print(q, printright);
                q = para_next(q);
                qlast = q;
            }
            para_print(q, printboth);
            p = para_next(p);
            q = para_next(q);
        }
        else {
            para_print(p, printleft);
            p = para_next(p);
        }
    }
    while (q != NULL) {
        para_print(q, printright);
        q = para_next(q);
    }

}