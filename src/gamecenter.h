
#pragma once

// 
// GameCenter関連
//

#include "co_defines.hpp"
#include <string>


namespace ngs {
namespace gamecenter {

#if (TARGET_OS_IPHONE) && defined (USE_GAMECENTER)

// iOSでGameCenter有効時
bool canUse();
void login(ViewController* view_controller);
void sendResults(const int score, const int destroyed);
void achievement(const std::string& name, const double percent, const bool demo = false);
#ifdef _DEBUG
void deleteAchievements();
#endif

void showGamecenter();
void showLeaderboard();

#else

#if (TARGET_OS_IPHONE) && !defined (USE_GAMECENTER)

// iOSでGameCenter無効時
void login(ViewController* view_controller) {}

#endif

// GameCenter無効時(iOS含む)
bool canUse() { return false; }
void sendResults(const int score, const int destroyed) {}
void achievement(const std::string& name, const double percent, const bool demo = false) {}
#ifdef _DEBUG
void deleteAchievements() {}
#endif

void showGamecenter() {}
void showLeaderboard() {}

#endif

}
}
