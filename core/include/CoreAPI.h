#pragma once
#include "TaskDatabase.h"
#include "UserProfile.h"
#include "TaskSelector.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace algotrainer {

    class CoreAPI {
    public:
        CoreAPI(const std::string& data_path);
        Task getNextTask();
        void submitResult(int task_id, bool solved);
        pybind11::dict getStats(); // Сразу питоновским словарем т.к значения разных типов
        void resetProgress();
        void setEnabledTopics(const std::vector<std::string>& topics);
        std::map<std::string, std::vector<Task>> getAllTasksByTopic() const;
        Task getTaskById(int id) const;
        void shutdown();

    private:
        TaskDatabase db;
        UserProfile profile;
        TaskSelector selector;
        std::string data_path;
    };

} // namespace algotrainer