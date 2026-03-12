#include "RatingCalculator.h"
#include "TaskDatabase.h"
#include <numeric>

namespace algotrainer {

    float RatingCalculator::calculateTopicRating(const std::string& topic,
                                                 const std::unordered_set<int>& solved_tasks,
                                                 const TaskDatabase& db) {

        auto task_ids = db.getTaskIdsByTopic(topic);
        if (task_ids.empty()) return 0.0f;

        size_t weight_sum = 0;
        size_t max_possible = 0;
        for (int id : task_ids) {
            auto &t = db.getTask(id);
            max_possible += t.difficulty;
            if (solved_tasks.count(id)) {
                weight_sum += t.difficulty;
            }
        }
        return (static_cast<float>(weight_sum) / static_cast<float>(max_possible)) * 100.0f;
    }

    float RatingCalculator::calculateOverallRating(const std::unordered_set<int>& solved_tasks,
                                                   const TaskDatabase& db) {
        auto topics = db.getAllTopics();
        if (topics.empty()) return 0.0f;

        size_t weight_sum = 0;
        size_t max_possible = 0;
        for (int id : solved_tasks) {
            auto &t = db.getTask(id);
            weight_sum += t.difficulty;
        }
        for (auto topic: topics) {
            auto task_ids = db.getTaskIdsByTopic(topic);
            for (auto id: task_ids) {
                auto &t = db.getTask(id);
                max_possible += t.difficulty;
            }
        }
        return (static_cast<float>(weight_sum) / static_cast<float>(max_possible)) * 100.0f;
    }

} // namespace algotrainer