#ifndef LEAGUETABLE_H
#define LEAGUETABLE_H

#include <string>
#include <vector>

struct TableEntry {
    std::string teamName;
    int matchesPlayed = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int goalDifference = 0;
    int points = 0;
};

class LeagueTable {
private:
    std::vector<TableEntry> table;
    size_t getOrAddTeamIndex(const std::string& teamName);
public:
    void updateMatch(const std::string& homeTeam, const std::string& awayTeam, int homeGoals, int awayGoals);
    void sortTable();
    void displayTable() const;
};

#endif