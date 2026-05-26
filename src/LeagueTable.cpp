#include "LeagueTable.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

size_t LeagueTable::getOrAddTeamIndex(const string& teamName) {
    for (size_t i = 0; i < table.size(); ++i) {
        if (table[i].teamName == teamName) return i;
    }
    TableEntry newTeam;
    newTeam.teamName = teamName;
    table.push_back(newTeam);
    return table.size() - 1;
}

void LeagueTable::updateMatch(const string& homeTeam, const string& awayTeam, int homeGoals, int awayGoals) {
    size_t homeIdx = getOrAddTeamIndex(homeTeam);
    size_t awayIdx = getOrAddTeamIndex(awayTeam);

    table[homeIdx].matchesPlayed++;
    table[awayIdx].matchesPlayed++;

    table[homeIdx].goalsFor += homeGoals;
    table[homeIdx].goalsAgainst += awayGoals;
    table[homeIdx].goalDifference = table[homeIdx].goalsFor - table[homeIdx].goalsAgainst;

    table[awayIdx].goalsFor += awayGoals;
    table[awayIdx].goalsAgainst += homeGoals;
    table[awayIdx].goalDifference = table[awayIdx].goalsFor - table[awayIdx].goalsAgainst;

    if (homeGoals > awayGoals) {
        table[homeIdx].wins++;
        table[homeIdx].points += 3;
        table[awayIdx].losses++;
    } else if (homeGoals < awayGoals) {
        table[awayIdx].wins++;
        table[awayIdx].points += 3;
        table[homeIdx].losses++;
    } else {
        table[homeIdx].draws++;
        table[homeIdx].points += 1;
        table[awayIdx].draws++;
        table[awayIdx].points += 1;
    }
}

void LeagueTable::sortTable() {
    sort(table.begin(), table.end(), [](const TableEntry& a, const TableEntry& b) {
        if (a.points != b.points) return a.points > b.points;
        if (a.goalDifference != b.goalDifference) return a.goalDifference > b.goalDifference;
        return a.goalsFor > b.goalsFor;
    });
}

void LeagueTable::displayTable() const {
    cout << "\n=========================================================================" << endl;
    cout << "                       SIMULATED LEAGUE TABLE" << endl;
    cout << "=========================================================================" << endl;
    cout << left << setw(4) << "Pos" << setw(22) << "Team" 
         << right << setw(5) << "Pld" << setw(5) << "W" << setw(5) << "D" << setw(5) << "L" 
         << setw(7) << "GF:GA" << setw(8) << "GD" << setw(8) << "Pts" << endl;
    cout << "------------------------------------------------------------------------" << endl;

    for (size_t i = 0; i < table.size(); ++i) {
        string gfga = to_string(table[i].goalsFor) + ":" + to_string(table[i].goalsAgainst);
        string gdStr = (table[i].goalDifference > 0 ? "+" : "") + to_string(table[i].goalDifference);
        
        cout << left << setw(4) << (i + 1) << setw(22) << table[i].teamName
             << right << setw(5) << table[i].matchesPlayed 
             << setw(5) << table[i].wins 
             << setw(5) << table[i].draws 
             << setw(5) << table[i].losses 
             << setw(7) << gfga 
             << setw(8) << gdStr 
             << setw(8) << table[i].points << endl;
    }
    cout << "=========================================================================" << endl;
}