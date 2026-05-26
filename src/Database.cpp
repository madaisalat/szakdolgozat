#include "Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <cmath>

using namespace std;

Database::~Database() {
    for (auto const& [name, teamPtr] : teams) {
        delete teamPtr;
    }
}
Database::Database(const Database& other) {
    this->allMatches = other.allMatches;
    for (auto const& [name, originalTeamPtr] : other.teams) {
        if (originalTeamPtr != nullptr) {
            this->teams[name] = new Team(*originalTeamPtr);
        }
    }
}

Database& Database::operator=(const Database& other) {
    if (this != &other) {
        for (auto const& [name, teamPtr] : teams) {
            delete teamPtr;
        }
        teams.clear();

        this->allMatches = other.allMatches;
        for (auto const& [name, originalTeamPtr] : other.teams) {
            if (originalTeamPtr != nullptr) {
                this->teams[name] = new Team(*originalTeamPtr);
            }
        }
    }
    return *this;
}

void Database::clear() {
    for (auto const& [name, teamPtr] : teams) {
        delete teamPtr;
    }
    teams.clear();
    allMatches.clear();
}

bool Database::teamExists(string name) const {
    return teams.find(name) != teams.end();
}

void Database::calculateBaselineParameters() {
    double totalGoals = 0;
    int totalMatches = allMatches.size();

    if (totalMatches == 0) { return; }

    for (const auto& m : allMatches) {
        totalGoals += (m.homeGoals + m.awayGoals);
    }

    double leagueAverage = totalGoals / (totalMatches * 2.0);

    for (auto& [name, teamPtr] : teams) {
        if (teamPtr->getMatchesPlayed() == 0) { continue; }

        double teamAvgScored = static_cast<double>(teamPtr->getGoalsScored()) / teamPtr->getMatchesPlayed(); 
        double teamAvgConceded = static_cast<double>(teamPtr->getGoalsConceded()) / teamPtr->getMatchesPlayed();

        double attack = log(teamAvgScored / leagueAverage);
        double defense = log(teamAvgConceded / leagueAverage);

        teamPtr->setModelParameters(attack, defense);
    }
}

long Database::dateConverter(string dateStr) {
    tm tm = {};
    stringstream ss(dateStr);
    
    ss >> get_time(&tm, "%d/%m/%y");
    
    if (ss.fail()) {
        ss.clear();
        ss.str(dateStr);
        ss >> get_time(&tm, "%Y-%m-%d");
        if (ss.fail()) return 0;
    }

    if (tm.tm_year < 70) { tm.tm_year += 100; }

    return mktime(&tm);
}

Team* Database::getOrCreateTeam(string name) {
    auto it = teams.find(name);
    if (it != teams.end()) {
        return it->second;
    }
    
    Team* newTeam = new Team(name);
    teams[name] = newTeam;
    return newTeam;
}

void Database::loadFromCSV(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << endl;
        return;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        vector<string> row;

        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        if (row.size() < 10) { continue; }

        string dateStr = row[1];
        long ts = dateConverter(dateStr);

        string homeName = row[3];
        string awayName = row[4];
        
        try {
            int hg = stoi(row[5]);
            int ag = stoi(row[6]);
            int hst = stoi(row[14]);
            int ast = stoi(row[15]);
            int hthg = stoi(row[8]);
            int athg = stoi(row[9]);

            Team* homeTeam = getOrCreateTeam(homeName);
            Team* awayTeam = getOrCreateTeam(awayName);

            allMatches.emplace_back(dateStr, ts, homeName, awayName, hg, ag, hst, ast, hthg, athg);
            
            int matchIndex = static_cast<int>(allMatches.size() - 1);
            homeTeam->addMatchIndex(matchIndex, hg, ag);
            awayTeam->addMatchIndex(matchIndex, ag, hg);

            double Ra = homeTeam->getElo();
            double Rb = awayTeam->getElo();

            double Ea = 1.0 / (1.0 + pow(10.0, (Rb - Ra) / 400.0));
            double Eb = 1.0 - Ea;

            double Sa, Sb;
            if (hg > ag) { Sa = 1.0; Sb = 0.0; }
            else if (ag > hg) { Sa = 0.0; Sb = 1.0; }
            else { Sa = 0.5; Sb = 0.5; }

            double K = 20.0;
            homeTeam->setElo(Ra + K * (Sa - Ea));
            awayTeam->setElo(Rb + K * (Sb - Eb));
        } catch (const exception& e) {
            cerr << "Error parsing match data (Date: " << dateStr << ", Teams: " << homeName << " vs " << awayName << "): " << e.what() << endl;
            continue;
        }
    }

    file.close();
    cout << "Succesfully loaded " << allMatches.size() << " Matches!" << endl;

    calculateBaselineParameters();
}