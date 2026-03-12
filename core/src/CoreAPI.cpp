#include "CoreAPI.h"
#include <stdexcept>
#include <filesystem>

namespace algotrainer {

CoreAPI::CoreAPI(const std::string& data_path)
    : data_path(data_path),
      selector(db, profile)
{
    std::string tasks_file = data_path + "/tasks.json";
    if (!db.load(tasks_file))
        throw std::runtime_error("Failed to load tasks.json from " + tasks_file);

    std::string profile_file = data_path + "/../profile/user_profile.json";
    if (!profile.load(profile_file)) {
        profile.setProfilePath(profile_file);
        profile.save();
    }
}

Task CoreAPI::getNextTask() {
    return selector.selectNextTask();
}

void CoreAPI::submitResult(int task_id, bool solved) {
    profile.addEntry(task_id, solved, db);
}

pybind11::dict CoreAPI::getStats() {
    pybind11::dict stats;
    stats["overall"] = profile.getOverallRating(db);

    pybind11::dict topics;
    for (const auto& topic : db.getAllTopics()) {
        topics[topic.c_str()] = profile.getTopicRating(topic, db);
    }
    stats["topics"] = topics;
    return stats;
}

void CoreAPI::resetProgress() {
    profile.reset();
}

void CoreAPI::setEnabledTopics(const std::vector<std::string>& topics) {
    selector.setEnabledTopics(topics);
}

std::map<std::string, std::vector<Task>> CoreAPI::getAllTasksByTopic() const {
    std::map<std::string, std::vector<Task>> result;
    auto topics = db.getAllTopics();
    for (const auto& topic : topics) {
        auto ids = db.getTaskIdsByTopic(topic);
        std::vector<Task> tasks;
        for (int id : ids) {
            tasks.push_back(db.getTask(id));
        }
        result[topic] = tasks;
    }
    return result;
}

Task CoreAPI::getTaskById(int id) const {
    return db.getTask(id);
}

void CoreAPI::shutdown() {
    profile.save();
}

} // namespace algotrainer