#ifndef MATCH_H
#define MATCH_H

#include <string>

class Match {
public:
    std::string date;
    long timestamp;
    std::string homeTeamName;
    std::string awayTeamName;

    int homeGoals;
    int awayGoals;
    int homeShotsOnTarget;
    int awayShotsOnTarget;
    int homeHalfTimeGoals;
    int awayHalfTimeGoals;

    Match(std::string d, long ts, std::string home, std::string away, 
          int hg, int ag, int hst, int ast, int hthg, int athg);
};

#endif