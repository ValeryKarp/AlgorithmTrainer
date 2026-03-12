#pragma once
#include "TaskDatabase.h"
#include "UserProfile.h"
#include <random>
#include <map>

namespace algotrainer {

    class TaskSelector {
    public:
        TaskSelector(const TaskDatabase& db, const UserProfile& profile);
        Task selectNextTask();
        void setEnabledTopics(const std::vector<std::string>& topics);

    private:
        const TaskDatabase& db;
        const UserProfile& profile;
        std::vector<std::string> enabled_topics;
        static constexpr float sigma = 1.5; // Параметр регулирующий скорость уменьшения веса темы при отклонении от искомого
        static constexpr float alpha = 0.1; // Модификатор веса решенных задач
        static constexpr float timeStep = 12; // Через сколько часов увеличивается вероятность
        static constexpr float maxTimeStep = 60; // Максимальное кол-во шагов
        static constexpr float timeFactor = 0.1; // Увеличения вероятности на каждом шагу
        mutable std::mt19937 rng; // mutable, чтобы можно было менять в const-методах

        std::map<std::string, float> calculateTopicWeights() const;
        std::string selectTopicByWeight(const std::map<std::string, float>& weights);
        std::map<int, float> calculateTaskWeights(const std::string& topic) const;
        int selectTaskByWeight(const std::map<int, float>& weights);
    };

} // namespace algotrainer