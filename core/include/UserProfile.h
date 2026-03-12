#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

namespace algotrainer {

    class TaskDatabase;

    class UserProfile {
    public:
        bool load(const std::string& filepath);
        bool save() const;
        void addEntry(int task_id, bool solved, const TaskDatabase& db);
        void reset(); // сброс прогресса

        float getTopicRating(const std::string& topic, const TaskDatabase& db) const;
        float getOverallRating(const TaskDatabase& db) const;
        std::chrono::system_clock::time_point getLastAttemptTime(int task_id) const;
        const std::unordered_set<int>& getSolvedTasks() const { return solved_tasks; }

        void setProfilePath(const std::string& path) { profile_path = path; }

    private:
        std::unordered_set<int> solved_tasks; // решённые задачи
        std::unordered_map<int, std::chrono::system_clock::time_point> last_attempt; // время последней попытки (любой)
        std::string profile_path;
    };

} // namespace algotrainer