#pragma once
#include <unordered_set>
#include <string>

namespace algotrainer {

    class TaskDatabase;

    class RatingCalculator {
    public:
        // Вычисляет рейтинг по теме
        static float calculateTopicRating(const std::string& topic,
                                          const std::unordered_set<int>& solved_tasks,
                                          const TaskDatabase& db);

        // Вычисляет общий рейтинг как среднее по темам
        static float calculateOverallRating(const std::unordered_set<int>& solved_tasks,
                                             const TaskDatabase& db);
    };

} // namespace algotrainer