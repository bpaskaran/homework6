#include "hw6.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---------- helpers ----------

int is_word_char(char c) {
    return isalnum((unsigned char)c);
}

int starts_with(const char *word, const char *prefix) {
    return strncmp(word, prefix, strlen(prefix)) == 0;
}

int ends_with(const char *word, const char *suffix) {
    int wl = strlen(word);
    int sl = strlen(suffix);
    if (sl > wl) return 0;
    return strcmp(word + wl - sl, suffix) == 0;
}

// ---------- normal replace ----------

void replace_normal(char *line, char *search, char *replace, char *out) {
    out[0] = '\0';
    char *pos;

    while ((pos = strstr(line, search)) != NULL) {
        strncat(out, line, pos - line);
        strcat(out, replace);
        line = pos + strlen(search);
    }
    strcat(out, line);
}

// ---------- wildcard replace ----------

void replace_wildcard(char *line, char *search, char *replace, char *out) {
    out[0] = '\0';

    int prefix = (search[strlen(search)-1] == '*');

    char clean[256];

    if (prefix) {
        strncpy(clean, search, strlen(search)-1);
        clean[strlen(search)-1] = '\0';
    } else {
        strcpy(clean, search + 1);
    }

    int i = 0;
    int len = strlen(line);

    while (i < len) {

        if (is_word_char(line[i])) {
            char word[256];
            int j = 0;

            while (i < len && is_word_char(line[i])) {
                word[j++] = line[i++];
            }
            word[j] = '\0';

            int match = 0;
            if (prefix) match = starts_with(word, clean);
            else match = ends_with(word, clean);

            if (match) strcat(out, replace);
            else strcat(out, word);

        } else {
            strncat(out, &line[i], 1);
            i++;
        }
    }
}

// ---------- main ----------

int main(int argc, char *argv[]) {

    // 1. MISSING_ARGUMENT
    if (argc < 7) return MISSING_ARGUMENT;

    char *search = NULL;
    char *replace = NULL;
    int start = 1;
    int end = 1000000;
    int wildcard = 0;

    int s_count = 0, r_count = 0, l_count = 0, w_count = 0;

    int s_missing = 0;
    int r_missing = 0;
    int l_invalid = 0;

    // ---------- parse ----------
    for (int i = 1; i < argc - 2; i++) {

        if (strcmp(argv[i], "-s") == 0) {
            s_count++;
            if (i+1 >= argc || argv[i+1][0] == '-') {
                s_missing = 1;
            } else {
                search = argv[++i];
            }
        }

        else if (strcmp(argv[i], "-r") == 0) {
            r_count++;
            if (i+1 >= argc || argv[i+1][0] == '-') {
                r_missing = 1;
            } else {
                replace = argv[++i];
            }
        }

        else if (strcmp(argv[i], "-l") == 0) {
            l_count++;
            if (i+1 >= argc) {
                l_invalid = 1;
            } else {
                char temp[100];
                strcpy(temp, argv[++i]);

                char *a = strtok(temp, ",");
                char *b = strtok(NULL, ",");

                if (!a || !b) {
                    l_invalid = 1;
                } else {
                    char *endptr1, *endptr2;

                    start = strtol(a, &endptr1, 10);
                    end = strtol(b, &endptr2, 10);

                    if (*endptr1 != '\0' || *endptr2 != '\0' ||
                        start <= 0 || end <= 0 || start > end) {
                        l_invalid = 1;
                    }
                }
            }
        }

        else if (strcmp(argv[i], "-w") == 0) {
            w_count++;
            wildcard = 1;
        }
    }

    // 2. DUPLICATE_ARGUMENT (HIGHEST PRIORITY AFTER MISSING)
    if (s_count > 1 || r_count > 1 || l_count > 1 || w_count > 1)
        return DUPLICATE_ARGUMENT;

    // 3. INPUT_FILE_MISSING
    FILE *in = fopen(argv[argc-2], "r");
    if (!in) return INPUT_FILE_MISSING;

    // 4. OUTPUT_FILE_UNWRITABLE
    FILE *out = fopen(argv[argc-1], "w");
    if (!out) {
        fclose(in);
        return OUTPUT_FILE_UNWRITABLE;
    }

    // 5. S_ARGUMENT_MISSING
    if (s_missing || !search) {
        fclose(in); fclose(out);
        return S_ARGUMENT_MISSING;
    }

    // 6. R_ARGUMENT_MISSING
    if (r_missing || !replace) {
        fclose(in); fclose(out);
        return R_ARGUMENT_MISSING;
    }

    // 7. L_ARGUMENT_INVALID
    if (l_invalid) {
        fclose(in); fclose(out);
        return L_ARGUMENT_INVALID;
    }

    // 8. WILDCARD_INVALID
    if (wildcard) {
        int stars = 0;
        for (int i = 0; search[i]; i++)
            if (search[i] == '*') stars++;

        if (stars != 1 ||
           !(search[0] == '*' || search[strlen(search)-1] == '*')) {
            fclose(in); fclose(out);
            return WILDCARD_INVALID;
        }
    }

    // ---------- process ----------
    char line[MAX_LINE];
    char result[MAX_LINE * 4];
    int line_num = 1;

    while (fgets(line, sizeof(line), in)) {

        if (line_num >= start && line_num <= end) {

            if (wildcard)
                replace_wildcard(line, search, replace, result);
            else
                replace_normal(line, search, replace, result);

            fputs(result, out);
        }
        else {
            fputs(line, out);
        }

        line_num++;
    }

    fclose(in);
    fclose(out);

    return 0;
}