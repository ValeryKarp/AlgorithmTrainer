#include "UserProfile.h"
#include "TaskDatabase.h"
#include "RatingCalculator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;
namespace chrono = std::chrono;

namespace algotrainer {

static std::string timepoint_to_string(chrono::system_clock::time_point tp) {
    auto t = chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

static chrono::system_clock::time_point string_to_timepoint(const std::string& s) {
    std::tm tm = {};
    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail()) return chrono::system_clock::time_point::min();
    auto t = std::mktime(&tm);
    return chrono::system_clock::from_time_t(t);
}

bool UserProfile::load(const std::string& filepath) {
    profile_path = filepath;
    std::ifstream f(filepath);
    if (!f.is_open()) return false;

    json j;
    try {
        f >> j;
    } catch (...) {
        return false;
    }

    if (j.contains("solved_tasks")) {
        for (const auto& id : j["solved_tasks"])
            solved_tasks.insert(id.get<int>());
    }

    if (j.contains("last_attempt")) {
        for (const auto& [key, val] : j["last_attempt"].items()) {
            int task_id = std::stoi(key);
            last_attempt[task_id] = string_to_timepoint(val.get<std::string>());
        }
    }
    return true;
}

bool UserProfile::save() const {
    json j;

    json solved_json = json::array();
    for (int id : solved_tasks)
        solved_json.push_back(id);
    j["solved_tasks"] = solved_json;

    json last_json;
    for (const auto& [id, tp] : last_attempt)
        last_json[std::to_string(id)] = timepoint_to_string(tp);
    j["last_attempt"] = last_json;

    std::ofstream f(profile_path);
    if (!f.is_open()) return false;
    f << j.dump(4);
    return true;
}

void UserProfile::addEntry(int task_id, bool solved, const TaskDatabase& db) {
    if (solved) {
        solved_tasks.insert(task_id);
    }
    last_attempt[task_id] = chrono::system_clock::now();
    save();
}

void UserProfile::reset() {
    solved_tasks.clear();
    last_attempt.clear();
    save();
}

float UserProfile::getTopicRating(const std::string& topic, const TaskDatabase& db) const {
    return RatingCalculator::calculateTopicRating(topic, solved_tasks, db);
}

float UserProfile::getOverallRating(const TaskDatabase& db) const {
    return RatingCalculator::calculateOverallRating(solved_tasks, db);
}

chrono::system_clock::time_point UserProfile::getLastAttemptTime(int task_id) const {
    auto it = last_attempt.find(task_id);
    if (it != last_attempt.end())
        return it->second;
    return chrono::system_clock::time_point::min();
}

} // namespace algotrainer