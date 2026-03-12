#include "TaskSelector.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace algotrainer {

TaskSelector::TaskSelector(const TaskDatabase& db, const UserProfile& profile)
    : db(db), profile(profile), rng(std::random_device{}()), enabled_topics() {}

void TaskSelector::setEnabledTopics(const std::vector<std::string>& topics) {
    enabled_topics = topics;
}

Task TaskSelector::selectNextTask() {
    auto topic_weights = calculateTopicWeights();
    std::string topic = selectTopicByWeight(topic_weights);
    auto task_weights = calculateTaskWeights(topic);
    int selected_id = selectTaskByWeight(task_weights);
    return db.getTask(selected_id);
}

std::map<std::string, float> TaskSelector::calculateTopicWeights() const {
    std::map<std::string, float> weights;
    std::vector<std::string> topics;
    if (enabled_topics.empty()) {
        topics = db.getAllTopics();
    } else {
        topics = enabled_topics;
    }
    for (const auto& t : topics) {
        float rating = profile.getTopicRating(t, db); // Вес обратно пропорционален рейтингу
        float w = 100.0f - rating;
        if (w < 1.0f) w = 1.0f;
        weights[t] = w;
    }
    return weights;
}

std::string TaskSelector::selectTopicByWeight(const std::map<std::string, float>& weights) {
    std::vector<std::string> topics;
    std::vector<float> probs;
    topics.reserve(weights.size());
    probs.reserve(weights.size());
    for (const auto& [t, w] : weights) {
        topics.push_back(t);
        probs.push_back(w);
    }
    std::discrete_distribution<> dist(probs.begin(), probs.end());
    int idx = dist(rng);
    return topics[idx];
}

std::map<int, float> TaskSelector::calculateTaskWeights(const std::string& topic) const {
    std::map<int, float> weights;

    float topic_rating = profile.getTopicRating(topic, db);
    float target = 1.0f + topic_rating / 100.0f * 9.0f;

    std::vector<int> ids = db.getTaskIdsByTopic(topic);
    std::unordered_set<int> solved = profile.getSolvedTasks();
    auto now = std::chrono::system_clock::now();

    bool hasUnsolved = false;
    for (int id : ids) {
        if (!solved.count(id)) {
            hasUnsolved = true;
            break;
        }
    }

    for (int id : ids) {
        float w;
        if (hasUnsolved) {
            // Если есть нерешённые, считаем вес по сложности
            Task t = db.getTask(id);
            w = std::exp(-std::abs(static_cast<float>(t.difficulty) - target) / sigma);
            if (solved.count(id)) {
                w *= alpha; // понижаем вес для решённых
            }
        } else {
            // Все задачи решены – вес зависит от времени последней попытки
            auto last = profile.getLastAttemptTime(id);
            double steps;
            auto diff = now - last;
            steps = std::chrono::duration_cast<std::chrono::duration<double>>(diff).count() / (timeStep * 3600.0);
            if (steps < 0) steps = 0;
            if (steps > maxTimeStep) steps = maxTimeStep;
            w = 1.0f + static_cast<float>(steps) * timeFactor;
        }
        weights[id] = w;
    }
    return weights;
}

int TaskSelector::selectTaskByWeight(const std::map<int, float>& weights) {
    std::vector<int> ids;
    std::vector<float> probs;
    ids.reserve(weights.size());
    probs.reserve(weights.size());
    for (const auto& [id, w] : weights) {
        ids.push_back(id);
        probs.push_back(w);
    }
    std::discrete_distribution<> dist(probs.begin(), probs.end());
    int idx = dist(rng);
    return ids[idx];
}

} // namespace algotrainer