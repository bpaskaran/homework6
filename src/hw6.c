#include "hw6.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// helper check if char is part of word
int isWord(char c) {
    return isalnum((unsigned char)c);
}

// check prefix
int checkStart(char *w, char *p) {
    return strncmp(w, p, strlen(p)) == 0;
}

// check suffix
int checkEnd(char *w, char *s) {
    int wl = strlen(w);
    int sl = strlen(s);
    if (sl > wl) return 0;
    return strcmp(w + wl - sl, s) == 0;
}

// normal replace
void doReplace(char *line, char *search, char *rep, char *out) {
    out[0] = '\0';
    char *p;

    while ((p = strstr(line, search)) != NULL) {
        strncat(out, line, p - line);
        strcat(out, rep);
        line = p + strlen(search);
    }
    strcat(out, line);
}

// wildcard replace
void doWildcard(char *line, char *search, char *rep, char *out) {
    out[0] = '\0';

    int len = strlen(search);
    int isPre = (search[len - 1] == '*');

    char temp[MAX_LINE];

    if (isPre) {
        strncpy(temp, search, len - 1);
        temp[len - 1] = '\0';
    } else {
        strcpy(temp, search + 1);
    }

    int i = 0;
    int n = strlen(line);

    while (i < n) {

        if (isalnum((unsigned char)line[i])) {

            char word[MAX_LINE];
            int j = 0;

            while (i < n && isalnum((unsigned char)line[i])) {
                word[j++] = line[i];
                i++;
            }
            word[j] = '\0';

            int ok = 0;
            if (isPre)
                ok = checkStart(word, temp);
            else
                ok = checkEnd(word, temp);

            if (ok)
                strcat(out, rep);
            else
                strcat(out, word);
        }
        else {
            strncat(out, &line[i], 1);
            i++;
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc < 7) return MISSING_ARGUMENT;

    char *s = NULL;
    char *r = NULL;

    int start = 1;
    int end = 1000000;

    int wflag = 0;

    int sc = 0, rc = 0, lc = 0, wc = 0;

    int smiss = 0;
    int rmiss = 0;
    int linvalid = 0;

    // parse 
    for (int i = 1; i < argc - 2; i++) {

        if (strcmp(argv[i], "-s") == 0) {
            sc++;
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                smiss = 1;
            } else {
                s = argv[++i];
            }
        }

        else if (strcmp(argv[i], "-r") == 0) {
            rc++;
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                rmiss = 1;
            } else {
                r = argv[++i];
            }
        }

        else if (strcmp(argv[i], "-l") == 0) {
            lc++;
            if (i + 1 >= argc) {
                linvalid = 1;
            } else {
                char tmp[MAX_LINE];
                strcpy(tmp, argv[++i]);

                char *a = strtok(tmp, ",");
                char *b = strtok(NULL, ",");

                if (!a || !b) {
                    linvalid = 1;
                } else {
                    char *e1, *e2;
                    start = strtol(a, &e1, 10);
                    end = strtol(b, &e2, 10);

                    if (*e1 != '\0' || *e2 != '\0' ||
                        start <= 0 || end <= 0 || start > end) {
                        linvalid = 1;
                    }
                }
            }
        }

        else if (strcmp(argv[i], "-w") == 0) {
            wc++;
            wflag = 1;
        }
    }

    // duplicates
    if (sc > 1 || rc > 1 || lc > 1 || wc > 1)
        return DUPLICATE_ARGUMENT;

    FILE *in = fopen(argv[argc - 2], "r");
    if (!in) return INPUT_FILE_MISSING;

    FILE *out = fopen(argv[argc - 1], "w");
    if (!out) {
        fclose(in);
        return OUTPUT_FILE_UNWRITABLE;
    }

    if (smiss || !s) {
        fclose(in); fclose(out);
        return S_ARGUMENT_MISSING;
    }

    if (rmiss || !r) {
        fclose(in); fclose(out);
        return R_ARGUMENT_MISSING;
    }

    if (linvalid) {
        fclose(in); fclose(out);
        return L_ARGUMENT_INVALID;
    }

    // wildcard check
    if (wflag) {
        int stars = 0;
        for (int i = 0; s[i]; i++) {
            if (s[i] == '*') stars++;
        }

        if (stars != 1 || (s[0] == '*' && s[strlen(s) - 1] == '*')) {
            fclose(in); fclose(out);
            return WILDCARD_INVALID;
        }
    }

    char line[MAX_LINE];
    char outbuf[MAX_LINE * 10];

    int lineNum = 1;

    while (fgets(line, sizeof(line), in)) {

        if (lineNum >= start && lineNum <= end) {

            if (wflag)
                doWildcard(line, s, r, outbuf);
            else
                doReplace(line, s, r, outbuf);

            fputs(outbuf, out);
        }
        else {
            fputs(line, out);
        }

        lineNum++;
    }

    fclose(in);
    fclose(out);

    return 0;
}