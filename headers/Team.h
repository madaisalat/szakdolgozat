#ifndef TEAM_H
#define TEAM_H

#include <string>
#include <vector>

class Team {
private:
    std::string name;
    double eloRating;
    double attackStrength;
    double defenseStrength;
    int goalsScored;
    int goalsConceded;
    int matchesPlayed;
    std::vector<int> matchHistory;

public:
    Team(std::string teamName, double initialElo = 1500.0);

    std::string getName() const;
    double getElo() const;
    double getAttack() const;
    double getDefense() const;
    int getGoalsScored() const;
    int getGoalsConceded() const;
    int getMatchesPlayed() const;
    const std::vector<int>& getMatchIndices() const;

    void addMatchIndex(int index, int scored, int conceded);
    void setElo(double newElo);
    void setModelParameters(double attack, double defense);
    void printStats() const;
};

#endif