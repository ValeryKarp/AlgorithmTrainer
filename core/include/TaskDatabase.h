#pragma once
#include "Task.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace algotrainer {

    class TaskDatabase {
    public:
        bool load(const std::string& filepath);

        // Для нахождения задач
        const Task& getTask(int id) const;
        std::vector<int> getTaskIdsByTopic(const std::string& topic) const;
        std::vector<int> getTaskIdsByDifficulty(int minDiff, int maxDiff, const std::string& topic = "") const;

        // Получение списка тем
        std::vector<std::string> getAllTopics() const;

        // Для подсчёта количества задач
        size_t getTaskCountByTopic(const std::string& topic) const;
        size_t getTotalTaskCount() const;

        size_t size() const { return tasks.size(); }

    private:
        std::vector<Task> tasks;
        std::unordered_map<int, const Task*> tasks_by_id;
        std::unordered_map<std::string, std::vector<int>> tasks_by_topic;
    };

} // namespace algotrainer