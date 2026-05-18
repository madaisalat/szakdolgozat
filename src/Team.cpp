#include "Team.h"
#include <iostream>
#include <iomanip>

using namespace std;


Team::Team(string teamName, double initialElo) : name(teamName), eloRating(initialElo), attackStrength(1.0), defenseStrength(1.0), goalsScored(0), goalsConceded(0), matchesPlayed(0) {}

void Team::addMatchIndex(int index, int scored, int conceded) {
    matchHistory.push_back(index);
    goalsScored += scored;
    goalsConceded += conceded;
    matchesPlayed++;
}

void Team::setElo(double newElo) {
    eloRating = newElo;
}

void Team::setModelParameters(double attack, double defense) {
    attackStrength = attack;
    defenseStrength = defense;
}

string Team::getName() const { return name; }
double Team::getElo() const { return eloRating; }
double Team::getAttack() const { return attackStrength; }
double Team::getDefense() const { return defenseStrength; }
int Team::getGoalsScored() const { return goalsScored; }
int Team::getGoalsConceded() const { return goalsConceded; }
int Team::getMatchesPlayed() const { return matchesPlayed; }
const vector<int>& Team::getMatchIndices() const { return matchHistory; }

void Team::printStats() const {
    cout << left << setw(20) << name 
              << " | Elo: " << fixed << setprecision(0) << eloRating 
              << " | Goals scored: " << setw(3) << goalsScored 
              << " | Goals conceded: " << setw(3) << goalsConceded 
              << " | Matches played: " << matchesPlayed << endl;
}