#include "TaskDatabase.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace algotrainer {

bool TaskDatabase::load(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return false;

    json j;
    try {
        f >> j;
    } catch (...) {
        return false;
    }

    tasks.clear();
    tasks_by_id.clear();
    tasks_by_topic.clear();

    for (const auto& item : j) {
        Task t;
        t.id = item.at("id").get<int>();
        t.topic = item.at("topic").get<std::string>();
        t.difficulty = item.at("difficulty").get<int>();
        t.question = item.at("question").get<std::string>();
        t.explanation = item.at("explanation").get<std::string>();
        t.title = item.at("title").get<std::string>();
        if (item.contains("image")) t.image_path = item["image"].get<std::string>();
        if (item.contains("source")) t.source = item["source"].get<std::string>();
        else t.source = "Unknown";

        tasks.push_back(std::move(t));
    }

    for (const auto& t : tasks) {
        tasks_by_id[t.id] = &t;
        tasks_by_topic[t.topic].push_back(t.id);
    }

    return true;
}

const Task& TaskDatabase::getTask(int id) const {
    auto it = tasks_by_id.find(id);
    if (it == tasks_by_id.end())
        throw std::out_of_range("Task id not found");
    return *it->second;
}

std::vector<int> TaskDatabase::getTaskIdsByTopic(const std::string& topic) const {
    auto it = tasks_by_topic.find(topic);
    if (it != tasks_by_topic.end())
        return it->second;
    return {};
}

std::vector<int> TaskDatabase::getTaskIdsByDifficulty(int minDiff, int maxDiff, const std::string& topic) const {
    std::vector<int> result;
    if (topic.empty()) {
        for (const auto& t : tasks) {
            if (t.difficulty >= minDiff && t.difficulty <= maxDiff)
                result.push_back(t.id);
        }
    } else {
        auto ids = getTaskIdsByTopic(topic);
        for (int id : ids) {
            const auto& t = getTask(id);
            if (t.difficulty >= minDiff && t.difficulty <= maxDiff)
                result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> TaskDatabase::getAllTopics() const {
    std::vector<std::string> topics;
    topics.reserve(tasks_by_topic.size());
    for (const auto& pair : tasks_by_topic)
        topics.push_back(pair.first);
    return topics;
}

size_t TaskDatabase::getTaskCountByTopic(const std::string& topic) const {
    auto it = tasks_by_topic.find(topic);
    if (it != tasks_by_topic.end())
        return it->second.size();
    return 0;
}

size_t TaskDatabase::getTotalTaskCount() const {
    return tasks.size();
}

} // namespace algotrainer