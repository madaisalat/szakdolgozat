#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include <map>
#include "Team.h"
#include "Match.h"

class Database {
private:
    std::vector<Match> allMatches;
    std::map<std::string, Team*> teams;

    long dateConverter(std::string dateStr);
public:
    ~Database();

    void loadFromCSV(std::string filename);
    void calculateBaselineParameters();

    bool teamExists(std::string name) const;
    Team* getOrCreateTeam(std::string name);

    const std::vector<Match>& getMatches() const { return allMatches; }
    const std::map<std::string, Team*>& getTeams() const { return teams; }
};

#endif