#include "Match.h"

using namespace std;

Match::Match(string d, long ts, string home, string away, int hg, int ag, int hst, int ast, int hthg, int athg) {
  date = d;
  timestamp = ts;
  homeTeamName = home;
  awayTeamName = away;
  homeGoals = hg;
  awayGoals = ag;
  homeShotsOnTarget = hst;
  awayShotsOnTarget = ast;
  homeHalfTimeGoals = hthg;
  awayHalfTimeGoals = athg;
}