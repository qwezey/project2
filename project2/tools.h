#ifndef tools_h
#define tools_h

#include <stdio.h>
#include <sys/types.h>

#include "forkTools.h"

//struct row {
//
//    char * color;
//    char * director_name;
//    double num_critic_for_reviews;
//    double duration;
//    double director_facebook_likes;
//    double actor_3_facebook_likes;
//    char * actor_2_name;
//    double actor_1_facebook_likes;
//    double gross;
//    char * genres;
//    char * actor_1_name;
//    char * movie_title;
//    double num_voted_users;
//    double cast_total_facebook_likes;
//    char * actor_3_name;
//    double facenumber_in_poster;
//    char * plot_keywords;
//    char * movie_imdb_link;
//    double num_user_for_reviews;
//    char * language;
//    char * country;
//    char * content_rating;
//    double budget;
//    double title_year;
//    double actor_2_facebook_likes;
//    double imdb_score;
//    double aspect_ratio;
//    double movie_facebook_likes;
//};

int tokenizeRow(char * line, char ** row);
void trim (char * str);
void removeChars (char * str, unsigned long startIndex, unsigned long endIndex);
unsigned int fillTable(FILE * csvFile, char *** table, char * lines);
void printTable (FILE * stream, char *** table, unsigned int rows, unsigned int columns);
int isNumber(const char * str);
int isXBeforeY (const char * x, const char * y, int areNumbers);
int isNumericColumn(char *** table, int rows, int columnIndex);
int findCsvFiles(const char * dirPath, char * ** csvPaths, int * numFound);
int isCsv(const char * csvPath);
unsigned int lineageParser(const char * path, char * ** lineage);
char * sortedCsvPath(const char * csvPath, const char * columnHeader, const char * outputDir);
int getColumnHeaderIndex(const char * columnHeader,
                         char *** table, const unsigned int columns);
void printDirTree(FILE * output, struct sharedMem * sharedMem);
unsigned int dirSubProcessCount(pid_t dirPid, struct sharedMem * sharedMem);
void checkDir(const char * path, const char * dirType);

#endif /* tools_h */
